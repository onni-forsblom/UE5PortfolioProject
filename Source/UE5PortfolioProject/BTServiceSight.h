// Copyright (C) 2024 Onni Forsblom

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTServiceSight.generated.h"

/**
 * 
 */
UCLASS()
class UE5PORTFOLIOPROJECT_API UBTServiceSight : public UBTService
{
	GENERATED_BODY()

public:
	UBTServiceSight();

protected:
	
	UPROPERTY(Category = Service, EditAnywhere, meta = (ClampMin = "0"))
	float HostileClearlySpottedSightRange = 1400;

	UPROPERTY(Category = Service, EditAnywhere, meta = (ClampMin = "0"))
	float SecondsFromInvestigationStartToFullySpot = 1;

	UPROPERTY(Category = Service, EditAnywhere)
	FBlackboardKeySelector DetectedHostileActorKey;

	UPROPERTY(Category = Service, EditAnywhere)
	FBlackboardKeySelector MoveToLocationKey;

	UPROPERTY(Category = Service, EditAnywhere)
	FBlackboardKeySelector SuspicionLevelKey;

	UPROPERTY(Category = Service, EditAnywhere)
	FBlackboardKeySelector SecondsSinceEnemyInvestigationStartKey;

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};