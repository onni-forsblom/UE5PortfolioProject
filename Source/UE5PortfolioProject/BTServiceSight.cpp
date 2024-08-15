// Copyright (C) 2024 Onni Forsblom

#include "BTServiceSight.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "SuspicionLevelEnum.h"
#include "NavigationSystem.h"

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
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComponent = OwnerComp.GetAIOwner()->GetAIPerceptionComponent();

	// If the perception component is invalid,
	// do nothing
	if (!AIPerceptionComponent) {
		return;
	}

	// Get an array of all sighted actors
	TArray<TObjectPtr<AActor>> SightedActors;
	AIPerceptionComponent->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), SightedActors);

	// Get an array of all sighted hostile actors
	TArray<TObjectPtr<AActor>> SightedHostileActors;
	Algo::CopyIf(SightedActors, SightedHostileActors, [&OwnerComp](TObjectPtr<AActor> Actor) {
		return OwnerComp.GetAIOwner()->GetTeamAttitudeTowards(*Actor) == ETeamAttitude::Hostile;
		});

	// If at least one hostile was sighted, get the closest sighted one and the distance to it
	TObjectPtr<AActor> ClosestSightedHostileActor = nullptr;
	float DistanceToClosestActor = TNumericLimits<float>::Max();
	if (!SightedHostileActors.IsEmpty()) {
		ClosestSightedHostileActor =
			UGameplayStatics::FindNearestActor(OwnerComp.GetAIOwner()->GetPawn()->GetActorLocation(), SightedHostileActors, DistanceToClosestActor);
	}

	// If the sighted hostile in the blackboard is different from the closest sighted one,
	// update the value in the blackboard
	TObjectPtr<AActor> LatestSightedHostileActor = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(DetectedHostileActorKey.SelectedKeyName));
	if (LatestSightedHostileActor != ClosestSightedHostileActor) {
		OwnerComp.GetBlackboardComponent()->SetValueAsObject(DetectedHostileActorKey.SelectedKeyName, ClosestSightedHostileActor);

		// If sight of hostile actors was lost
		// set the move to location to the predicted location of the last hostile seen
		if (!ClosestSightedHostileActor) {
			FVector LastHostileLocation = AIPerceptionComponent->GetActorInfo(*LatestSightedHostileActor)->GetStimulusLocation(UAISense::GetSenseID<UAISense_Sight>());
			FVector HostilePredictedLocation = GetActorPredictedLocation(LastHostileLocation, LatestSightedHostileActor);
			OwnerComp.GetBlackboardComponent()->SetValueAsVector(
				MoveToLocationKey.SelectedKeyName,
				HostilePredictedLocation);
		}
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

FVector UBTServiceSight::GetActorPredictedLocation(FVector& ActorPreviousKnownLocation, TObjectPtr<AActor> ActorToPredict) const
{
	// Get the initial predicted location
	FVector PredictedLocation = ActorPreviousKnownLocation + ActorToPredict->GetVelocity().GetSafeNormal() * LocationPredictionDistance;

	// Set up rest of the parameters for a trace
	FHitResult HitResult;
	ECollisionChannel CollisionChannel = ActorToPredict->GetRootComponent()->GetCollisionObjectType();
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(ActorToPredict);

	// If there is something between the latest and predicted location the actor would collide with,
	// set the hit location as the predicted location
	if (GetWorld()->LineTraceSingleByChannel(HitResult, ActorPreviousKnownLocation, PredictedLocation, CollisionChannel, Params)) {
		PredictedLocation = HitResult.Location;
	}
	
	// If the predicted location can be projected to the nav mesh, return to projected location
	FNavLocation ProjectedLocation;
	if (UNavigationSystemV1::GetCurrent(GetWorld())->ProjectPointToNavigation(PredictedLocation, ProjectedLocation)) {
		return ProjectedLocation.Location;
	}

	// Else, just return the previous known location
	return ActorPreviousKnownLocation;
}