// Copyright (C) 2024 Onni Forsblom

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GenericTeamAgentInterface.h"
#include "BaseCharacter.generated.h"

UCLASS()
class UE5PORTFOLIOPROJECT_API ABaseCharacter : public ACharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FGenericTeamId TeamId = FGenericTeamId(0);

	// Sets default values for this character's properties
	// Sets the character's skeletal mesh to render custom depth
	ABaseCharacter(const FObjectInitializer& ObjectInitializer);

	virtual FGenericTeamId GetGenericTeamId() const override { return TeamId; };

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
