// Copyright (C) 2024 Onni Forsblom

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ActorVisibilityWorldSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class UE5PORTFOLIOPROJECT_API UActorVisibilityWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:

	// Updated in UInvisibilityComponent when play begins and component is destroyed
	TArray<TObjectPtr<AActor>> ActorsWithInvisibilityComponent;
};
