// Copyright Epic Games, Inc. All Rights Reserved.

#include "UE5PortfolioProjectCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "CustomCharacterMovementComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Kismet/KismetMathLibrary.h"
#include "SpecialVisionComponent.h"


//////////////////////////////////////////////////////////////////////////
// AUE5PortfolioProjectCharacter

AUE5PortfolioProjectCharacter::AUE5PortfolioProjectCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCustomCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	AIStimuliSourceComponent = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("AIStimuliSource"));
	AIStimuliSourceComponent->RegisterForSense(UAISense_Sight::StaticClass());
	AIStimuliSourceComponent->RegisterWithPerceptionSystem();

	SpecialVisionComponent = CreateDefaultSubobject<USpecialVisionComponent>(TEXT("SpecialVisionComponent"));
}

void AUE5PortfolioProjectCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (TObjectPtr<APlayerController> PlayerController = Cast<APlayerController>(Controller))
	{
		if (TObjectPtr<UEnhancedInputLocalPlayerSubsystem> Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

bool AUE5PortfolioProjectCharacter::CanJumpInternal_Implementation() const
{
	// If the character has the custom movement component and can wall jump
	// return true
	TObjectPtr<UCustomCharacterMovementComponent> CustomMovementComponent = Cast<UCustomCharacterMovementComponent>(GetCharacterMovement());
	if (CustomMovementComponent
		&& CustomMovementComponent->CanWallJump()){
		return true;
	}


	// Else, do the normal jump check
	return Super::CanJumpInternal_Implementation();
}

bool AUE5PortfolioProjectCharacter::ShouldMoveInFacedDirection(TObjectPtr<UCustomCharacterMovementComponent> CustomMovementComponent, FVector2D& MovementVector)
{
	// Return true if the character has performed a wall jump, they are still in the air
	// and the movement input has not been changed beyond a certain threshold
	return CustomMovementComponent->WallJumpDirection != FVector::ZeroVector
		&& CustomMovementComponent->IsFalling()
		&& FVector2D::Distance(PreviousMovementVector.GetSafeNormal(), MovementVector.GetSafeNormal()) < MoveInFacedDirectionThreshold;
}

void AUE5PortfolioProjectCharacter::ShootProjectile()
{
	// Spawn the projectile in the direction of control rotation
	// and in a certain location from this actor according to control rotation
	FRotator SpawnRotation = GetControlRotation();
	FVector SpawnLocation = GetActorLocation()
		+ SpawnRotation.Vector() * ProjectileSpawnLocationOffset.X
		+ UKismetMathLibrary::GetRightVector(SpawnRotation) * ProjectileSpawnLocationOffset.Y
		+ UKismetMathLibrary::GetUpVector(SpawnRotation) * ProjectileSpawnLocationOffset.Z;
	
	// Set this actor as the projectile's instigator
	FActorSpawnParameters ActorSpawnParameters;
	ActorSpawnParameters.Instigator = this;

	GetWorld()->SpawnActor<AActor>(ProjectileActorClass, SpawnLocation, SpawnRotation, ActorSpawnParameters);
}

//////////////////////////////////////////////////////////////////////////
// Input

void AUE5PortfolioProjectCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AUE5PortfolioProjectCharacter::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AUE5PortfolioProjectCharacter::HandleMoveStop);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AUE5PortfolioProjectCharacter::Look);

		//Shooting a projectile
		EnhancedInputComponent->BindAction(ProjectileShootAction, ETriggerEvent::Triggered, this, &AUE5PortfolioProjectCharacter::ShootProjectile);

		//Toggling special vision
		EnhancedInputComponent->BindAction(SpecialVisionToggleAction, ETriggerEvent::Triggered, SpecialVisionComponent, &USpecialVisionComponent::ToggleVision);

		// Teleporting & rewinding with the custom movement component
		UCustomCharacterMovementComponent* CustomMovementComponent = Cast<UCustomCharacterMovementComponent>(GetCharacterMovement());
		if (CustomMovementComponent) {
			EnhancedInputComponent->BindAction(TeleportAction, ETriggerEvent::Triggered, CustomMovementComponent, &UCustomCharacterMovementComponent::TryTeleport);
			EnhancedInputComponent->BindAction(RewindAction, ETriggerEvent::Triggered, CustomMovementComponent, &UCustomCharacterMovementComponent::StartRewind);
		}

	}

}

void AUE5PortfolioProjectCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// Check if movement input should just be added to the character's faced direction
	// after performing a wall jump
	TObjectPtr<UCustomCharacterMovementComponent> CustomMovementComponent = Cast<UCustomCharacterMovementComponent>(GetCharacterMovement());
	if (ShouldMoveInFacedDirection(CustomMovementComponent, MovementVector)) {
		AddMovementInput(CustomMovementComponent->WallJumpDirection);
		return;
	}
	
	// When performing normal movement, make sure the wall jump direction is zeroed
	CustomMovementComponent->WallJumpDirection = FVector::ZeroVector;

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}

	// Store the movement vector for potential comparison later
	PreviousMovementVector = MovementVector;
}

void AUE5PortfolioProjectCharacter::HandleMoveStop(const FInputActionValue& Value)
{
	// On stopping movement input, set the previous movement vector to zero
	PreviousMovementVector = FVector2D::ZeroVector;
}

void AUE5PortfolioProjectCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}