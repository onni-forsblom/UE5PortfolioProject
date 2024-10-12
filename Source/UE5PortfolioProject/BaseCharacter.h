// Copyright (C) 2024 Onni Forsblom

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GenericTeamAgentInterface.h"
#include "BaseCharacter.generated.h"

UCLASS()
class UE5PORTFOLIOPROJECT_API ABaseCharacter : public ACharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:

	// Component for setting the character's team and keeping track of it
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UTeamComponent> TeamComponent;	

	// Sets default values for this character's properties
	// Sets the character's skeletal mesh to render custom depth
	ABaseCharacter(const FObjectInitializer& ObjectInitializer);

	// Returns the team id from the team component
	// If it is invalid, returns team id for no team
	virtual FGenericTeamId GetGenericTeamId() const override;
};