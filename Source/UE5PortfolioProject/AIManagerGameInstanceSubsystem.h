// Copyright (C) 2024 Onni Forsblom

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AIManagerGameInstanceSubsystem.generated.h"

class UAIManager;
class AAIController;

/**
 * 
 */
UCLASS()
class UE5PORTFOLIOPROJECT_API UAIManagerGameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:

	// Binds functions for registering a team actor and post world init
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

private:

	TMap<uint8, TStrongObjectPtr<UAIManager>> TeamIdToAIManagerMap;

	// Properly clears AI managers' investigation and AI agent data
	// and binds function to registering a team actor
	void PostWorldInitialization(UWorld* World, const UWorld::InitializationValues IVS);

	// Assigns an AI to an AI manager according to Team Id, if the actor has an AI controller
	UFUNCTION()
	void HandleTeamActorRegistered(uint8 TeamId, AActor* RegisteredActor);

	// Returns actor's AI controller if it has one
	TObjectPtr<AAIController> GetAIController(AActor& Actor);
};