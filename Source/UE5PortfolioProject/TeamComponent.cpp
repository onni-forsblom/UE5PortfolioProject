// Copyright (C) 2024 Onni Forsblom


#include "TeamComponent.h"
#include "TeamWorldSubsystem.h"

// Sets default values for this component's properties
UTeamComponent::UTeamComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UTeamComponent::BeginPlay()
{
	Super::BeginPlay();

	// If the owner is a pawn, try to set the pawn's controller's team id
	SetControllerTeamId(TeamId);

	// Register owner to the desired team

	TObjectPtr<UTeamWorldSubsystem> TeamWorldSubsystem = GetWorld()->GetSubsystem<UTeamWorldSubsystem>();
	TeamWorldSubsystem->RegisterActorToTeam(TeamId, *GetOwner());
}

void UTeamComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	// If the owner is a pawn, try to set the pawn's controller's team id
	SetControllerTeamId(FGenericTeamId::NoTeam);

	// If the cached world pointer or the team world subsystem is not valid,
	// do nothing

	TObjectPtr<UWorld> World = GetWorld();
	if (!World) {
		return;
	}

	TObjectPtr<UTeamWorldSubsystem> TeamWorldSubsystem = GetWorld()->GetSubsystem<UTeamWorldSubsystem>();
	if (!TeamWorldSubsystem) {
		return;
	}

	// Unregister owner from its team

	TeamWorldSubsystem->UnregisterActorFromTeam(TeamId, *GetOwner());
}

void UTeamComponent::SetControllerTeamId(FGenericTeamId NewTeamId)
{
	TObjectPtr<APawn> OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) {
		return;
	}

	auto TeamAgent = Cast<IGenericTeamAgentInterface>(OwnerPawn->GetController());
	if (TeamAgent) {
		TeamAgent->SetGenericTeamId(NewTeamId);
	}
}