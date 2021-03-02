// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using Improbable.Worker.CInterop;
using System.Diagnostics;

namespace Improbable.WorkerCoordinator
{
    /// <summary>
    /// Simulated player client's infomation.
    /// The coordinator use this to start & restart simulated player clients.
    /// </summary>
    struct ClientInfo
    {
        public string ClientName;
        public string DevAuthToken;
        public string TargetDeployment;
        public long StartTick;
        public long EndTick;
    }

    /// <summary>
    /// Worker coordinator that connects and runs simulated players.
    /// The coordinator runs as a managed worker inside a hosting deployment (i.e. the simulated player deployment).
    /// </summary>
    internal class ManagedWorkerCoordinator : AbstractWorkerCoordinator
    {
        // Arguments for the coordinator.
        private const string SimulatedPlayerSpawnCountArg = "simulated_player_spawn_count";
        private const string InitialStartDelayArg = "coordinator_start_delay_millis";
        private const string MaxLifetimeArg = "max_lifetime";
        private const string MinLifetimeArg = "min_lifetime";
        private const string UseNewSimulatedPlayerArg = "use_new_simulated_player";

        // Worker flags.
        private const string DevAuthTokenWorkerFlag = "simulated_players_dev_auth_token";
        private const string TargetDeploymentWorkerFlag = "simulated_players_target_deployment";
        private const string DeploymentTotalNumSimulatedPlayersWorkerFlag = "total_num_simulated_players";

        private const int AverageDelayMillisBetweenConnections = 1500;

        // Argument placeholders for simulated players - these will be replaced by the coordinator by their actual values.
        private const string SimulatedPlayerWorkerNamePlaceholderArg = "<IMPROBABLE_SIM_PLAYER_WORKER_ID>";
        private const string DevAuthTokenPlaceholderArg = "<DEV_AUTH_TOKEN>";
        private const string TargetDeploymentPlaceholderArg = "<TARGET_DEPLOYMENT>";

        private const string CoordinatorWorkerType = "SimulatedPlayerCoordinator";
        private const string SimulatedPlayerFilename = "StartSimulatedClient.sh";
        private const string StopSimulatedPlayerFilename = "StopSimulatedClient.sh";

        private const int MillisToTicks = 10000;

        private static Random Random;

        // Coordinator options.
        private string ReceptionistHost;
        private ushort ReceptionistPort;
        private string CoordinatorWorkerId;
        private int NumSimulatedPlayersToStart;
        private int InitialStartDelayMillis;
        private string[] SimulatedPlayerArgs;

        private bool UseNewSimulatedPlayer = false;
        private int MaxLifetime;
        private int MinLifetime;

        private List<ClientInfo> WaitList = new List<ClientInfo>();
        private List<ClientInfo> RunningList = new List<ClientInfo>();

