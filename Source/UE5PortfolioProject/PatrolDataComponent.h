// Copyright (C) 2024 Onni Forsblom

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PatrolDataComponent.generated.h"

UENUM(BlueprintType)
enum EPatrolSpotDirection : uint8 {
	BothWays,
	FlipRotationOnReturn,
	ForwardsOnly,
	BackwardsOnly
};

USTRUCT(BlueprintType)
struct FPatrolSpotData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditInstanceOnly)
	TEnumAsByte<EPatrolSpotDirection> PatrolSpotDirection = EPatrolSpotDirection::BothWays;

	UPROPERTY(BlueprintReadOnly, EditInstanceOnly)
	FVector TargetLocation;

	UPROPERTY(BlueprintReadOnly, EditInstanceOnly)
	float ZRotationTarget;

	UPROPERTY(BlueprintReadOnly, EditInstanceOnly)
	float SecondsToWait = 0;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UE5PORTFOLIOPROJECT_API UPatrolDataComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, EditInstanceOnly)
	TArray<FPatrolSpotData> PatrolSpotsData;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	bool bShouldPingPong = true;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TSubclassOf<class UNavigationQueryFilter> NavQueryFilterClass;

	// Sets default values for this component's properties
	UPatrolDataComponent();
};
