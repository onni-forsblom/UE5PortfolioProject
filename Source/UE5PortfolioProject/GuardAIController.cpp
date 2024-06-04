// Copyright (C) 2024 Onni Forsblom

#include "GuardAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

AGuardAIController::AGuardAIController()
{
	// Create and set the AIPerceptionComponent
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
	SetPerceptionComponent(*AIPerceptionComponent);

	// Create and configure the senses
	UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("AISightConfig"));
	AIPerceptionComponent->ConfigureSense(*SightConfig);
}

FGenericTeamId AGuardAIController::GetGenericTeamId() const
{
	auto TeamAgent = Cast<IGenericTeamAgentInterface>(GetPawn());
	return TeamAgent ? TeamAgent->GetGenericTeamId() : FGenericTeamId::NoTeam;
}