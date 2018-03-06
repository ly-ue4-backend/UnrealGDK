// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// Note that this file has been generated automatically

#include "SpatialTypeBinding_GameStateBase.h"
#include "Engine.h"
#include "SpatialOS.h"
#include "EntityBuilder.h"

#include "../SpatialConstants.h"
#include "../SpatialConditionMapFilter.h"
#include "../SpatialUnrealObjectRef.h"
#include "../SpatialActorChannel.h"
#include "../SpatialPackageMapClient.h"
#include "../SpatialNetDriver.h"
#include "../SpatialInterop.h"

const FRepHandlePropertyMap& USpatialTypeBinding_GameStateBase::GetHandlePropertyMap()
{
	static FRepHandlePropertyMap HandleToPropertyMap;
	if (HandleToPropertyMap.Num() == 0)
	{
		UClass* Class = FindObject<UClass>(ANY_PACKAGE, TEXT("GameStateBase"));
		HandleToPropertyMap.Add(1, FRepHandleData{nullptr, Class->FindPropertyByName("bHidden"), COND_None, REPNOTIFY_OnChanged, 148});
		HandleToPropertyMap.Add(2, FRepHandleData{nullptr, Class->FindPropertyByName("bReplicateMovement"), COND_None, REPNOTIFY_OnChanged, 148});
		HandleToPropertyMap.Add(3, FRepHandleData{nullptr, Class->FindPropertyByName("bTearOff"), COND_None, REPNOTIFY_OnChanged, 148});
		HandleToPropertyMap.Add(4, FRepHandleData{nullptr, Class->FindPropertyByName("RemoteRole"), COND_None, REPNOTIFY_OnChanged, 164});
		HandleToPropertyMap.Add(5, FRepHandleData{nullptr, Class->FindPropertyByName("Owner"), COND_None, REPNOTIFY_OnChanged, 168});
		HandleToPropertyMap.Add(6, FRepHandleData{nullptr, Class->FindPropertyByName("ReplicatedMovement"), COND_SimulatedOrPhysics, REPNOTIFY_OnChanged, 176});
		HandleToPropertyMap.Add(7, FRepHandleData{Class->FindPropertyByName("AttachmentReplication"), nullptr, COND_Custom, REPNOTIFY_OnChanged, 232});
		HandleToPropertyMap[7].Property = Cast<UStructProperty>(HandleToPropertyMap[7].Parent)->Struct->FindPropertyByName("AttachParent");
		HandleToPropertyMap.Add(8, FRepHandleData{Class->FindPropertyByName("AttachmentReplication"), nullptr, COND_Custom, REPNOTIFY_OnChanged, 240});
		HandleToPropertyMap[8].Property = Cast<UStructProperty>(HandleToPropertyMap[8].Parent)->Struct->FindPropertyByName("LocationOffset");
		HandleToPropertyMap.Add(9, FRepHandleData{Class->FindPropertyByName("AttachmentReplication"), nullptr, COND_Custom, REPNOTIFY_OnChanged, 252});
		HandleToPropertyMap[9].Property = Cast<UStructProperty>(HandleToPropertyMap[9].Parent)->Struct->FindPropertyByName("RelativeScale3D");
		HandleToPropertyMap.Add(10, FRepHandleData{Class->FindPropertyByName("AttachmentReplication"), nullptr, COND_Custom, REPNOTIFY_OnChanged, 264});
		HandleToPropertyMap[10].Property = Cast<UStructProperty>(HandleToPropertyMap[10].Parent)->Struct->FindPropertyByName("RotationOffset");
		HandleToPropertyMap.Add(11, FRepHandleData{Class->FindPropertyByName("AttachmentReplication"), nullptr, COND_Custom, REPNOTIFY_OnChanged, 276});
		HandleToPropertyMap[11].Property = Cast<UStructProperty>(HandleToPropertyMap[11].Parent)->Struct->FindPropertyByName("AttachSocket");
		HandleToPropertyMap.Add(12, FRepHandleData{Class->FindPropertyByName("AttachmentReplication"), nullptr, COND_Custom, REPNOTIFY_OnChanged, 288});
		HandleToPropertyMap[12].Property = Cast<UStructProperty>(HandleToPropertyMap[12].Parent)->Struct->FindPropertyByName("AttachComponent");
		HandleToPropertyMap.Add(13, FRepHandleData{nullptr, Class->FindPropertyByName("Role"), COND_None, REPNOTIFY_OnChanged, 296});
		HandleToPropertyMap.Add(14, FRepHandleData{nullptr, Class->FindPropertyByName("bCanBeDamaged"), COND_None, REPNOTIFY_OnChanged, 344});
		HandleToPropertyMap.Add(15, FRepHandleData{nullptr, Class->FindPropertyByName("Instigator"), COND_None, REPNOTIFY_OnChanged, 352});
		HandleToPropertyMap.Add(16, FRepHandleData{nullptr, Class->FindPropertyByName("GameModeClass"), COND_InitialOnly, REPNOTIFY_OnChanged, 1080});
		HandleToPropertyMap.Add(17, FRepHandleData{nullptr, Class->FindPropertyByName("SpectatorClass"), COND_None, REPNOTIFY_OnChanged, 1096});
		HandleToPropertyMap.Add(18, FRepHandleData{nullptr, Class->FindPropertyByName("bReplicatedHasBegunPlay"), COND_None, REPNOTIFY_OnChanged, 1120});
		HandleToPropertyMap.Add(19, FRepHandleData{nullptr, Class->FindPropertyByName("ReplicatedWorldTimeSeconds"), COND_None, REPNOTIFY_OnChanged, 1124});
	}
	return HandleToPropertyMap;
}

