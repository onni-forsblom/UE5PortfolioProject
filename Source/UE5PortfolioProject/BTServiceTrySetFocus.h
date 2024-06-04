// Copyright (C) 2024 Onni Forsblom

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTServiceTrySetFocus.generated.h"

/**
 * 
 */
UCLASS()
class UE5PORTFOLIOPROJECT_API UBTServiceTrySetFocus : public UBTService
{
	GENERATED_BODY()

public:
	UBTServiceTrySetFocus();

protected:

	UPROPERTY(Category = Service, EditAnywhere)
	FBlackboardKeySelector ActorToFocusOnKey;

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
