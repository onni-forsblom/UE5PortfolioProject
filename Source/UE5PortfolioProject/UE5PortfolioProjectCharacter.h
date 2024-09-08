 // Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "BaseCharacter.h"
#include "UE5PortfolioProjectCharacter.generated.h"


UCLASS(config=Game)
class AUE5PortfolioProjectCharacter : public ABaseCharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> FollowCamera;

	/** Projectile to fire with projectile shoot action*/
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	TSubclassOf<AActor> ProjectileActorClass;

	/** How far in each direction to spawn the projectile away from this actor (forwards, right & up) */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	FVector ProjectileSpawnLocationOffset = FVector(0, 50, 75);

	TObjectPtr<class UAIPerceptionStimuliSourceComponent> AIStimuliSourceComponent;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputMappingContext> DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> LookAction;

	/** Teleport Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> TeleportAction;

	/** Rewind Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> RewindAction;

	/* Projectile shooting Input Action*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> ProjectileShootAction;

	/* Special vision toggle Input Action*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> SpecialVisionToggleAction;

	UPROPERTY(EditAnywhere)
	class USpecialVisionComponent* SpecialVisionComponent;

	/* The amount by which movement input can change before no longer following faced direction after a wall jump */
	UPROPERTY(EditDefaultsOnly, Category = Input, meta = (ClampMin = "0"))
	float MoveInFacedDirectionThreshold = 0.1f;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for releasing movement input */
	void HandleMoveStop(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
			

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	AUE5PortfolioProjectCharacter(const FObjectInitializer& ObjectInitializer);

	// To add mapping context
	virtual void BeginPlay();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

private:

	virtual bool CanJumpInternal_Implementation() const;

	bool ShouldMoveInFacedDirection(TObjectPtr<class UCustomCharacterMovementComponent> CustomMovementComponent, FVector2D& MovementVector);

	void ShootProjectile();

	FVector2D PreviousMovementVector;
};