UClass* USpatialTypeBinding_GameStateBase::GetBoundClass() const
{
	return AGameStateBase::StaticClass();
}

void USpatialTypeBinding_GameStateBase::Init(USpatialInterop* InInterop, USpatialPackageMapClient* InPackageMap)
{
	Super::Init(InInterop, InPackageMap);

}

void USpatialTypeBinding_GameStateBase::BindToView()
{
	TSharedPtr<worker::View> View = Interop->GetSpatialOS()->GetView().Pin();
	if (Interop->GetNetDriver()->GetNetMode() == NM_Client)
	{
		SingleClientAddCallback = View->OnAddComponent<improbable::unreal::UnrealGameStateBaseSingleClientReplicatedData>([this](
			const worker::AddComponentOp<improbable::unreal::UnrealGameStateBaseSingleClientReplicatedData>& Op)
		{
			auto Update = improbable::unreal::UnrealGameStateBaseSingleClientReplicatedData::Update::FromInitialData(Op.Data);
			USpatialActorChannel* ActorChannel = Interop->GetActorChannelByEntityId(Op.EntityId);
			if (ActorChannel)
			{
				ClientReceiveUpdate_SingleClient(ActorChannel, Update);
			}
			else
			{
				PendingSingleClientData.Add(Op.EntityId, Op.Data);
			}
		});
		SingleClientUpdateCallback = View->OnComponentUpdate<improbable::unreal::UnrealGameStateBaseSingleClientReplicatedData>([this](
			const worker::ComponentUpdateOp<improbable::unreal::UnrealGameStateBaseSingleClientReplicatedData>& Op)
		{
			USpatialActorChannel* ActorChannel = Interop->GetActorChannelByEntityId(Op.EntityId);
			if (ActorChannel)
			{
				ClientReceiveUpdate_SingleClient(ActorChannel, Op.Update);
			}
			else
			{
				Op.Update.ApplyTo(PendingSingleClientData.FindOrAdd(Op.EntityId));
			}
		});
		MultiClientAddCallback = View->OnAddComponent<improbable::unreal::UnrealGameStateBaseMultiClientReplicatedData>([this](
			const worker::AddComponentOp<improbable::unreal::UnrealGameStateBaseMultiClientReplicatedData>& Op)
		{
			auto Update = improbable::unreal::UnrealGameStateBaseMultiClientReplicatedData::Update::FromInitialData(Op.Data);
			USpatialActorChannel* ActorChannel = Interop->GetActorChannelByEntityId(Op.EntityId);
			if (ActorChannel)
			{
				ClientReceiveUpdate_MultiClient(ActorChannel, Update);
			}
			else
			{
				PendingMultiClientData.Add(Op.EntityId, Op.Data);
			}
		});
		MultiClientUpdateCallback = View->OnComponentUpdate<improbable::unreal::UnrealGameStateBaseMultiClientReplicatedData>([this](
			const worker::ComponentUpdateOp<improbable::unreal::UnrealGameStateBaseMultiClientReplicatedData>& Op)
		{
			USpatialActorChannel* ActorChannel = Interop->GetActorChannelByEntityId(Op.EntityId);
			if (ActorChannel)
			{
				ClientReceiveUpdate_MultiClient(ActorChannel, Op.Update);
			}
			else
			{
				Op.Update.ApplyTo(PendingMultiClientData.FindOrAdd(Op.EntityId));
			}
		});
	}
}

void USpatialTypeBinding_GameStateBase::UnbindFromView()
{
	TSharedPtr<worker::View> View = Interop->GetSpatialOS()->GetView().Pin();
	if (Interop->GetNetDriver()->GetNetMode() == NM_Client)
	{
		View->Remove(SingleClientAddCallback);
		View->Remove(SingleClientUpdateCallback);
		View->Remove(MultiClientAddCallback);
		View->Remove(MultiClientUpdateCallback);
	}
	for (auto& Callback : RPCReceiverCallbacks)
	{
		View->Remove(Callback);
	}
}

worker::ComponentId USpatialTypeBinding_GameStateBase::GetReplicatedGroupComponentId(EReplicatedPropertyGroup Group) const
{
	switch (Group)
	{
	case GROUP_SingleClient:
		return improbable::unreal::UnrealGameStateBaseSingleClientReplicatedData::ComponentId;
	case GROUP_MultiClient:
		return improbable::unreal::UnrealGameStateBaseMultiClientReplicatedData::ComponentId;
	default:
		checkNoEntry();
		return 0;
	}
}

