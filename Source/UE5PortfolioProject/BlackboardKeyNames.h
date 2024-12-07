// Copyright (C) 2024 Onni Forsblom

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "BlackboardKeyNames.generated.h"

/**
 *
 */
UCLASS()
class UE5PORTFOLIOPROJECT_API UBlackboardKeyNames : public UObject
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintPure)
	static FName GetMoveToLocationKeyName() { return "MoveToLocation"; };

	UFUNCTION(BlueprintPure)
	static FName GetSuspicionLevelKeyName() { return "SuspicionLevel"; };

	UFUNCTION(BlueprintPure)
	static FName GetReportingStatusKeyName() { return "ReportingStatus"; };

	UFUNCTION(BlueprintPure)
	static FName GetDetectedHostileActorKeyName() { return "DetectedHostileActor"; };
};