        /// <summary>
        /// The arguments to start the coordinator must begin with:
        ///     receptionist
        ///     {hostname}                              Receptionist hostname
        ///     {port}                                  Receptionist port
        ///     {worker_id}                             Worker id of the coordinator
        ///
        /// Next, two optional arguments can be specified:
        ///     simulated_player_spawn_count={value}    Number of simulated player clients to start per coordinator (defaults to 1)
        ///     coordinator_start_delay_millis={value}  Minimum delay before the coordinator starts a simulated client, to prevent clients
        ///                                             from connecting too soon to the target deployment (defaults to 10000)
        ///
        /// All following arguments will be passed to the simulated player instance.
        /// These arguments can contain the following placeholders, which will be replaced by the coordinator:
        ///     `<IMPROBABLE_SIM_PLAYER_WORKER_ID>`     will be replaced with the worker id of the simulated player
        ///     `<DEV_AUTH_TOKEN>`                      if a development auth token is specified as a worker flag, this will be replaced by the generated development auth login token
        ///     `<TARGET_DEPLOYMENT>`                   will be replaced with the name of the deployment the simulated player clients will connect to
        ///
        /// WORKER FLAGS
        /// Additionally, the following worker flags are expected by the coordinator:
        ///     simulated_players_dev_auth_token        The development auth token used to generate login tokens and player identity tokens
        ///     simulated_players_target_deployment     Name of the target deployment which the simulated players will connect to
        ///     target_num_simulated_players            The total number of simulated players that will connect to the target deployment. This value is used to determine the delay in connecting simulated players.
        /// </summary>
        public static ManagedWorkerCoordinator FromArgs(Logger logger, string[] args)
        {
            // Keeps track of the number of arguments used for the coordinator.
            // The first 4 arguments are for connecting to the Receptionist.
            // 2 optional arguments can be provided after the Receptionist args to
            // modify the default options of the coordinator.
            var numArgsToSkip = 4;

            // Optional args with default values.
            var numSimulatedPlayersToStart = 1;
            var initialStartDelayMillis = 10000;
            if (Util.HasIntegerArgument(args, SimulatedPlayerSpawnCountArg))
            {
                numSimulatedPlayersToStart = Util.GetIntegerArgument(args, SimulatedPlayerSpawnCountArg);
                numArgsToSkip++;
            }
            if (Util.HasIntegerArgument(args, InitialStartDelayArg))
            {
                initialStartDelayMillis = Util.GetIntegerArgument(args, InitialStartDelayArg);
                numArgsToSkip++;
            }

            // Default do not use new simulated player to restart.
            int useNewSimulatedPlayer = 0;
            if (Util.HasIntegerArgument(args, UseNewSimulatedPlayerArg))
            {
                useNewSimulatedPlayer = Util.GetIntegerArgument(args, UseNewSimulatedPlayerArg);
                numArgsToSkip++;
            }

            // Default life time in minutes.
            int maxLifetime = 10;
            int minLifetime = 5;
            if (Util.HasIntegerArgument(args, MaxLifetimeArg))
            {
                maxLifetime = Util.GetIntegerArgument(args, MaxLifetimeArg);
                numArgsToSkip++;
            }
            if (Util.HasIntegerArgument(args, MinLifetimeArg))
            {
                minLifetime = Util.GetIntegerArgument(args, MinLifetimeArg);
                numArgsToSkip++;
            }

            return new ManagedWorkerCoordinator(logger)
            {
                // Receptionist args.
                ReceptionistHost = args[1],
                ReceptionistPort = Convert.ToUInt16(args[2]),
                CoordinatorWorkerId = args[3],

                // Coordinator options.
                NumSimulatedPlayersToStart = numSimulatedPlayersToStart,
                InitialStartDelayMillis = initialStartDelayMillis,

                // Remove arguments that are only for the coordinator.
                SimulatedPlayerArgs = args.Skip(numArgsToSkip).ToArray(),

                // Use new simulated player to restart.
                UseNewSimulatedPlayer = useNewSimulatedPlayer > 0,

                // Lifetime
                MaxLifetime = maxLifetime,
                MinLifetime = minLifetime
            };
        }

        private ManagedWorkerCoordinator(Logger logger) : base(logger)
        {
            Random = new Random(Guid.NewGuid().GetHashCode());
        }

        public override void Run()
        {
            var connection = CoordinatorConnection.ConnectAndKeepAlive(Logger, ReceptionistHost, ReceptionistPort, CoordinatorWorkerId, CoordinatorWorkerType);

            // Read worker flags.
            string devAuthToken = connection.GetWorkerFlag(DevAuthTokenWorkerFlag);
            string targetDeployment = connection.GetWorkerFlag(TargetDeploymentWorkerFlag);
            int deploymentTotalNumSimulatedPlayers = int.Parse(GetWorkerFlagOrDefault(connection, DeploymentTotalNumSimulatedPlayersWorkerFlag, "100"));

            Logger.WriteLog($"Starting {NumSimulatedPlayersToStart} simulated players.");
            Thread.Sleep(InitialStartDelayMillis);

            var maxDelayMillis = deploymentTotalNumSimulatedPlayers * AverageDelayMillisBetweenConnections;

            // Distribute player connections uniformly by generating a random time to connect between now and maxDelayMillis,
            // such that, on average, a player connects every AverageDelayMillisBetweenConnections milliseconds deployment-wide.
            // There can be multiple coordinator workers per deployment, to ensure that the combined connections created by all
            // coordinators are spread out uniformly, generate a start delay for each player independently of other players' start delays.
            var startDelaysMillis = new int[NumSimulatedPlayersToStart];
            for (int i = 0; i < NumSimulatedPlayersToStart; i++)
            {
                startDelaysMillis[i] = Random.Next(maxDelayMillis);
            }

            Array.Sort(startDelaysMillis);

            long curTicks = DateTime.Now.Ticks;
            for (int i = 0; i < NumSimulatedPlayersToStart; i++)
            {
                string clientName = "SimulatedPlayer" + Guid.NewGuid();
                var timeToSleep = startDelaysMillis[i];
                if (i > 0)
                {
                    timeToSleep -= startDelaysMillis[i - 1];
                }

                ClientInfo clientInfo = new ClientInfo()
                {
                    ClientName = clientName,
                    StartTick = timeToSleep * MillisToTicks + curTicks,
                    DevAuthToken = devAuthToken,
                    TargetDeployment = targetDeployment
                };

                WaitList.Add(clientInfo);
            }

            while(true)
            {
                Tick();

                Thread.Sleep(TimeSpan.FromSeconds(30));
            }
        }