worker::Entity USpatialTypeBinding_GameStateBase::CreateActorEntity(const FString& ClientWorkerId, const FVector& Position, const FString& Metadata, const FPropertyChangeState& InitialChanges, USpatialActorChannel* Channel) const
{
	// Setup initial data.
	improbable::unreal::UnrealGameStateBaseSingleClientReplicatedData::Data SingleClientData;
	improbable::unreal::UnrealGameStateBaseSingleClientReplicatedData::Update SingleClientUpdate;
	bool bSingleClientUpdateChanged = false;
	improbable::unreal::UnrealGameStateBaseMultiClientReplicatedData::Data MultiClientData;
	improbable::unreal::UnrealGameStateBaseMultiClientReplicatedData::Update MultiClientUpdate;
	bool bMultiClientUpdateChanged = false;
	BuildSpatialComponentUpdate(InitialChanges, Channel, SingleClientUpdate, bSingleClientUpdateChanged, MultiClientUpdate, bMultiClientUpdateChanged);
	SingleClientUpdate.ApplyTo(SingleClientData);
	MultiClientUpdate.ApplyTo(MultiClientData);

	// Create entity.
	std::string ClientWorkerIdString = TCHAR_TO_UTF8(*ClientWorkerId);

	improbable::WorkerAttributeSet WorkerAttribute{{worker::List<std::string>{"UnrealWorker"}}};
	improbable::WorkerAttributeSet ClientAttribute{{worker::List<std::string>{"UnrealClient"}}};
	improbable::WorkerAttributeSet OwningClientAttribute{{"workerId:" + ClientWorkerIdString}};

	improbable::WorkerRequirementSet WorkersOnly{{WorkerAttribute}};
	improbable::WorkerRequirementSet ClientsOnly{{ClientAttribute}};
	improbable::WorkerRequirementSet OwningClientOnly{{OwningClientAttribute}};
	improbable::WorkerRequirementSet AnyUnrealWorkerOrClient{{WorkerAttribute, ClientAttribute}};
	improbable::WorkerRequirementSet AnyUnrealWorkerOrOwningClient{{WorkerAttribute, OwningClientAttribute}};

	// Set up unreal metadata.
	improbable::unreal::UnrealMetadata::Data UnrealMetadata;
	if (Channel->Actor->IsFullNameStableForNetworking())
	{
		UnrealMetadata.set_static_path({std::string{TCHAR_TO_UTF8(*Channel->Actor->GetPathName(Channel->Actor->GetWorld()))}});
	}
	if (!ClientWorkerIdString.empty())
	{
		UnrealMetadata.set_owner_worker_id({ClientWorkerIdString});
	}

	// Build entity.
	const improbable::Coordinates SpatialPosition = SpatialConstants::LocationToSpatialOSCoordinates(Position);
	return improbable::unreal::FEntityBuilder::Begin()
		.AddPositionComponent(improbable::Position::Data{SpatialPosition}, WorkersOnly)
		.AddMetadataComponent(improbable::Metadata::Data{TCHAR_TO_UTF8(*Metadata)})
		.SetPersistence(true)
		.SetReadAcl(AnyUnrealWorkerOrClient)
		.AddComponent<improbable::unreal::UnrealMetadata>(UnrealMetadata, WorkersOnly)
		.AddComponent<improbable::unreal::UnrealGameStateBaseSingleClientReplicatedData>(SingleClientData, WorkersOnly)
		.AddComponent<improbable::unreal::UnrealGameStateBaseMultiClientReplicatedData>(MultiClientData, WorkersOnly)
		.AddComponent<improbable::unreal::UnrealGameStateBaseCompleteData>(improbable::unreal::UnrealGameStateBaseCompleteData::Data{}, WorkersOnly)
		.AddComponent<improbable::unreal::UnrealGameStateBaseClientRPCs>(improbable::unreal::UnrealGameStateBaseClientRPCs::Data{}, OwningClientOnly)
		.AddComponent<improbable::unreal::UnrealGameStateBaseServerRPCs>(improbable::unreal::UnrealGameStateBaseServerRPCs::Data{}, WorkersOnly)
		.Build();
}

void USpatialTypeBinding_GameStateBase::SendComponentUpdates(const FPropertyChangeState& Changes, USpatialActorChannel* Channel, const worker::EntityId& EntityId) const
{
	// Build SpatialOS updates.
	improbable::unreal::UnrealGameStateBaseSingleClientReplicatedData::Update SingleClientUpdate;
	bool bSingleClientUpdateChanged = false;
	improbable::unreal::UnrealGameStateBaseMultiClientReplicatedData::Update MultiClientUpdate;
	bool bMultiClientUpdateChanged = false;
	BuildSpatialComponentUpdate(Changes, Channel, SingleClientUpdate, bSingleClientUpdateChanged, MultiClientUpdate, bMultiClientUpdateChanged);

	// Send SpatialOS updates if anything changed.
	TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
	if (bSingleClientUpdateChanged)
	{
		Connection->SendComponentUpdate<improbable::unreal::UnrealGameStateBaseSingleClientReplicatedData>(EntityId, SingleClientUpdate);
	}
	if (bMultiClientUpdateChanged)
	{
		Connection->SendComponentUpdate<improbable::unreal::UnrealGameStateBaseMultiClientReplicatedData>(EntityId, MultiClientUpdate);
	}
}

void USpatialTypeBinding_GameStateBase::SendRPCCommand(UObject* TargetObject, const UFunction* const Function, FFrame* const Frame)
{
	TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
	auto SenderFuncIterator = RPCToSenderMap.Find(Function->GetFName());
	checkf(*SenderFuncIterator, TEXT("Sender for %s has not been registered with RPCToSenderMap."), *Function->GetFName().ToString());
	(this->*(*SenderFuncIterator))(Connection.Get(), Frame, TargetObject);
}

void USpatialTypeBinding_GameStateBase::ApplyQueuedStateToChannel(USpatialActorChannel* ActorChannel)
{
	improbable::unreal::UnrealGameStateBaseSingleClientReplicatedData::Data* SingleClientData = PendingSingleClientData.Find(ActorChannel->GetEntityId());
	if (SingleClientData)
	{
		auto Update = improbable::unreal::UnrealGameStateBaseSingleClientReplicatedData::Update::FromInitialData(*SingleClientData);
		PendingSingleClientData.Remove(ActorChannel->GetEntityId());
		ClientReceiveUpdate_SingleClient(ActorChannel, Update);
	}
	improbable::unreal::UnrealGameStateBaseMultiClientReplicatedData::Data* MultiClientData = PendingMultiClientData.Find(ActorChannel->GetEntityId());
	if (MultiClientData)
	{
		auto Update = improbable::unreal::UnrealGameStateBaseMultiClientReplicatedData::Update::FromInitialData(*MultiClientData);
		PendingMultiClientData.Remove(ActorChannel->GetEntityId());
		ClientReceiveUpdate_MultiClient(ActorChannel, Update);
	}
}

