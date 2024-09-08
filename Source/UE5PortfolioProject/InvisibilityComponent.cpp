// Copyright (C) 2024 Onni Forsblom


#include "InvisibilityComponent.h"
#include "ActorVisibilityWorldSubsystem.h"

// Sets default values for this component's properties
UInvisibilityComponent::UInvisibilityComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}


void UInvisibilityComponent::BeginPlay()
{
	Super::BeginPlay();

	// Add this component's owner to the globally available array of actors with invisibility components

	TObjectPtr<UActorVisibilityWorldSubsystem> ActorVisibilityWorldSubsystem = GetWorld()->GetSubsystem<UActorVisibilityWorldSubsystem>();
	ActorVisibilityWorldSubsystem->ActorsWithInvisibilityComponent.Add(GetOwner());
}

void UInvisibilityComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);

	// If the cached world pointer is not valid, do nothing

	TObjectPtr<UWorld> World = GetWorld();
	if (!World) {
		return;
	}

	// Remove this component's owner from the globally available array of actors with invisibility components

	TObjectPtr<UActorVisibilityWorldSubsystem> ActorVisibilityWorldSubsystem = World->GetSubsystem<UActorVisibilityWorldSubsystem>();
	ActorVisibilityWorldSubsystem->ActorsWithInvisibilityComponent.Remove(GetOwner());
}