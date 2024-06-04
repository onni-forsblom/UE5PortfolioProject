// Copyright (C) 2024 Onni Forsblom

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "GuardAIController.generated.h"

/**
 * 
 */
UCLASS()
class UE5PORTFOLIOPROJECT_API AGuardAIController : public AAIController
{
	GENERATED_BODY()
	
public:

	AGuardAIController();

	virtual FGenericTeamId GetGenericTeamId() const override;

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAIPerceptionComponent* AIPerceptionComponent;
};