void USpatialTypeBinding_GameStateBase::BuildSpatialComponentUpdate(
	const FPropertyChangeState& Changes,
	USpatialActorChannel* Channel,
	improbable::unreal::UnrealGameStateBaseSingleClientReplicatedData::Update& SingleClientUpdate,
	bool& bSingleClientUpdateChanged,
	improbable::unreal::UnrealGameStateBaseMultiClientReplicatedData::Update& MultiClientUpdate,
	bool& bMultiClientUpdateChanged) const
{
	// Build up SpatialOS component updates.
	auto& PropertyMap = GetHandlePropertyMap();
	FChangelistIterator ChangelistIterator(Changes.Changed, 0);
	FRepHandleIterator HandleIterator(ChangelistIterator, Changes.Cmds, Changes.BaseHandleToCmdIndex, 0, 1, 0, Changes.Cmds.Num() - 1);
	while (HandleIterator.NextHandle())
	{
		const FRepLayoutCmd& Cmd = Changes.Cmds[HandleIterator.CmdIndex];
		const uint8* Data = Changes.SourceData + HandleIterator.ArrayOffset + Cmd.Offset;
		auto& PropertyMapData = PropertyMap[HandleIterator.Handle];
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending property update. actor %s (%lld), property %s (handle %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*Channel->Actor->GetName(),
			Channel->GetEntityId(),
			*Cmd.Property->GetName(),
			HandleIterator.Handle);
		switch (GetGroupFromCondition(PropertyMapData.Condition))
		{
		case GROUP_SingleClient:
			ServerSendUpdate_SingleClient(Data, HandleIterator.Handle, Cmd.Property, Channel, SingleClientUpdate);
			bSingleClientUpdateChanged = true;
			break;
		case GROUP_MultiClient:
			ServerSendUpdate_MultiClient(Data, HandleIterator.Handle, Cmd.Property, Channel, MultiClientUpdate);
			bMultiClientUpdateChanged = true;
			break;
		}
	}
}

void USpatialTypeBinding_GameStateBase::ServerSendUpdate_SingleClient(
	const uint8* RESTRICT Data,
	int32 Handle,
	UProperty* Property,
	USpatialActorChannel* Channel,
	improbable::unreal::UnrealGameStateBaseSingleClientReplicatedData::Update& OutUpdate) const
{
}

