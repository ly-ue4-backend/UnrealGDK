// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "SpatialConstants.h"
#include "SpatialView/ComponentData.h"

DECLARE_LOG_CATEGORY_EXTERN(LogInitialOnlyFilter, Log, All);

class USpatialNetDriver;

namespace SpatialGDK
{
class InitialOnlyFilter
{
public:
	InitialOnlyFilter(USpatialNetDriver* InNetDriver);

	bool HasInitialOnlyData(Worker_EntityId EntityId) const;
	bool HasInitialOnlyDataOrRequest(Worker_EntityId EntityId);
	void FlushRequests();
	void HandleInitialOnlyResponse(const Worker_EntityQueryResponseOp& Op);
	const TArray<ComponentData>& GetInitialOnlyData(Worker_EntityId EntityId) const;

private:
	USpatialNetDriver* NetDriver;
	TSet<Worker_EntityId_Key> PendingInitialOnlyRequests;
	TMap<Worker_EntityId_Key, TArray<ComponentData>> RetrievedInitialOnlyData;
};
} // namespace SpatialGDK
