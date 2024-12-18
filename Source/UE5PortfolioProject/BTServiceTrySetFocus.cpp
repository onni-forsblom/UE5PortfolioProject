// Copyright (C) 2024 Onni Forsblom

#include "BTServiceTrySetFocus.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UBTServiceTrySetFocus::UBTServiceTrySetFocus()
{
	// Set default values and blackboard key filters
	RandomDeviation = 0;
	ActorToFocusOnKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTServiceTrySetFocus, ActorToFocusOnKey), AActor::StaticClass());
}

void UBTServiceTrySetFocus::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	TObjectPtr<AActor> ActorToFocusOn = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(ActorToFocusOnKey.SelectedKeyName));

	// If focus has already been set to the desired actor, do nothing
	if (ActorToFocusOn == OwnerComp.GetAIOwner()->GetFocusActor()) {
		return;
	}

	// If a valid actor is obtained with the blackboard key, set focus to it
	// Else, clear focus
	ActorToFocusOn ? OwnerComp.GetAIOwner()->SetFocus(ActorToFocusOn) : OwnerComp.GetAIOwner()->ClearFocus(EAIFocusPriority::Gameplay);
}