void USpatialTypeBinding_GameStateBase::ServerSendUpdate_MultiClient(
	const uint8* RESTRICT Data,
	int32 Handle,
	UProperty* Property,
	USpatialActorChannel* Channel,
	improbable::unreal::UnrealGameStateBaseMultiClientReplicatedData::Update& OutUpdate) const
{
	switch (Handle)
	{
		case 1: // field_bhidden
		{
			bool Value = static_cast<UBoolProperty*>(Property)->GetPropertyValue(Data);

			OutUpdate.set_field_bhidden(Value);
			break;
		}
		case 2: // field_breplicatemovement
		{
			bool Value = static_cast<UBoolProperty*>(Property)->GetPropertyValue(Data);

			OutUpdate.set_field_breplicatemovement(Value);
			break;
		}
		case 3: // field_btearoff
		{
			bool Value = static_cast<UBoolProperty*>(Property)->GetPropertyValue(Data);

			OutUpdate.set_field_btearoff(Value);
			break;
		}
		case 4: // field_remoterole
		{
			TEnumAsByte<ENetRole> Value = *(reinterpret_cast<TEnumAsByte<ENetRole> const*>(Data));

			OutUpdate.set_field_remoterole(uint32_t(Value));
			break;
		}
		case 5: // field_owner
		{
			AActor* Value = *(reinterpret_cast<AActor* const*>(Data));

			if (Value != nullptr)
			{
				FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(Value);
				improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
				if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
				{
					Interop->QueueOutgoingObjectUpdate_Internal(Value, Channel, 5);
				}
				else
				{
					OutUpdate.set_field_owner(ObjectRef);
				}
			}
			else
			{
				OutUpdate.set_field_owner(SpatialConstants::NULL_OBJECT_REF);
			}
			break;
		}
		case 6: // field_replicatedmovement
		{
			FRepMovement Value = *(reinterpret_cast<FRepMovement const*>(Data));

			{
				TArray<uint8> ValueData;
				FMemoryWriter ValueDataWriter(ValueData);
				bool Success;
				Value.NetSerialize(ValueDataWriter, PackageMap, Success);
				OutUpdate.set_field_replicatedmovement(std::string(reinterpret_cast<char*>(ValueData.GetData()), ValueData.Num()));
			}
			break;
		}
		case 7: // field_attachmentreplication_attachparent
		{
			AActor* Value = *(reinterpret_cast<AActor* const*>(Data));

			if (Value != nullptr)
			{
				FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(Value);
				improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
				if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
				{
					Interop->QueueOutgoingObjectUpdate_Internal(Value, Channel, 7);
				}
				else
				{
					OutUpdate.set_field_attachmentreplication_attachparent(ObjectRef);
				}
			}
			else
			{
				OutUpdate.set_field_attachmentreplication_attachparent(SpatialConstants::NULL_OBJECT_REF);
			}
			break;
		}
		case 8: // field_attachmentreplication_locationoffset
		{
			FVector_NetQuantize100 Value = *(reinterpret_cast<FVector_NetQuantize100 const*>(Data));

			OutUpdate.set_field_attachmentreplication_locationoffset(improbable::Vector3f(Value.X, Value.Y, Value.Z));
			break;
		}
		case 9: // field_attachmentreplication_relativescale3d
		{
			FVector_NetQuantize100 Value = *(reinterpret_cast<FVector_NetQuantize100 const*>(Data));

			OutUpdate.set_field_attachmentreplication_relativescale3d(improbable::Vector3f(Value.X, Value.Y, Value.Z));
			break;
		}
		case 10: // field_attachmentreplication_rotationoffset
		{
			FRotator Value = *(reinterpret_cast<FRotator const*>(Data));

			OutUpdate.set_field_attachmentreplication_rotationoffset(improbable::unreal::UnrealFRotator(Value.Yaw, Value.Pitch, Value.Roll));
			break;
		}
		case 11: // field_attachmentreplication_attachsocket
		{
			FName Value = *(reinterpret_cast<FName const*>(Data));

			OutUpdate.set_field_attachmentreplication_attachsocket(TCHAR_TO_UTF8(*Value.ToString()));
			break;
		}
		case 12: // field_attachmentreplication_attachcomponent
		{
			USceneComponent* Value = *(reinterpret_cast<USceneComponent* const*>(Data));

			if (Value != nullptr)
			{
				FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(Value);
				improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
				if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
				{
					Interop->QueueOutgoingObjectUpdate_Internal(Value, Channel, 12);
				}
				else
				{
					OutUpdate.set_field_attachmentreplication_attachcomponent(ObjectRef);
				}
			}
			else
			{
				OutUpdate.set_field_attachmentreplication_attachcomponent(SpatialConstants::NULL_OBJECT_REF);
			}
			break;
		}
		case 13: // field_role
		{
			TEnumAsByte<ENetRole> Value = *(reinterpret_cast<TEnumAsByte<ENetRole> const*>(Data));

			OutUpdate.set_field_role(uint32_t(Value));
			break;
		}
		case 14: // field_bcanbedamaged
		{
			bool Value = static_cast<UBoolProperty*>(Property)->GetPropertyValue(Data);

			OutUpdate.set_field_bcanbedamaged(Value);
			break;
		}
		case 15: // field_instigator
		{
			APawn* Value = *(reinterpret_cast<APawn* const*>(Data));

			if (Value != nullptr)
			{
				FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(Value);
				improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
				if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
				{
					Interop->QueueOutgoingObjectUpdate_Internal(Value, Channel, 15);
				}
				else
				{
					OutUpdate.set_field_instigator(ObjectRef);
				}
			}
			else
			{
				OutUpdate.set_field_instigator(SpatialConstants::NULL_OBJECT_REF);
			}
			break;
		}
		case 16: // field_gamemodeclass
		{
			TSubclassOf<AGameModeBase>  Value = *(reinterpret_cast<TSubclassOf<AGameModeBase>  const*>(Data));

			// UNSUPPORTED UClassProperty OutUpdate.set_field_gamemodeclass(Value);
			break;
		}
		case 17: // field_spectatorclass
		{
			TSubclassOf<ASpectatorPawn>  Value = *(reinterpret_cast<TSubclassOf<ASpectatorPawn>  const*>(Data));

			// UNSUPPORTED UClassProperty OutUpdate.set_field_spectatorclass(Value);
			break;
		}
		case 18: // field_breplicatedhasbegunplay
		{
			bool Value = static_cast<UBoolProperty*>(Property)->GetPropertyValue(Data);

			OutUpdate.set_field_breplicatedhasbegunplay(Value);
			break;
		}
		case 19: // field_replicatedworldtimeseconds
		{
			float Value = *(reinterpret_cast<float const*>(Data));

			OutUpdate.set_field_replicatedworldtimeseconds(Value);
			break;
		}
	default:
		checkf(false, TEXT("Unknown replication handle %d encountered when creating a SpatialOS update."));
		break;
	}
}

void USpatialTypeBinding_GameStateBase::ClientReceiveUpdate_SingleClient(
	USpatialActorChannel* ActorChannel,
	const improbable::unreal::UnrealGameStateBaseSingleClientReplicatedData::Update& Update) const
{
	Interop->PreReceiveSpatialUpdate(ActorChannel);

	TArray<UProperty*> RepNotifies;
	Interop->PostReceiveSpatialUpdate(ActorChannel, RepNotifies);
}

