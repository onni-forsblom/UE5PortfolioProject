// Copyright (C) 2024 Onni Forsblom

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
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

	// Gets the team attitude towards the other actor via team component
	// If the other actor has no team component, uses the super method
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComponent;

private:

	UFUNCTION(BlueprintCallable)
	void HandleNoiseStimuli(FVector NoiseLocation);
};
