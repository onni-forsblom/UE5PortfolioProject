// Copyright (C) 2024 Onni Forsblom

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TeamWorldSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class UE5PORTFOLIOPROJECT_API UTeamWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:

	// Map containing sets of actors belonging to each team by id
	TMap<uint8, TSet<TObjectPtr<AActor>>> ActorsByTeamId;
};
