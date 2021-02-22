// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/InitialOnlyFilter.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialOSDispatcherInterface.h"
#include "Interop/SpatialReceiver.h"

DEFINE_LOG_CATEGORY(LogInitialOnlyFilter);

namespace SpatialGDK
{
InitialOnlyFilter::InitialOnlyFilter(USpatialNetDriver* InNetDriver)
	: NetDriver(InNetDriver)
{
}

bool InitialOnlyFilter::HasInitialOnlyData(Worker_EntityId EntityId) const
{
	return (RetrievedInitialOnlyData.Find(EntityId) != nullptr);
}

bool InitialOnlyFilter::HasInitialOnlyDataOrRequest(Worker_EntityId EntityId)
{
	if (RetrievedInitialOnlyData.Find(EntityId) != nullptr)
	{
		return true;
	}

	PendingInitialOnlyRequests.Add(EntityId);
	return false;
}

void InitialOnlyFilter::FlushRequests()
{
	if (PendingInitialOnlyRequests.Num() == 0)
	{
		return;
	}

	TArray<Worker_Constraint> EntityConstraintArray;

	for (auto Entity : PendingInitialOnlyRequests)
	{
		Worker_Constraint Constraints{};
		Constraints.constraint_type = WORKER_CONSTRAINT_TYPE_ENTITY_ID;
		Constraints.constraint.entity_id_constraint.entity_id = Entity;

		EntityConstraintArray.Add(Constraints);
	}
	PendingInitialOnlyRequests.Empty();

	Worker_EntityQuery InitialOnlyQuery{};

	InitialOnlyQuery.constraint.constraint_type = WORKER_CONSTRAINT_TYPE_OR;
	InitialOnlyQuery.constraint.constraint.or_constraint.constraint_count = EntityConstraintArray.Num();
	InitialOnlyQuery.constraint.constraint.or_constraint.constraints = EntityConstraintArray.GetData();
	InitialOnlyQuery.snapshot_result_type_component_set_id_count = 1;
	InitialOnlyQuery.snapshot_result_type_component_set_ids = &SpatialConstants::INITIAL_ONLY_COMPONENT_SET_ID;

	const Worker_RequestId RequestID = NetDriver->Connection->SendEntityQueryRequest(&InitialOnlyQuery, SpatialGDK::RETRY_UNTIL_COMPLETE);
	EntityQueryDelegate InitialOnlyQueryDelegate;
	InitialOnlyQueryDelegate.BindRaw(this, &InitialOnlyFilter::HandleInitialOnlyResponse);

	NetDriver->Receiver->AddEntityQueryDelegate(RequestID, InitialOnlyQueryDelegate);
}

void InitialOnlyFilter::HandleInitialOnlyResponse(const Worker_EntityQueryResponseOp& Op)
{
	if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
	{
		return;
	}

	if (Op.result_count == 0)
	{
		return;
	}

	for (uint32_t i = 0; i < Op.result_count; ++i)
	{
		const Worker_Entity* Entity = &Op.results[i];
		TArray<ComponentData>& ComponentDatas = RetrievedInitialOnlyData.FindOrAdd(Entity->entity_id);
		for (uint32_t j = 0; j < Entity->component_count; ++j)
		{
			const Worker_ComponentData* ComponentData = &Entity->components[j];
			ComponentDatas.Emplace(ComponentData::CreateCopy(ComponentData->schema_type, ComponentData->component_id));
		}

		NetDriver->Connection->GetCoordinator().RefreshEntityCompleteness(Entity->entity_id);
	}
}

const TArray<SpatialGDK::ComponentData>& InitialOnlyFilter::GetInitialOnlyData(Worker_EntityId EntityId) const
{
	return *RetrievedInitialOnlyData.Find(EntityId);
}

void InitialOnlyFilter::RemoveInitialOnlyData(Worker_EntityId EntityId)
{
	RetrievedInitialOnlyData.FindAndRemoveChecked(EntityId);
}

} // namespace SpatialGDK
