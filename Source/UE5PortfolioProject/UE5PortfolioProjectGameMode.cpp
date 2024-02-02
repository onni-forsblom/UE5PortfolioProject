// Copyright Epic Games, Inc. All Rights Reserved.

#include "UE5PortfolioProjectGameMode.h"
#include "UE5PortfolioProjectCharacter.h"
#include "UObject/ConstructorHelpers.h"

AUE5PortfolioProjectGameMode::AUE5PortfolioProjectGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
