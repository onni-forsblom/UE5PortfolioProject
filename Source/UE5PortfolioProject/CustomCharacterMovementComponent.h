// Copyright (C) 2024 Onni Forsblom

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CustomCharacterMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class UE5PORTFOLIOPROJECT_API UCustomCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	class FSavedMove_Custom : public FSavedMove_Character {

		typedef FSavedMove_Character Super;

		uint8 bSavedWantsToTeleport : 1;
		FVector SavedTeleportDestination;

		uint8 bSavedWantsToRewind : 1;
		float SavedRewindProgress;

		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;
		virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData) override;
		virtual void PrepMoveFor(class ACharacter* C) override;
	};

	class FNetworkPredictionData_Client_Custom : public FNetworkPredictionData_Client_Character
	{
	public:
		FNetworkPredictionData_Client_Custom(const UCharacterMovementComponent& ClientMovement);

		typedef FNetworkPredictionData_Client_Character Super;

		virtual FSavedMovePtr AllocateNewMove() override;
	};
	
public:
	/* Maximum teleport distance */
	UPROPERTY(EditDefaultsOnly, Category = Teleportation)
	float TeleportDistance = 1000;

	/* Distance to check for land below when trying to teleport past objects */
	UPROPERTY(EditDefaultsOnly, Category = Teleportation, meta = (ClampMin = "0"))
	float LandCheckDistance = 3000;

	/* Number of times the character's status is recorded per second */
	UPROPERTY(EditDefaultsOnly, Category = Rewinding, meta = (ClampMin = "0"))
	uint8 RewindCapturesPerSecond = 30;

	/** The period of time in seconds for which to record data for rewinding */
	UPROPERTY(EditDefaultsOnly, Category = Rewinding, meta = (ClampMin = "0"))
	float RewindCaptureLengthSeconds = 2;

	/* The time in seconds to perform a full rewind */
	UPROPERTY(EditDefaultsOnly, Category = Rewinding, meta = (ClampMin = "0"))
	float RewindFullPerformLengthSeconds = 2;

	/* The minimum speed at which the character will be launched from a wall */
	UPROPERTY(EditDefaultsOnly, Category = WallJumping, meta = (ClampMin = "0"))
	float MinWallJumpSpeed = 150;

	/* The multiple of which the wall jump angle will be */
	UPROPERTY(EditDefaultsOnly, Category = WallJumping, meta = (ClampMin = "0"))
	float WallJumpAngleFactor = 22.5f;

	/* The maximum angle which can be faced away from a wall to perform a wall jump */
	UPROPERTY(EditDefaultsOnly, Category = WallJumping, meta = (ClampMin = "0", ClampMax="89.99999"))
	float MaxWallJumpAngle = 78.75f;

	/* The scale by which the character's capsule radius will be multiplied to check if they are close enough to the wall for a wall jump */
	UPROPERTY(EditDefaultsOnly, Category = WallJumping, meta = (ClampMin = "1.005"))
	float WallJumpCapsuleRadiusScale = 1.5f;

	mutable FVector WallJumpDirection = FVector::ZeroVector;

	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	virtual void UpdateFromCompressedFlags(uint8 Flags) override;

	void OnMovementUpdated(float DeltaTime, const FVector& OldLocation, const FVector& OldVelocity) override;

	void TryTeleport();

	void StartRewind();

	bool CanWallJump() const;

	virtual bool DoJump(bool bReplayingMoves) override;

protected:
	virtual void InitializeComponent() override;

private:
	bool bWantsToTeleport;
	FVector TeleportDestination;

	bool bWantsToRewind;
	FTimerHandle RewindCaptureTimer;
	TArray<FVector> RewindTargetLocations;
	FVector RewindTargetLocation;

	float RewindProgress = 0;

	void CaptureRewindData();

	void SetTargetRewindLocation();

	UFUNCTION(Server, Reliable)
	void Server_SendTeleportDestination(FVector SentTeleportDestination);

	UFUNCTION(Server, Reliable)
	void Server_SendRewindData(FVector SentRewindData);

	void Teleport(bool bUsedLineTrace = false);
};