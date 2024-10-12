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

	// Make sure the team world subsystem has a set for the actors belonging to the owner's team
	// Then add the owner to the set

	TObjectPtr<UTeamWorldSubsystem> TeamWorldSubsystem = GetWorld()->GetSubsystem<UTeamWorldSubsystem>();
	TSet<TObjectPtr<AActor>>& TeamActors =  TeamWorldSubsystem->ActorsByTeamId.FindOrAdd(TeamId.GetId());
	TeamActors.Add(GetOwner());

	// If the owner is a pawn, try to set the pawn's controller's team id
	SetControllerTeamId(TeamId);
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

	// Remove the owner from the team world subsystem's set of actors belonging to the owner's team
	// (If there is a valid set)

	TSet<TObjectPtr<AActor>>* TeamActors = TeamWorldSubsystem->ActorsByTeamId.Find(TeamId.GetId());

	if (!TeamActors) {
		return;
	}

	TeamActors->Remove(GetOwner());

	// If the set is now empty, remove it from the team world subsystem's map of sets
	if (TeamActors->IsEmpty()) {
		TeamWorldSubsystem->ActorsByTeamId.Remove(TeamId.GetId());
	}
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