// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/CrossServerRPCHandler.h"
#include "EngineClasses/SpatialLoadBalanceEnforcer.h"
#include "EngineClasses/SpatialNetBitWriter.h"
#include "Interop/Connection/SpatialGDKSpanId.h"
#include "Interop/RPCs/SpatialRPCService.h"
#include "Interop/SpatialClassInfoManager.h"
#include "Schema/RPCPayload.h"
#include "Utils/RPCContainer.h"
#include "Utils/RepDataUtils.h"

#include "CoreMinimal.h"
#include "TimerManager.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialSender.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialSender, Log, All);

class USpatialActorChannel;
class SpatialDispatcher;
class USpatialNetDriver;
class USpatialPackageMapClient;
class USpatialReceiver;
class USpatialStaticComponentView;
class USpatialClassInfoManager;
class USpatialWorkerConnection;

namespace SpatialGDK
{
class SpatialEventTracer;
}

// TODO: Clear TMap entries when USpatialActorChannel gets deleted - UNR:100
// care for actor getting deleted before actor channel
using FChannelObjectPair = TPair<TWeakObjectPtr<USpatialActorChannel>, TWeakObjectPtr<UObject>>;
using FUpdatesQueuedUntilAuthority = TMap<Worker_EntityId_Key, TArray<FWorkerComponentUpdate>>;

UCLASS()
class SPATIALGDK_API USpatialSender : public UObject
{
	GENERATED_BODY()

public:
	void Init(USpatialNetDriver* InNetDriver, FTimerManager* InTimerManager, SpatialGDK::SpatialRPCService* InRPCService,
			  SpatialGDK::SpatialEventTracer* InEventTracer);

	// Actor Updates
	void SendComponentUpdates(UObject* Object, const FClassInfo& Info, USpatialActorChannel* Channel, const FRepChangeState* RepChanges,
							  const FHandoverChangeState* HandoverChanges, uint32& OutBytesWritten);
	void SendPositionUpdate(Worker_EntityId EntityId, const FVector& Location);

	void SendAuthorityIntentUpdate(const AActor& Actor, VirtualWorkerId NewAuthoritativeVirtualWorkerId) const;
	FRPCErrorInfo SendRPC(const FPendingRPCParams& Params);
	bool SendCrossServerRPC(UObject* TargetObject, const SpatialGDK::RPCSender& Sender, UFunction* Function,
							const SpatialGDK::RPCPayload& Payload, USpatialActorChannel* Channel, const FUnrealObjectRef& TargetObjectRef);
	bool SendRingBufferedRPC(UObject* TargetObject, const SpatialGDK::RPCSender& Sender, UFunction* Function,
							 const SpatialGDK::RPCPayload& Payload, USpatialActorChannel* Channel, const FUnrealObjectRef& TargetObjectRef,
							 const FSpatialGDKSpanId& SpanId);
	void SendCommandResponse(Worker_RequestId RequestId, Worker_CommandResponse& Response, const FSpatialGDKSpanId& CauseSpanId);
	void SendEmptyCommandResponse(Worker_ComponentId ComponentId, Schema_FieldId CommandIndex, Worker_RequestId RequestId,
								  const FSpatialGDKSpanId& CauseSpanId);
	void SendCommandFailure(Worker_RequestId RequestId, const FString& Message, const FSpatialGDKSpanId& CauseSpanId);
	void SendAddComponentForSubobject(USpatialActorChannel* Channel, UObject* Subobject, const FClassInfo& Info, uint32& OutBytesWritten);
	void SendAddComponents(Worker_EntityId EntityId, TArray<FWorkerComponentData> ComponentDatas);
	void SendRemoveComponentForClassInfo(Worker_EntityId EntityId, const FClassInfo& Info);
	void SendRemoveComponents(Worker_EntityId EntityId, TArray<Worker_ComponentId> ComponentIds);
	void SendInterestBucketComponentChange(const Worker_EntityId EntityId, const Worker_ComponentId OldComponent,
										   const Worker_ComponentId NewComponent);
	void SendActorTornOffUpdate(Worker_EntityId EntityId, Worker_ComponentId ComponentId);

