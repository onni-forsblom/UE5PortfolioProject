// Copyright (C) 2024 Onni Forsblom

#include "GuardAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BlackboardKeyNames.h"
#include "SuspicionLevelEnum.h"
#include "TeamComponent.h"
#include "TeamWorldSubsystem.h"

AGuardAIController::AGuardAIController()
{
	// Create and set the AIPerceptionComponent
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
	SetPerceptionComponent(*AIPerceptionComponent);

	// Create and configure the senses
	TObjectPtr<UAISenseConfig_Sight> SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("AISightConfig"));
	AIPerceptionComponent->ConfigureSense(*SightConfig);

	TObjectPtr<UAISenseConfig_Hearing> HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("AIHearingConfig"));
	AIPerceptionComponent->ConfigureSense(*HearingConfig);
}

ETeamAttitude::Type AGuardAIController::GetTeamAttitudeTowards(const AActor& Other) const
{
	TObjectPtr<UTeamComponent> OtherTeamComponent = Other.GetComponentByClass<UTeamComponent>();
	if (!OtherTeamComponent) {
		return Super::GetTeamAttitudeTowards(Other);
	}
	return FGenericTeamId::GetAttitude(GetGenericTeamId(), OtherTeamComponent->TeamId);
}

void AGuardAIController::HandleNoiseStimuli(FVector NoiseLocation)
{
	// If the AI is not suspicious of anything and a suspicious noise is heard,
	// set the suspicion level to investigating distraction and the move to location
	if (GetBlackboardComponent()->GetValueAsEnum(UBlackboardKeyNames::GetSuspicionLevelKeyName())
		== ESuspicionLevel::Normal) {
		GetBlackboardComponent()->SetValueAsEnum(UBlackboardKeyNames::GetSuspicionLevelKeyName(), ESuspicionLevel::InvestigatingDistraction);
		GetBlackboardComponent()->SetValueAsVector(UBlackboardKeyNames::GetMoveToLocationKeyName(), NoiseLocation);
	}
}