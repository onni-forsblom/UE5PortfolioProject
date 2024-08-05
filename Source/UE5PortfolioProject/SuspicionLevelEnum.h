// Copyright (C) 2024 Onni Forsblom

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum ESuspicionLevel : uint8 {
	Normal,
	InvestigatingDistraction,
	InvestigatingEnemy,
	EnemyClearlySpotted
};
