// Copyright (C) 2024 Onni Forsblom

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VisionType.h"
#include "InvisibilityComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UE5PORTFOLIOPROJECT_API UInvisibilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	// Set of vision types this actor is visible to
	UPROPERTY(EditAnywhere)
	TSet<TEnumAsByte<EVisionType>> VisionTypesVisibleTo;

	// Sets default values for this component's properties
	UInvisibilityComponent();

protected:

	// Called when the game starts
	// Add this component's owner to the globally available array of actors with invisibility components
	virtual void BeginPlay() override;

	// Remove this component's owner from the globally available array of actors with invisibility components
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
};