        private void Tick()
        {
            long curTicks = DateTime.Now.Ticks;
            int len;

            Logger.WriteLog($"=======> curTick={curTicks}, wait={WaitList.Count}, running={RunningList.Count}");

            // Check wait list.
            len = WaitList.Count;
            for (int i = len - 1; i >= 0; --i)
            {
                ClientInfo clientInfo = WaitList[i];
                if (curTicks >= clientInfo.StartTick)
                {
                    // Start client.
                    StartSimulatedPlayer(clientInfo.ClientName, clientInfo.DevAuthToken, clientInfo.TargetDeployment);

                    // Record end time.
                    //clientInfo.EndTick = TimeSpan.FromMinutes(Random.Next(MinLifetime, MaxLifetime)).Ticks + curTicks;
                    clientInfo.EndTick = TimeSpan.FromSeconds(Random.Next(MinLifetime, MaxLifetime)).Ticks + curTicks;

                    // Move to running list.
                    WaitList.RemoveAt(i);
                    RunningList.Add(clientInfo);

                    Logger.WriteLog($"=======> start client info ClientName={clientInfo.ClientName}, StartTick={clientInfo.StartTick}, EndTick={clientInfo.EndTick}, curTick={curTicks}");
                }
            }

            // Check running list.
            len = RunningList.Count;
            for (int i = len - 1; i >= 0; --i)
            {
                ClientInfo clientInfo = RunningList[i];
                if (curTicks >= clientInfo.EndTick)
                {
                    // End client.
                    KillProcess(clientInfo.ClientName);

                    Logger.WriteLog($"=======> end client info ClientName={clientInfo.ClientName}, StartTick={clientInfo.StartTick}, EndTick={clientInfo.EndTick}, curTick={curTicks}");

                    // Delay 10 seconds to restart.
                    clientInfo.StartTick = TimeSpan.FromSeconds(10).Ticks + curTicks;

                    // Restart with new simulated player.
                    if (UseNewSimulatedPlayer)
                    {
                        clientInfo.ClientName = "SimulatedPlayer" + Guid.NewGuid();
                    }

                    // Move to wait list.
                    RunningList.RemoveAt(i);
                    WaitList.Add(clientInfo);
                }
            }
        }

        private void StartSimulatedPlayer(string simulatedPlayerName, string devAuthToken, string targetDeployment)
        {
            try
            {
                // Pass in the dev auth token and the target deployment
                if (!String.IsNullOrEmpty(devAuthToken) && !String.IsNullOrEmpty(targetDeployment))
                {
                    string[] simulatedPlayerArgs = Util.ReplacePlaceholderArgs(SimulatedPlayerArgs, new Dictionary<string, string>() {
                        { SimulatedPlayerWorkerNamePlaceholderArg, simulatedPlayerName },
                        { DevAuthTokenPlaceholderArg, devAuthToken },
                        { TargetDeploymentPlaceholderArg, targetDeployment }
                    });

                    // Prepend the simulated player id as an argument to the start client script.
                    // This argument is consumed by the start client script and will not be passed to the client worker.
                    simulatedPlayerArgs = new string[] { simulatedPlayerName }.Concat(simulatedPlayerArgs).ToArray();

                    // Start the client
                    string flattenedArgs = string.Join(" ", simulatedPlayerArgs);
                    CreateSimulatedPlayerProcess(simulatedPlayerName, SimulatedPlayerFilename, flattenedArgs);
                }
                else
                {
                    Logger.WriteLog($"No development auth token or target deployment provided through worker flags \"{DevAuthTokenWorkerFlag}\" and \"{TargetDeploymentWorkerFlag}\".");
                }
            }
            catch (Exception e)
            {
                Logger.WriteError($"Failed to start simulated player: {e.Message}");
            }
        }

        private static string GetWorkerFlagOrDefault(Connection connection, string flagName, string defaultValue)
        {
            string flagValue = connection.GetWorkerFlag(flagName);
            if (flagValue != null)
            {
                return flagValue;
            }

            return defaultValue;
        }

        private void KillProcess(string key)
        {
            try
            {
                Process process = Process.Start(StopSimulatedPlayerFilename, key);
                process.WaitForExit();
            }
            catch(Exception e)
            {
                Logger.WriteError($"Failed to kill process with key={key}: {e.Message}");
            }
        }
    }
}
