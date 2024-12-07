// Copyright (C) 2024 Onni Forsblom


#include "TeamWorldSubsystem.h"

void UTeamWorldSubsystem::RegisterActorToTeam(uint8 TeamId, AActor& ActorToRegister)
{
	TeamIdToActorsMap.FindOrAdd(TeamId).Add(&ActorToRegister);
	OnActorRegistered.Broadcast(TeamId, &ActorToRegister);
}

void UTeamWorldSubsystem::UnregisterActorFromTeam(uint8 TeamId, AActor& ActorToUnregister)
{
	TSet<TObjectPtr<AActor>>* TeamActors = TeamIdToActorsMap.Find(TeamId);

	if (!TeamActors) {
		return;
	}

	TeamActors->Remove(&ActorToUnregister);

	if (TeamActors->IsEmpty()) {
		TeamIdToActorsMap.Remove(TeamId);
	}
	
	OnActorUnregistered.Broadcast(TeamId, &ActorToUnregister);
}