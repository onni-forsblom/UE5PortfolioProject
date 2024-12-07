// Copyright (C) 2024 Onni Forsblom

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Decorators/BTDecorator_Blackboard.h"
#include "BTDecorator_OtherBlackboard.generated.h"

/**
 * 
 */
UCLASS()
class UE5PORTFOLIOPROJECT_API UBTDecorator_OtherBlackboard : public UBTDecorator_Blackboard
{
	GENERATED_BODY()

	UBTDecorator_OtherBlackboard(const FObjectInitializer& ObjectInitializer);

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

	virtual EBlackboardNotificationResult OnBlackboardKeyValueChange(const UBlackboardComponent& Blackboard, FBlackboard::FKey ChangedKeyID) override;

protected:

	// The actor whose blackboard to evaluate
	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector OtherActorKey;

	// Register observer for other actor's blackboard
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// Unregister observer for other actor's blackboard
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:

	// Gets the blackboard component of the other actor
	TObjectPtr<UBlackboardComponent> GetOtherBlackboardComponent(UBehaviorTreeComponent& OwnerComp) const;

	TObjectPtr<UBlackboardComponent> OwnerBB;
};
