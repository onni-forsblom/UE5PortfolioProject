// Copyright (C) 2024 Onni Forsblom

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VisionType.h"
#include "SpecialVisionComponent.generated.h"


USTRUCT(BlueprintType)
struct FSpecialVision
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FWeightedBlendable WeightedBlendable;

	UPROPERTY(EditAnywhere)
	TEnumAsByte<EVisionType> VisionType;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UE5PORTFOLIOPROJECT_API USpecialVisionComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	// Sets default values for this component's properties
	USpecialVisionComponent();

	// Add/remove the special vision material from the player's camera
	// depending on if it is already in there
	void ToggleVision();

protected:

	// Sets actors visibilities at the beginning
	virtual void BeginPlay() override;

private:

	bool bVisionIsActive = false;

	UPROPERTY(EditAnywhere)
	FSpecialVision SpecialVision;

	// Hide/unhide actors with invisibility component from the player
	void SetActorsVisibilities();
};