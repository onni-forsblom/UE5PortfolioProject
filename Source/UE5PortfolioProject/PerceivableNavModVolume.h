// Copyright (C) 2024 Onni Forsblom

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PerceivableNavModVolume.generated.h"

// NavModVolume for representing where the AIs think an object is.
// Intended to be added to other actors.
UCLASS()
class UE5PORTFOLIOPROJECT_API APerceivableNavModVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APerceivableNavModVolume(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	// Registering for senses and setting up for
	// a possible parent actor is done here
	virtual void BeginPlay() override;

private:

	AActor* ParentActor;

	uint8 AIsActivelyPerceivingParent = 0;

	class UCustomAIStimuliSourceComponent* AIStimuliSourceComponent;

	class UCustomAIStimuliSourceComponent* ParentAIStimuliSourceComponent;

	UPROPERTY(EditAnywhere)
	class UShapeComponent* ShapeComponent;

	UPROPERTY(EditAnywhere)
	class UNavModifierComponent* NavModifierComponent;

	// NavArea class for where the AIs think the parent is
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UNavArea> ParentNavAreaClass;

	// NavArea class for when an AI spots this actor.
	// (Represents the area where the parent used to be)
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UNavArea> NavAreaClassOnPerceived = nullptr;

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class UAISense>> AISensesToRegisterFor;

	// If succesfully perceived (while not in the same location as the parent)
	// set the appropriate NavArea class. This represents the AI learning
	// that the parent actor has been moved to an unknown location.
	UFUNCTION()
	void HandleOnPerceivedUpdated(AActor* ActorPerceivedBy, FAIStimulus Stimulus);

	// Activates when the parent is perceived or no longer perceived by an AI.
	// If an AI succesfully senses this, reattach this actor to the parent
	// and set the appropriate NavArea class. Else, detach from parent.
	// This represent the AI learning of the parent actor's current location.
	UFUNCTION()
	void HandleParentOnPerceivedUpdated(AActor* ActorPerceivedBy, FAIStimulus Stimulus);

	// Sets up parent's AI stimuli source component and the starting relationship to parent.
	// Does nothing if this actor has no parent.
	void SetUpParent();
};
