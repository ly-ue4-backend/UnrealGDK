// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
using System;
using System.Threading;
using System.Collections.Generic;
using System.Diagnostics;

namespace Improbable.WorkerCoordinator
{
    /// <summary>
    /// Simulated player client's information.
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

    internal interface ILifetimeComponentHost
    {
        void StartClient(ClientInfo clientInfo);
        void StopClient(ClientInfo clientInfo);

        Process GetActiveProcess(string clientName);
        void RemoveActiveProcess(string clientName);
    }

    internal class LifetimeComponent
    {
        // Arguments for lifetime management.
        public const string MaxLifetimeArg = "max_lifetime";
        public const string MinLifetimeArg = "min_lifetime";
        public const string UseNewSimulatedPlayerArg = "use_new_simulated_player";

        // Lifetime management parameters.
        private ILifetimeComponentHost Host;
        private bool UseNewSimulatedPlayer;
        private int MaxLifetime;
        private int MinLifetime;
        private List<ClientInfo> WaitingList;
        private List<ClientInfo> StartingList;
        private List<ClientInfo> RunningList;

        private int TickIntervalSeconds = 1;
        private Logger Logger;
        private Random Random;

        // Temp variables to avoid allocate variable in tick method.
        private long CurTicks;
        private int Length;
        private ClientInfo ClientInfo;
        private Process ClientProcess;

        /// <summary>
        /// Create lifetime component, will return null if max lifetime argument not in args.
        /// </summary>
        /// <param name="logger"></param>
        /// <param name="args"></param>
        /// <param name="numArgs"></param>
        /// <returns></returns>
        public static LifetimeComponent Create(Logger logger, string[] args, out int numArgs)
        {
            numArgs = 0;

            // Max lifetime argument.
            int maxLifetime = 0;
            if (Util.HasIntegerArgument(args, MaxLifetimeArg))
            {
                maxLifetime = Util.GetIntegerArgument(args, MaxLifetimeArg);
                numArgs++;
            }

            // Use max lifetime as default.
            int minLifetime = maxLifetime;
            if (Util.HasIntegerArgument(args, MinLifetimeArg))
            {
                minLifetime = Util.GetIntegerArgument(args, MinLifetimeArg);
                numArgs++;
            }

            // Default do not use new simulated player to restart.
            int useNewSimulatedPlayer = 0;
            if (Util.HasIntegerArgument(args, UseNewSimulatedPlayerArg))
            {
                useNewSimulatedPlayer = Util.GetIntegerArgument(args, UseNewSimulatedPlayerArg);
                numArgs++;
            }

            // Disable function by do not define max lifetime.
            if (maxLifetime > 0)
            {
                return new LifetimeComponent(maxLifetime, minLifetime, useNewSimulatedPlayer > 0, logger);
            }

            return null;
        }

        private LifetimeComponent(int maxLifetime, int minLifetime, bool useNewSimulatedPlayer, Logger logger)
        {
            UseNewSimulatedPlayer = useNewSimulatedPlayer;
            MaxLifetime = maxLifetime;
            MinLifetime = minLifetime;
            Logger = logger;

            WaitingList = new List<ClientInfo>();
            StartingList = new List<ClientInfo>();
            RunningList = new List<ClientInfo>();

            Random = new Random(Guid.NewGuid().GetHashCode());
        }

        public void SetHost(ILifetimeComponentHost host)
        {
            Host = host;
        }

        public void AddSimulatedPlayer(ClientInfo clientInfo)
        {
            WaitingList.Add(clientInfo);

            Logger.WriteLog($"=======> add client info ClientName={ClientInfo.ClientName}, StartTick={ClientInfo.StartTick}, EndTick={ClientInfo.EndTick}, curTick={CurTicks}");
        }

        private long NewLifetimeTicks()
        {
            return TimeSpan.FromMinutes(Random.Next(MinLifetime, MaxLifetime)).Ticks;
        }

        public void Start()
        {
            // Loop tick.
            while (true)
            {
                Tick();

                Thread.Sleep(TimeSpan.FromSeconds(TickIntervalSeconds));
            }
        }

        private void Tick()
        {
            CurTicks = DateTime.Now.Ticks;

            // Data flow is waiting list -> starting list -> running list -> waiting list.
            // Checking sequence is running list -> starting list -> waiting list.

            // Running list.
            Length = RunningList.Count;
            for (int i = Length - 1; i >= 0; --i)
            {
                ClientInfo = RunningList[i];
                if (CurTicks >= ClientInfo.EndTick)
                {
                    // End client.
                    Host?.StopClient(ClientInfo);

                    Logger.WriteLog($"=======> end client info ClientName={ClientInfo.ClientName}, StartTick={ClientInfo.StartTick}, EndTick={ClientInfo.EndTick}, curTick={CurTicks}");

                    // Delay 10 seconds to restart.
                    ClientInfo.StartTick = TimeSpan.FromSeconds(10).Ticks + CurTicks;

                    // Restart with new simulated player.
                    if (UseNewSimulatedPlayer)
                    {
                        ClientInfo.ClientName = "SimulatedPlayer" + Guid.NewGuid();
                    }

                    // Move to wait list.
                    RunningList.RemoveAt(i);
                    WaitingList.Add(ClientInfo);

                    Logger.WriteLog($"=======> move to starting list ClientName={ClientInfo.ClientName}, StartTick={ClientInfo.StartTick}, EndTick={ClientInfo.EndTick}, curTick={CurTicks}");
                }
            }

            // Starting list.
            Length = StartingList.Count;
            for (int i = Length - 1; i >= 0; --i)
            {
                ClientInfo = StartingList[i];
                ClientProcess = Host?.GetActiveProcess(ClientInfo.ClientName);
                if (ClientProcess != null && ClientProcess.HasExited)
                {
                    Host?.RemoveActiveProcess(ClientInfo.ClientName);
                    StartingList.RemoveAt(i);

                    if (ClientProcess.ExitCode == 0)
                    {
                        // move it to running list.
                        ClientInfo.EndTick = CurTicks + NewLifetimeTicks();
                        RunningList.Add(ClientInfo);

                        Logger.WriteLog($"=======> move to running list ClientName={ClientInfo.ClientName}, StartTick={ClientInfo.StartTick}, EndTick={ClientInfo.EndTick}, curTick={CurTicks}");
                    }
                    else
                    {
                        // Try restart by moving it back to waiting list.
                        WaitingList.Add(ClientInfo);

                        Logger.WriteLog($"=======> move back to waiting list ClientName={ClientInfo.ClientName}, StartTick={ClientInfo.StartTick}, EndTick={ClientInfo.EndTick}, curTick={CurTicks}");
                    }
                }
            }

            // Waiting list.
            Length = WaitingList.Count;
            for (int i = Length - 1; i >= 0; --i)
            {
                ClientInfo = WaitingList[i];
                if (CurTicks >= ClientInfo.StartTick)
                {
                    // Start client.
                    Host?.StartClient(ClientInfo);

                    // Move to running list.
                    WaitingList.RemoveAt(i);
                    StartingList.Add(ClientInfo);

                    Logger.WriteLog($"=======> move back to waiting list ClientName={ClientInfo.ClientName}, StartTick={ClientInfo.StartTick}, EndTick={ClientInfo.EndTick}, curTick={CurTicks}");
                }
            }
        }
    }
}