	void SendCreateEntityRequest(USpatialActorChannel* Channel, uint32& OutBytesWritten);
	void RetireEntity(const Worker_EntityId EntityId, bool bIsNetStartupActor);

	// Creates an entity containing just a tombstone component and the minimal data to resolve an actor.
	void CreateTombstoneEntity(AActor* Actor);

	void EnqueueRetryRPC(TSharedRef<FReliableRPCForRetry> RetryRPC);
	void FlushRetryRPCs();
	void RetryReliableRPC(TSharedRef<FReliableRPCForRetry> RetryRPC);

	void RegisterChannelForPositionUpdate(USpatialActorChannel* Channel);
	void ProcessPositionUpdates();

	void UpdateInterestComponent(AActor* Actor);

	void ProcessOrQueueOutgoingRPC(const FUnrealObjectRef& InTargetObjectRef, const SpatialGDK::RPCSender& InSenderInfo,
								   SpatialGDK::RPCPayload&& InPayload);
	void ProcessUpdatesQueuedUntilAuthority(Worker_EntityId EntityId, Worker_ComponentId ComponentId);

	void FlushRPCService();

	SpatialGDK::RPCPayload CreateRPCPayloadFromParams(UObject* TargetObject, const FUnrealObjectRef& TargetObjectRef, UFunction* Function,
													  ERPCType Type, void* Params);

	// Creates an entity authoritative on this server worker, ensuring it will be able to receive updates for the GSM.
	UFUNCTION()
	void CreateServerWorkerEntity();
	void RetryServerWorkerEntityCreation(Worker_EntityId EntityId, int AttemptCounter);

	void UpdatePartitionEntityInterestAndPosition();

	void ClearPendingRPCs(const Worker_EntityId EntityId);

	bool ValidateOrExit_IsSupportedClass(const FString& PathName);

	void SendClaimPartitionRequest(Worker_EntityId SystemWorkerEntityId, Worker_PartitionId PartitionId) const;

private:
	// Create a copy of an array of components. Deep copies all Schema_ComponentData.
	static TArray<FWorkerComponentData> CopyEntityComponentData(const TArray<FWorkerComponentData>& EntityComponents);
	// Create a copy of an array of components. Deep copies all Schema_ComponentData.
	static void DeleteEntityComponentData(TArray<FWorkerComponentData>& EntityComponents);

	// Create an entity given a set of components and an ID. Retries with the same component data and entity ID on timeout.
	void CreateEntityWithRetries(Worker_EntityId EntityId, FString EntityName, TArray<FWorkerComponentData> Components);

	// Actor Lifecycle
	Worker_RequestId CreateEntity(USpatialActorChannel* Channel, uint32& OutBytesWritten);
	Worker_ComponentData CreateLevelComponentData(AActor* Actor);

	void AddTombstoneToEntity(const Worker_EntityId EntityId);

	void PeriodicallyProcessOutgoingRPCs();

	// RPC Construction
	FSpatialNetBitWriter PackRPCDataToSpatialNetBitWriter(UFunction* Function, void* Parameters) const;

	// RPC Tracking
#if !UE_BUILD_SHIPPING
	void TrackRPC(AActor* Actor, UFunction* Function, const SpatialGDK::RPCPayload& Payload, const ERPCType RPCType);
#endif

private:
	UPROPERTY()
	USpatialNetDriver* NetDriver;

	UPROPERTY()
	USpatialStaticComponentView* StaticComponentView;

	UPROPERTY()
	USpatialWorkerConnection* Connection;

	UPROPERTY()
	USpatialReceiver* Receiver;

	UPROPERTY()
	USpatialPackageMapClient* PackageMap;

	UPROPERTY()
	USpatialClassInfoManager* ClassInfoManager;

	FTimerManager* TimerManager;

	SpatialGDK::SpatialRPCService* RPCService;

	FRPCContainer OutgoingRPCs{ ERPCQueueType::Send };

	TArray<TSharedRef<FReliableRPCForRetry>> RetryRPCs;

	FUpdatesQueuedUntilAuthority UpdatesQueuedUntilAuthorityMap;

	FChannelsToUpdatePosition ChannelsToUpdatePosition;

	SpatialGDK::SpatialEventTracer* EventTracer;
};
