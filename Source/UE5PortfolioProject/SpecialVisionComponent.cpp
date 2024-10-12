// Copyright (C) 2024 Onni Forsblom


#include "SpecialVisionComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "InvisibilityComponent.h"
#include "ActorVisibilityWorldSubsystem.h"

// Sets default values for this component's properties
USpecialVisionComponent::USpecialVisionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}

void USpecialVisionComponent::ToggleVision()
{
	// Get the owner pawn's player controller if possible.
	// If not, do nothing

	TObjectPtr<APawn> OwningPawn = Cast<APawn>(GetOwner());
	if (!OwningPawn) {
		return;
	}
	TObjectPtr<APlayerController> OwnerPlayerController = Cast<APlayerController>(OwningPawn->GetController());
	if (!OwnerPlayerController) {
		return;
	}

	// Toggle the vision at this point
	// and set the actors visibilities
	bVisionIsActive = !bVisionIsActive;
	SetActorsVisibilities();

	// Then try to get the pawn's camera.
	// If this is not possible, just set the actors visibilities

	TObjectPtr<UCameraComponent> Camera = OwningPawn->GetComponentByClass<UCameraComponent>();
	if (!Camera) {
		SetActorsVisibilities();
		return;
	}

	// Add/remove the special vision weighted blendable to/from the camera's weighted blendables

	TArray<FWeightedBlendable>& WeightedBlendables = Camera->PostProcessSettings.WeightedBlendables.Array;
	if (bVisionIsActive) {
		WeightedBlendables.Add(SpecialVision.WeightedBlendable);
	}
	else {
		for (uint8 i = 0; i < WeightedBlendables.Num(); i++) {
			if (WeightedBlendables[i].Object == SpecialVision.WeightedBlendable.Object) {
				WeightedBlendables.RemoveAt(i);
				break;
			}
		}
	}
}

void USpecialVisionComponent::BeginPlay()
{
	Super::BeginPlay();

	SetActorsVisibilities();
}

void USpecialVisionComponent::SetActorsVisibilities()
{
	// Get the owner pawn's player controller if possible.
	// If not, do nothing

	TObjectPtr<APawn> OwningPawn = Cast<APawn>(GetOwner());
	if (!OwningPawn) {
		return;
	}

	TObjectPtr<APlayerController> OwnerPlayerController = Cast<APlayerController>(OwningPawn->GetController());
	if (!OwnerPlayerController) {
		return;
	}

	// Hide/unhide actors with invisibility component from the player
	// depending on what vision types they are visible to

	TObjectPtr<UActorVisibilityWorldSubsystem> ActorVisibilityWorldSubsystem = GetWorld()->GetSubsystem<UActorVisibilityWorldSubsystem>();
	for (TObjectPtr<AActor> Actor : ActorVisibilityWorldSubsystem->ActorsWithInvisibilityComponent) {
		
		TObjectPtr<UInvisibilityComponent> InvisibilityComponent = Actor->FindComponentByClass<UInvisibilityComponent>();

		// If the special vision is on and the actor is visible to its vision type
		// OR if the special vision is off but the actor can be seen with regular vision,
		// unhide the actor from the player
		if ((bVisionIsActive && InvisibilityComponent->VisionTypesVisibleTo.Contains(StaticCast<TEnumAsByte<EVisionType>>(SpecialVision.VisionType)))
			|| (!bVisionIsActive && InvisibilityComponent->VisionTypesVisibleTo.Contains(EVisionType::Regular))) {
			OwnerPlayerController->HiddenActors.Remove(Actor);
		}
		// Otherwise, hide it from the player
		else {
			OwnerPlayerController->HiddenActors.Add(Actor);
		}
	}
}
