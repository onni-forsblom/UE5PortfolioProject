// Copyright (C) 2024 Onni Forsblom


#include "AIManagerGameInstanceSubsystem.h"
#include "TeamWorldSubsystem.h"
#include "AIController.h"
#include "AIManager.h"

void UAIManagerGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	TObjectPtr<UTeamWorldSubsystem> TeamWorldSubsystem = GetWorld()->GetSubsystem<UTeamWorldSubsystem>();
	if (TeamWorldSubsystem) {
		TeamWorldSubsystem->OnActorRegistered.AddDynamic(this, &UAIManagerGameInstanceSubsystem::HandleTeamActorRegistered);
	}
	FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UAIManagerGameInstanceSubsystem::PostWorldInitialization);
}

void UAIManagerGameInstanceSubsystem::PostWorldInitialization(UWorld* World, const UWorld::InitializationValues IVS)
{
	for (auto AIManager : TeamIdToAIManagerMap) {
		for (auto LocationToInvestigationData : AIManager.Value->LocationToInvestigationDataMap) {
			World->GetTimerManager().ClearTimer(LocationToInvestigationData.Value.DelegationTimerHandle);
		}
		AIManager.Value->AIs.Empty();
		AIManager.Value->LocationToInvestigationDataMap.Empty();
	}

	TObjectPtr<UTeamWorldSubsystem> TeamWorldSubsystem = World->GetSubsystem<UTeamWorldSubsystem>();
	if (!TeamWorldSubsystem) {
		return;
	}
	TeamWorldSubsystem->OnActorRegistered.AddDynamic(this, &UAIManagerGameInstanceSubsystem::HandleTeamActorRegistered);
}

void UAIManagerGameInstanceSubsystem::HandleTeamActorRegistered(uint8 TeamId, AActor* RegisteredActor)
{
	TObjectPtr<AAIController> AIController = GetAIController(*RegisteredActor);

	if (!AIController) {
		return;
	}

	TStrongObjectPtr<UAIManager> AIManager;

	if (TeamIdToAIManagerMap.Contains(TeamId)) {
		AIManager =
			*TeamIdToAIManagerMap
			.Find(TeamId);
	} else {
		AIManager = TStrongObjectPtr<UAIManager>(NewObject<UAIManager>(this));
		TeamIdToAIManagerMap.Add(TeamId, AIManager);
	}

	AIManager->AIs.Add(AIController);
}

TObjectPtr<AAIController> UAIManagerGameInstanceSubsystem::GetAIController(AActor& Actor)
{
	TObjectPtr<APawn> Pawn = Cast<APawn>(&Actor);
	if (!Pawn) {
		return nullptr;
	}

	return Cast<AAIController>(Pawn->GetController());
}