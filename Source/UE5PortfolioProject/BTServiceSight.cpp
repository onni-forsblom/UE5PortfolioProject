// Copyright (C) 2024 Onni Forsblom

#include "BTServiceSight.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "SuspicionLevelEnum.h"

UBTServiceSight::UBTServiceSight()
{
	// Set default values for this service and filters
	RandomDeviation = 0;
	DetectedHostileActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTServiceSight, DetectedHostileActorKey), AActor::StaticClass());
	MoveToLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTServiceSight, MoveToLocationKey));
	SuspicionLevelKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UBTServiceSight, SuspicionLevelKey), StaticEnum<ESuspicionLevel>());
	SecondsSinceEnemyInvestigationStartKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTServiceSight, SecondsSinceEnemyInvestigationStartKey));
}

void UBTServiceSight::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	UAIPerceptionComponent* AIPerceptionComponent = OwnerComp.GetAIOwner()->GetAIPerceptionComponent();

	// If the perception component is invalid,
	// do nothing
	if (!AIPerceptionComponent) {
		return;
	}

	// Get an array of all sighted actors
	TArray<AActor*> SightedActors;
	AIPerceptionComponent->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), SightedActors);

	// Get an array of all sighted hostile actors
	TArray<AActor*> SightedHostileActors;
	Algo::CopyIf(SightedActors, SightedHostileActors, [&OwnerComp](AActor* Actor) {
		return OwnerComp.GetAIOwner()->GetTeamAttitudeTowards(*Actor) == ETeamAttitude::Hostile;
		});

	// If at least one hostile was sighted, get the closest one and the distance to it
	// and set the values for the detected hostile and the move to location
	AActor* ClosestSightedHostileActor = nullptr;
	float DistanceToClosestActor = TNumericLimits<float>::Max();
	if (!SightedHostileActors.IsEmpty()) {
		ClosestSightedHostileActor = 
			UGameplayStatics::FindNearestActor(OwnerComp.GetAIOwner()->GetPawn()->GetActorLocation(), SightedHostileActors, DistanceToClosestActor);
		OwnerComp.GetBlackboardComponent()->SetValueAsObject(DetectedHostileActorKey.SelectedKeyName, ClosestSightedHostileActor);
		OwnerComp.GetBlackboardComponent()->SetValueAsVector(MoveToLocationKey.SelectedKeyName, ClosestSightedHostileActor->GetActorLocation());
	}
	// Else, clear the value for the detected hostile
	else {
		OwnerComp.GetBlackboardComponent()->ClearValue(DetectedHostileActorKey.SelectedKeyName);
	}

	float SecondsSinceEnemyInvestigationStart =
		OwnerComp.GetBlackboardComponent()->GetValueAsFloat(SecondsSinceEnemyInvestigationStartKey.SelectedKeyName);

	// Handle reactions to stimuli according to each suspicion level
	switch (OwnerComp.GetBlackboardComponent()->GetValueAsEnum(SuspicionLevelKey.SelectedKeyName))
	{
	case ESuspicionLevel::Normal:
	case ESuspicionLevel::InvestigatingDistraction:
		// Start investigating a hostile if one is sighted when the suspicion level is normal
		if (ClosestSightedHostileActor) {
			OwnerComp.GetBlackboardComponent()->SetValueAsEnum(SuspicionLevelKey.SelectedKeyName, ESuspicionLevel::InvestigatingEnemy);
		}
		break;
	case ESuspicionLevel::InvestigatingEnemy:
		// Increment the timer counting up from the start of the investigation
		OwnerComp.GetBlackboardComponent()->SetValueAsFloat(
			SecondsSinceEnemyInvestigationStartKey.SelectedKeyName, SecondsSinceEnemyInvestigationStart += DeltaSeconds);

		// If a hostile is sighted from close enough 
		// and a set time has passed since the initial sighting,
		// consider the hostile fully sighted
		if (ClosestSightedHostileActor
			&& DistanceToClosestActor <= HostileClearlySpottedSightRange
			&& SecondsSinceEnemyInvestigationStart >= SecondsFromInvestigationStartToFullySpot) {
			OwnerComp.GetBlackboardComponent()->SetValueAsEnum(SuspicionLevelKey.SelectedKeyName, ESuspicionLevel::EnemyClearlySpotted);
		}
		break;
	default:
		break;
	}
}
