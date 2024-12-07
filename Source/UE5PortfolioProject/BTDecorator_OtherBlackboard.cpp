// Copyright (C) 2024 Onni Forsblom


#include "BTDecorator_OtherBlackboard.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_OtherBlackboard::UBTDecorator_OtherBlackboard(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NodeName = "Other Blackboard Based Condition";
	bCreateNodeInstance = true;
}

bool UBTDecorator_OtherBlackboard::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const UBlackboardComponent* BlackboardComp = GetOtherBlackboardComponent(OwnerComp);
	return BlackboardComp && EvaluateOnBlackboard(*BlackboardComp);
}

EBlackboardNotificationResult UBTDecorator_OtherBlackboard::OnBlackboardKeyValueChange(const UBlackboardComponent& Blackboard, FBlackboard::FKey ChangedKeyID)
{
	return Super::OnBlackboardKeyValueChange(*OwnerBB, ChangedKeyID);
}

void UBTDecorator_OtherBlackboard::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	TObjectPtr<UBlackboardComponent> OtherBBComp = GetOtherBlackboardComponent(OwnerComp);
	if (OtherBBComp)
	{
		OwnerBB = OwnerComp.GetBlackboardComponent();
		OtherBBComp->RegisterObserver(BlackboardKey.GetSelectedKeyID(), this, FOnBlackboardChangeNotification::CreateUObject(this, &UBTDecorator_BlackboardBase::OnBlackboardKeyValueChange));
	}
}

void UBTDecorator_OtherBlackboard::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	TObjectPtr<UBlackboardComponent> OtherBBComp = GetOtherBlackboardComponent(OwnerComp);
	if (OtherBBComp)
	{
		OtherBBComp->UnregisterObserversFrom(this);
	}
}

TObjectPtr<UBlackboardComponent> UBTDecorator_OtherBlackboard::GetOtherBlackboardComponent(UBehaviorTreeComponent& OwnerComp) const
{
	TObjectPtr<UBlackboardComponent> BBComp = OwnerComp.GetBlackboardComponent();
	TObjectPtr<APawn> OtherPawn = Cast<APawn>(BBComp->GetValueAsObject(OtherActorKey.SelectedKeyName));
	if (!OtherPawn) {
		return nullptr;
	}
	TObjectPtr<AAIController> OtherAIController = Cast<AAIController>(OtherPawn->GetController());
	if (!OtherAIController) {
		return nullptr;
	}
	return OtherAIController->GetBlackboardComponent();
}