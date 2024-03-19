// Copyright (C) 2024 Onni Forsblom

#include "BTTask_PatrollerMoveTo.h"
#include "AIController.h"
#include "PatrolDataComponent.h"

EBTNodeResult::Type UBTTask_PatrollerMoveTo::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Try to get the owning AIController
	AAIController* AIOwner = OwnerComp.GetAIOwner();

	// If there is a valid AIController, try to set the filter class
	if (AIOwner) {
		// Try to get the patrol data component from the related pawn
		UPatrolDataComponent* PatrolDataComponent = AIOwner->GetPawn()->GetComponentByClass<UPatrolDataComponent>();

		// If a patrol data component was found, set the filter class accordingly
		if (PatrolDataComponent) {
			TSubclassOf<UNavigationQueryFilter> NavQueryFilterClass
				= PatrolDataComponent->NavQueryFilterClass;
			FilterClass = NavQueryFilterClass ? NavQueryFilterClass : nullptr;
		}
	}
	
	// Execute rest of the task defined in the parent class
	return Super::ExecuteTask(OwnerComp, NodeMemory);
}