void USpatialTypeBinding_GameStateBase::ClientReceiveUpdate_MultiClient(
	USpatialActorChannel* ActorChannel,
	const improbable::unreal::UnrealGameStateBaseMultiClientReplicatedData::Update& Update) const
{
	Interop->PreReceiveSpatialUpdate(ActorChannel);

	TArray<UProperty*> RepNotifies;
	const bool bIsServer = Interop->GetNetDriver()->IsServer();
	const bool bAutonomousProxy = ActorChannel->IsClientAutonomousProxy(improbable::unreal::UnrealGameStateBaseClientRPCs::ComponentId);
	const FRepHandlePropertyMap& HandleToPropertyMap = GetHandlePropertyMap();
	FSpatialConditionMapFilter ConditionMap(ActorChannel, bAutonomousProxy);

	if (!Update.field_bhidden().empty())
	{
		// field_bhidden
		uint16 Handle = 1;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = reinterpret_cast<uint8*>(ActorChannel->Actor) + RepData->Offset;
			bool Value = static_cast<UBoolProperty*>(RepData->Property)->GetPropertyValue(PropertyData);

			Value = (*Update.field_bhidden().data());

			ApplyIncomingPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_breplicatemovement().empty())
	{
		// field_breplicatemovement
		uint16 Handle = 2;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = reinterpret_cast<uint8*>(ActorChannel->Actor) + RepData->Offset;
			bool Value = static_cast<UBoolProperty*>(RepData->Property)->GetPropertyValue(PropertyData);

			Value = (*Update.field_breplicatemovement().data());

			ApplyIncomingPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_btearoff().empty())
	{
		// field_btearoff
		uint16 Handle = 3;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = reinterpret_cast<uint8*>(ActorChannel->Actor) + RepData->Offset;
			bool Value = static_cast<UBoolProperty*>(RepData->Property)->GetPropertyValue(PropertyData);

			Value = (*Update.field_btearoff().data());

			ApplyIncomingPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_remoterole().empty())
	{
		// field_remoterole
		uint16 Handle = 4;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(RepData->Condition))
		{
			// On the client, we need to swap Role/RemoteRole.
			if (!bIsServer)
			{
				Handle = 13;
				RepData = &HandleToPropertyMap[Handle];
			}

			uint8* PropertyData = reinterpret_cast<uint8*>(ActorChannel->Actor) + RepData->Offset;
			TEnumAsByte<ENetRole> Value = *(reinterpret_cast<TEnumAsByte<ENetRole> const*>(PropertyData));

			Value = TEnumAsByte<ENetRole>(uint8((*Update.field_remoterole().data())));

			// Downgrade role from AutonomousProxy to SimulatedProxy if we aren't authoritative over
			// the server RPCs component.
			if (!bIsServer && Value == ROLE_AutonomousProxy && !bAutonomousProxy)
			{
				Value = ROLE_SimulatedProxy;
			}

			ApplyIncomingPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_owner().empty())
	{
		// field_owner
		uint16 Handle = 5;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(RepData->Condition))
		{
			bool bWriteObjectProperty = true;
			uint8* PropertyData = reinterpret_cast<uint8*>(ActorChannel->Actor) + RepData->Offset;
			AActor* Value = *(reinterpret_cast<AActor* const*>(PropertyData));

			{
				improbable::unreal::UnrealObjectRef ObjectRef = (*Update.field_owner().data());
				check(ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF);
				if (ObjectRef == SpatialConstants::NULL_OBJECT_REF)
				{
					Value = nullptr;
				}
				else
				{
					FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
					if (NetGUID.IsValid())
					{
						UObject* Object_Raw = PackageMap->GetObjectFromNetGUID(NetGUID, true);
						checkf(Object_Raw, TEXT("An object ref %s should map to a valid object."), *ObjectRefToString(ObjectRef));
						Value = dynamic_cast<AActor*>(Object_Raw);
						checkf(Value, TEXT("Object ref %s maps to object %s with the wrong class."), *ObjectRefToString(ObjectRef), *Object_Raw->GetFullName());
					}
					else
					{
						UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received unresolved object property. Value: %s. actor %s (%llu), property %s (handle %d)"),
							*Interop->GetSpatialOS()->GetWorkerId(),
							*ObjectRefToString(ObjectRef),
							*ActorChannel->Actor->GetName(),
							ActorChannel->GetEntityId(),
							*RepData->Property->GetName(),
							Handle);
						bWriteObjectProperty = false;
						Interop->QueueIncomingObjectUpdate_Internal(ObjectRef, ActorChannel, RepData);
					}
				}
			}

			if (bWriteObjectProperty)
			{
				ApplyIncomingPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

				UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
					*Interop->GetSpatialOS()->GetWorkerId(),
					*ActorChannel->Actor->GetName(),
					ActorChannel->GetEntityId(),
					*RepData->Property->GetName(),
					Handle);
			}
		}
	}
	if (!Update.field_replicatedmovement().empty())
	{
		// field_replicatedmovement
		uint16 Handle = 6;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = reinterpret_cast<uint8*>(ActorChannel->Actor) + RepData->Offset;
			FRepMovement Value = *(reinterpret_cast<FRepMovement const*>(PropertyData));

			{
				auto& ValueDataStr = (*Update.field_replicatedmovement().data());
				TArray<uint8> ValueData;
				ValueData.Append(ValueDataStr.data(), ValueDataStr.size());
				FMemoryReader ValueDataReader(ValueData);
				bool bSuccess;
				Value.NetSerialize(ValueDataReader, PackageMap, bSuccess);
			}

			ApplyIncomingPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_attachmentreplication_attachparent().empty())
	{
		// field_attachmentreplication_attachparent
		uint16 Handle = 7;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(RepData->Condition))
		{
			bool bWriteObjectProperty = true;
			uint8* PropertyData = reinterpret_cast<uint8*>(ActorChannel->Actor) + RepData->Offset;
			AActor* Value = *(reinterpret_cast<AActor* const*>(PropertyData));

			{
				improbable::unreal::UnrealObjectRef ObjectRef = (*Update.field_attachmentreplication_attachparent().data());
				check(ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF);
				if (ObjectRef == SpatialConstants::NULL_OBJECT_REF)
				{
					Value = nullptr;
				}
				else
				{
					FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
					if (NetGUID.IsValid())
					{
						UObject* Object_Raw = PackageMap->GetObjectFromNetGUID(NetGUID, true);
						checkf(Object_Raw, TEXT("An object ref %s should map to a valid object."), *ObjectRefToString(ObjectRef));
						Value = dynamic_cast<AActor*>(Object_Raw);
						checkf(Value, TEXT("Object ref %s maps to object %s with the wrong class."), *ObjectRefToString(ObjectRef), *Object_Raw->GetFullName());
					}
					else
					{
						UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received unresolved object property. Value: %s. actor %s (%llu), property %s (handle %d)"),
							*Interop->GetSpatialOS()->GetWorkerId(),
							*ObjectRefToString(ObjectRef),
							*ActorChannel->Actor->GetName(),
							ActorChannel->GetEntityId(),
							*RepData->Property->GetName(),
							Handle);
						bWriteObjectProperty = false;
						Interop->QueueIncomingObjectUpdate_Internal(ObjectRef, ActorChannel, RepData);
					}
				}
			}

			if (bWriteObjectProperty)
			{
				ApplyIncomingPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

				UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
					*Interop->GetSpatialOS()->GetWorkerId(),
					*ActorChannel->Actor->GetName(),
					ActorChannel->GetEntityId(),
					*RepData->Property->GetName(),
					Handle);
			}
		}
	}
	if (!Update.field_attachmentreplication_locationoffset().empty())
	{
		// field_attachmentreplication_locationoffset
		uint16 Handle = 8;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = reinterpret_cast<uint8*>(ActorChannel->Actor) + RepData->Offset;
			FVector_NetQuantize100 Value = *(reinterpret_cast<FVector_NetQuantize100 const*>(PropertyData));

			{
				auto& Vector = (*Update.field_attachmentreplication_locationoffset().data());
				Value.X = Vector.x();
				Value.Y = Vector.y();
				Value.Z = Vector.z();
			}

			ApplyIncomingPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_attachmentreplication_relativescale3d().empty())
	{
		// field_attachmentreplication_relativescale3d
		uint16 Handle = 9;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = reinterpret_cast<uint8*>(ActorChannel->Actor) + RepData->Offset;
			FVector_NetQuantize100 Value = *(reinterpret_cast<FVector_NetQuantize100 const*>(PropertyData));

			{
				auto& Vector = (*Update.field_attachmentreplication_relativescale3d().data());
				Value.X = Vector.x();
				Value.Y = Vector.y();
				Value.Z = Vector.z();
			}

			ApplyIncomingPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_attachmentreplication_rotationoffset().empty())
	{
		// field_attachmentreplication_rotationoffset
		uint16 Handle = 10;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = reinterpret_cast<uint8*>(ActorChannel->Actor) + RepData->Offset;
			FRotator Value = *(reinterpret_cast<FRotator const*>(PropertyData));

			{
				auto& Rotator = (*Update.field_attachmentreplication_rotationoffset().data());
				Value.Yaw = Rotator.yaw();
				Value.Pitch = Rotator.pitch();
				Value.Roll = Rotator.roll();
			}

			ApplyIncomingPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_attachmentreplication_attachsocket().empty())
	{
		// field_attachmentreplication_attachsocket
		uint16 Handle = 11;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = reinterpret_cast<uint8*>(ActorChannel->Actor) + RepData->Offset;
			FName Value = *(reinterpret_cast<FName const*>(PropertyData));

			Value = FName(((*Update.field_attachmentreplication_attachsocket().data())).data());

			ApplyIncomingPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_attachmentreplication_attachcomponent().empty())
	{
		// field_attachmentreplication_attachcomponent
		uint16 Handle = 12;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(RepData->Condition))
		{
			bool bWriteObjectProperty = true;
			uint8* PropertyData = reinterpret_cast<uint8*>(ActorChannel->Actor) + RepData->Offset;
			USceneComponent* Value = *(reinterpret_cast<USceneComponent* const*>(PropertyData));

			{
				improbable::unreal::UnrealObjectRef ObjectRef = (*Update.field_attachmentreplication_attachcomponent().data());
				check(ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF);
				if (ObjectRef == SpatialConstants::NULL_OBJECT_REF)
				{
					Value = nullptr;
				}
				else
				{
					FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
					if (NetGUID.IsValid())
					{
						UObject* Object_Raw = PackageMap->GetObjectFromNetGUID(NetGUID, true);
						checkf(Object_Raw, TEXT("An object ref %s should map to a valid object."), *ObjectRefToString(ObjectRef));
						Value = dynamic_cast<USceneComponent*>(Object_Raw);
						checkf(Value, TEXT("Object ref %s maps to object %s with the wrong class."), *ObjectRefToString(ObjectRef), *Object_Raw->GetFullName());
					}
					else
					{
						UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received unresolved object property. Value: %s. actor %s (%llu), property %s (handle %d)"),
							*Interop->GetSpatialOS()->GetWorkerId(),
							*ObjectRefToString(ObjectRef),
							*ActorChannel->Actor->GetName(),
							ActorChannel->GetEntityId(),
							*RepData->Property->GetName(),
							Handle);
						bWriteObjectProperty = false;
						Interop->QueueIncomingObjectUpdate_Internal(ObjectRef, ActorChannel, RepData);
					}
				}
			}

			if (bWriteObjectProperty)
			{
				ApplyIncomingPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

				UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
					*Interop->GetSpatialOS()->GetWorkerId(),
					*ActorChannel->Actor->GetName(),
					ActorChannel->GetEntityId(),
					*RepData->Property->GetName(),
					Handle);
			}
		}
	}
	if (!Update.field_role().empty())
	{
		// field_role
		uint16 Handle = 13;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(RepData->Condition))
		{
			// On the client, we need to swap Role/RemoteRole.
			if (!bIsServer)
			{
				Handle = 4;
				RepData = &HandleToPropertyMap[Handle];
			}

			uint8* PropertyData = reinterpret_cast<uint8*>(ActorChannel->Actor) + RepData->Offset;
			TEnumAsByte<ENetRole> Value = *(reinterpret_cast<TEnumAsByte<ENetRole> const*>(PropertyData));

			Value = TEnumAsByte<ENetRole>(uint8((*Update.field_role().data())));

			ApplyIncomingPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_bcanbedamaged().empty())
	{
		// field_bcanbedamaged
		uint16 Handle = 14;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = reinterpret_cast<uint8*>(ActorChannel->Actor) + RepData->Offset;
			bool Value = static_cast<UBoolProperty*>(RepData->Property)->GetPropertyValue(PropertyData);

			Value = (*Update.field_bcanbedamaged().data());

			ApplyIncomingPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_instigator().empty())
	{
		// field_instigator
		uint16 Handle = 15;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(RepData->Condition))
		{
			bool bWriteObjectProperty = true;
			uint8* PropertyData = reinterpret_cast<uint8*>(ActorChannel->Actor) + RepData->Offset;
			APawn* Value = *(reinterpret_cast<APawn* const*>(PropertyData));

			{
				improbable::unreal::UnrealObjectRef ObjectRef = (*Update.field_instigator().data());
				check(ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF);
				if (ObjectRef == SpatialConstants::NULL_OBJECT_REF)
				{
					Value = nullptr;
				}
				else
				{
					FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
					if (NetGUID.IsValid())
					{
						UObject* Object_Raw = PackageMap->GetObjectFromNetGUID(NetGUID, true);
						checkf(Object_Raw, TEXT("An object ref %s should map to a valid object."), *ObjectRefToString(ObjectRef));
						Value = dynamic_cast<APawn*>(Object_Raw);
						checkf(Value, TEXT("Object ref %s maps to object %s with the wrong class."), *ObjectRefToString(ObjectRef), *Object_Raw->GetFullName());
					}
					else
					{
						UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received unresolved object property. Value: %s. actor %s (%llu), property %s (handle %d)"),
							*Interop->GetSpatialOS()->GetWorkerId(),
							*ObjectRefToString(ObjectRef),
							*ActorChannel->Actor->GetName(),
							ActorChannel->GetEntityId(),
							*RepData->Property->GetName(),
							Handle);
						bWriteObjectProperty = false;
						Interop->QueueIncomingObjectUpdate_Internal(ObjectRef, ActorChannel, RepData);
					}
				}
			}

			if (bWriteObjectProperty)
			{
				ApplyIncomingPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

				UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
					*Interop->GetSpatialOS()->GetWorkerId(),
					*ActorChannel->Actor->GetName(),
					ActorChannel->GetEntityId(),
					*RepData->Property->GetName(),
					Handle);
			}
		}
	}
	if (!Update.field_gamemodeclass().empty())
	{
		// field_gamemodeclass
		uint16 Handle = 16;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(RepData->Condition))
		{
			bool bWriteObjectProperty = true;
			uint8* PropertyData = reinterpret_cast<uint8*>(ActorChannel->Actor) + RepData->Offset;
			TSubclassOf<AGameModeBase>  Value = *(reinterpret_cast<TSubclassOf<AGameModeBase>  const*>(PropertyData));

			// UNSUPPORTED UClassProperty Value (*Update.field_gamemodeclass().data())

			if (bWriteObjectProperty)
			{
				ApplyIncomingPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

				UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
					*Interop->GetSpatialOS()->GetWorkerId(),
					*ActorChannel->Actor->GetName(),
					ActorChannel->GetEntityId(),
					*RepData->Property->GetName(),
					Handle);
			}
		}
	}
	if (!Update.field_spectatorclass().empty())
	{
		// field_spectatorclass
		uint16 Handle = 17;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(RepData->Condition))
		{
			bool bWriteObjectProperty = true;
			uint8* PropertyData = reinterpret_cast<uint8*>(ActorChannel->Actor) + RepData->Offset;
			TSubclassOf<ASpectatorPawn>  Value = *(reinterpret_cast<TSubclassOf<ASpectatorPawn>  const*>(PropertyData));

			// UNSUPPORTED UClassProperty Value (*Update.field_spectatorclass().data())

			if (bWriteObjectProperty)
			{
				ApplyIncomingPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

				UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
					*Interop->GetSpatialOS()->GetWorkerId(),
					*ActorChannel->Actor->GetName(),
					ActorChannel->GetEntityId(),
					*RepData->Property->GetName(),
					Handle);
			}
		}
	}
	if (!Update.field_breplicatedhasbegunplay().empty())
	{
		// field_breplicatedhasbegunplay
		uint16 Handle = 18;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = reinterpret_cast<uint8*>(ActorChannel->Actor) + RepData->Offset;
			bool Value = static_cast<UBoolProperty*>(RepData->Property)->GetPropertyValue(PropertyData);

			Value = (*Update.field_breplicatedhasbegunplay().data());

			ApplyIncomingPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_replicatedworldtimeseconds().empty())
	{
		// field_replicatedworldtimeseconds
		uint16 Handle = 19;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = reinterpret_cast<uint8*>(ActorChannel->Actor) + RepData->Offset;
			float Value = *(reinterpret_cast<float const*>(PropertyData));

			Value = (*Update.field_replicatedworldtimeseconds().data());

			ApplyIncomingPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	Interop->PostReceiveSpatialUpdate(ActorChannel, RepNotifies);
}
