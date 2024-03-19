// Copyright (C) 2024 Onni Forsblom

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "BTTask_PatrollerMoveTo.generated.h"

/**
 * 
 */
UCLASS()
class UE5PORTFOLIOPROJECT_API UBTTask_PatrollerMoveTo : public UBTTask_MoveTo
{
	GENERATED_BODY()

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
