// Copyright (C) 2024 Onni Forsblom


#include "BaseCharacter.h"
#include "TeamComponent.h"

ABaseCharacter::ABaseCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GetComponentByClass<USkeletalMeshComponent>()->SetRenderCustomDepth(true);
	TeamComponent = CreateDefaultSubobject<UTeamComponent>("TeamComponent");
}

FGenericTeamId ABaseCharacter::GetGenericTeamId() const
{
	if (TeamComponent) {
		return TeamComponent->TeamId;
	}
	return FGenericTeamId::NoTeam;
}
