// Copyright (C) 2024 Onni Forsblom

#pragma once

#include "CoreMinimal.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "CustomAIStimuliSourceComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPerceivedUpdated, AActor*, ActorPerceivedBy, FAIStimulus, Stimulus);

/**
 * An AIPerceptionStimuliSourceComponent with a delegate
 * that is supposed to be activated when perceived by an actor
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UE5PORTFOLIOPROJECT_API UCustomAIStimuliSourceComponent : public UAIPerceptionStimuliSourceComponent
{
	GENERATED_BODY()
	
public:

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnPerceivedUpdated OnPerceivedUpdated;
};
