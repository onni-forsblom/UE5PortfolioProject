// Copyright (C) 2024 Onni Forsblom

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GenericTeamAgentInterface.h"
#include "TeamComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UE5PORTFOLIOPROJECT_API UTeamComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	// Id of the team the owner will be assigned to
	UPROPERTY(EditAnywhere)
	FGenericTeamId TeamId = FGenericTeamId(0);

	// Sets default values for this component's properties
	UTeamComponent();

protected:
	// Called when the game starts
	// Makes sure the team world subsystem has a set for the actors belonging to the owner's team
	// Then adds the owner to the set
	virtual void BeginPlay() override;

	// Remove the owner from the team world subsystem's set of actors belonging to the owner's team
	// And if the set becomes empty, removes it from the team world subsystem's map of sets
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

private:

	// If the owner is a pawn, tries to set the pawn's controller's team id
	void SetControllerTeamId(FGenericTeamId NewTeamId);
};
