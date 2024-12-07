// Copyright (C) 2024 Onni Forsblom

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TeamWorldSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FActorTeamRegistering, uint8, TeamId, AActor*, RegisteredActor);

/**
 * 
 */
UCLASS()
class UE5PORTFOLIOPROJECT_API UTeamWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:

	FActorTeamRegistering OnActorRegistered;
	FActorTeamRegistering OnActorUnregistered;

	void RegisterActorToTeam(uint8 TeamId, AActor& ActorToRegister);

	void UnregisterActorFromTeam(uint8 TeamId, AActor& ActorToUnregister);

private:

	TMap<uint8, TSet<TObjectPtr<AActor>>> TeamIdToActorsMap;
};
