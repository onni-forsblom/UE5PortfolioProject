// Copyright (C) 2024 Onni Forsblom

#include "CustomCharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"

bool UCustomCharacterMovementComponent::FSavedMove_Custom::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	// Check if the current move and the next move can be combined to save bandwidth
	FSavedMove_Custom* NewCustomMove = static_cast<FSavedMove_Custom*>(NewMove.Get());

	// If the moves do not have the same teleport or rewind status,
	// they cannot be combined
	if (bSavedWantsToTeleport != NewCustomMove->bSavedWantsToTeleport
		|| bSavedWantsToRewind != NewCustomMove->bSavedWantsToRewind) {
		return false;
	}

	return Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void UCustomCharacterMovementComponent::FSavedMove_Custom::Clear()
{
	// Reset all variables including teleporting and rewinding
	Super::Clear();

	bSavedWantsToTeleport = 0;
	SavedTeleportDestination = FVector::ZeroVector;

	bSavedWantsToRewind = 0;
	SavedRewindProgress = 0;
}

uint8 UCustomCharacterMovementComponent::FSavedMove_Custom::GetCompressedFlags() const
{
	// Set the regular compressed flags
	uint8 Result = Super::GetCompressedFlags();

	// If we want to teleport or rewind, 
	// set the custom flag for teleporting & rewinding
	if (bSavedWantsToTeleport) {
		Result |= FLAG_Custom_0;
	}
	if (bSavedWantsToRewind) {
		Result |= FLAG_Custom_1;
	}

	// Return the flags
	return Result;
}

void UCustomCharacterMovementComponent::FSavedMove_Custom::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData)
{
	// Set the saved move with the variables from the movement component
	Super::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);
	UCustomCharacterMovementComponent* CharacterMovement = Cast<UCustomCharacterMovementComponent>(C->GetCharacterMovement());

	// Include teleportation
	bSavedWantsToTeleport = CharacterMovement->bWantsToTeleport;
	SavedTeleportDestination = CharacterMovement->TeleportDestination;

	// Include rewinding
	bSavedWantsToRewind = CharacterMovement->bWantsToRewind;
	SavedRewindProgress = CharacterMovement->RewindProgress;
}

void UCustomCharacterMovementComponent::FSavedMove_Custom::PrepMoveFor(ACharacter* C)
{
	// Set the character movement component with variables from saved move
	Super::PrepMoveFor(C);
	UCustomCharacterMovementComponent* CharacterMovement = Cast<UCustomCharacterMovementComponent>(C->GetCharacterMovement());

	// Include teleportation
	CharacterMovement->bWantsToTeleport = bSavedWantsToTeleport;
	CharacterMovement->TeleportDestination = SavedTeleportDestination;

	// Include rewinding
	CharacterMovement->bWantsToRewind = bSavedWantsToRewind;
	CharacterMovement->RewindProgress = SavedRewindProgress;
}

UCustomCharacterMovementComponent::FNetworkPredictionData_Client_Custom::FNetworkPredictionData_Client_Custom(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr UCustomCharacterMovementComponent::FNetworkPredictionData_Client_Custom::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_Custom());
}

FNetworkPredictionData_Client* UCustomCharacterMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != nullptr);

	if (ClientPredictionData == nullptr)
	{
		UCustomCharacterMovementComponent* MutableThis = const_cast<UCustomCharacterMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_Custom(*this);
	}

	return ClientPredictionData;
}

void UCustomCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	// Update movement statuses from flags
	Super::UpdateFromCompressedFlags(Flags);

	// Include teleportion
	bWantsToTeleport = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;

	// Include rewind
	bWantsToRewind = (Flags & FSavedMove_Character::FLAG_Custom_1) != 0;
}

void UCustomCharacterMovementComponent::OnMovementUpdated(float DeltaTime, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaTime, OldLocation, OldVelocity);

	if (bWantsToRewind) {
		// Set the target location if locally controlled
		if (PawnOwner->IsLocallyControlled()) {
			SetTargetRewindLocation();
			// Send the location to server if this is the client.
			if (PawnOwner->GetLocalRole() == ENetRole::ROLE_AutonomousProxy) {
				Server_SendRewindData(RewindTargetLocation);
			}
		}
		// If the player wants to rewind, move the owning actor to the target location
		const bool bSweep = false;
		FHitResult* OutSweepHitResult = nullptr;
		GetOwner()->SetActorLocation(RewindTargetLocation, bSweep, OutSweepHitResult, ETeleportType::TeleportPhysics);
	}

	// If the player wants to teleport, teleport to the defined destination
	// and stop trying to teleport
	if (bWantsToTeleport) {
		bWantsToTeleport = false;

		const bool bSweep = false;
		FHitResult* OutSweepHitResult = nullptr;
		GetOwner()->SetActorLocation(TeleportDestination, bSweep, OutSweepHitResult, ETeleportType::TeleportPhysics);
	}
}

void UCustomCharacterMovementComponent::TryTeleport()
{
	// Get the player view point for line-trace later
	// and the rotation for the direction to the teleport target
	FVector ViewPointLocation;
	FRotator ViewPointRotation;
	GetController()->GetPlayerViewPoint(ViewPointLocation, ViewPointRotation);

	// Calculate the (initial) teleport destination
	TeleportDestination =
		GetOwner()->GetActorLocation() +
		ViewPointRotation.Vector() * TeleportDistance;

	// Check if the teleport destination works and possibly adjust it
	bool bCanTeleportToLocation = GetWorld()->FindTeleportSpot(GetOwner(), TeleportDestination, GetOwner()->GetActorRotation());

	// Do a line-trace from the player's viewpoint to the direction of the camera
	FHitResult StraightMoveResult;
	const uint16 LineTraceDistance = 1500;
	FVector LineTraceEnd = ViewPointLocation + ViewPointRotation.Vector() * LineTraceDistance;
	GetWorld()->LineTraceSingleByChannel(
		StraightMoveResult, ViewPointLocation, LineTraceEnd, ECollisionChannel::ECC_Visibility);

	// If the teleport destination did not work,
	// try to teleport to the impact point of the previous line-trace (with some possible adjustments)
	if (!bCanTeleportToLocation) {
		TeleportDestination = StraightMoveResult.ImpactPoint;
		Teleport(false);
		return;
	}

	// If the previous line trace did not hit anything
	// just teleport to the destination
	if (!StraightMoveResult.bBlockingHit) {
		Teleport();
		return;
	}

	// If the previous line trace did hit something
	// but outside the teleport range,
	// just teleport to the destination
	float DistanceToHit = FVector::Distance(GetOwner()->GetActorLocation(), StraightMoveResult.ImpactPoint);
	if (DistanceToHit >= TeleportDistance) {
		Teleport();
		return;
	}

	// Do a line-trace to check if there is ground beneath the teleport spot
	FHitResult LandBelowResult;
	FVector LandCheckEndLocation = TeleportDestination + FVector::DownVector * LandCheckDistance;
	GetWorld()->LineTraceSingleByChannel(LandBelowResult, TeleportDestination, LandCheckEndLocation, ECollisionChannel::ECC_Visibility);

	// If land was found, teleport
	if (LandBelowResult.bBlockingHit) {
		Teleport();
		return;
	}
	// If all else fails, teleport the player next to the solid object ahead
	TeleportDestination = StraightMoveResult.ImpactPoint;
	Teleport(false);
}

void UCustomCharacterMovementComponent::StartRewind()
{
	// Do not rewind if already rewinding or if there is no rewind data
	if (bWantsToRewind
		|| RewindTargetLocations.IsEmpty()) {
		return;
	}

	// Pause capturing data for rewinding
	GetOwner()->GetWorldTimerManager().PauseTimer(RewindCaptureTimer);

	// Set the flag for wanting to rewind to true
	bWantsToRewind = true;
}

bool UCustomCharacterMovementComponent::CanWallJump() const
{
	// Reset wall jump direction
	WallJumpDirection = FVector::ZeroVector;

	// If the character is not falling, wall jumping is not possible
	if (!IsFalling()) {
		return false;
	}

	// Do a linetrace to find a blocking static mesh component in front of the character
	// The linetrace length is defined by the maximum allowed angle from the wall's normal 
	FHitResult OutHit;
	FVector Start = LastUpdateLocation; 
	FVector End =
		Start 
		+ UpdatedComponent->GetForwardVector() 
		* CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius()/FMath::Sin(FMath::DegreesToRadians(90 - MaxWallJumpAngle));
	const FName ProfileName = "BlockAll";

	// Ignore the owning actor during linetrace
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	GetWorld()->LineTraceSingleByProfile(OutHit, Start, End, ProfileName, Params);

	if (OutHit.bBlockingHit 
		&& OutHit.GetComponent()->IsA(UStaticMeshComponent::StaticClass())) {
		// If a static mesh component was hit, check if the player is close enough to the wall
		// if not, return false
		End = Start - OutHit.Normal * CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius() * WallJumpCapsuleRadiusScale;
		GetWorld()->LineTraceSingleByProfile(OutHit, Start, End, ProfileName, Params);
		if (!OutHit.bBlockingHit) {
			return false;
		}

		// Set the mutable wall jump normal by mirroring the character's forward vector
		// by the wall's normal vector
		WallJumpDirection = UpdatedComponent->GetForwardVector().MirrorByVector(OutHit.Normal);

		// If we do not want the wall jump angle to be a multiple of something,
		// just return true here
		if (WallJumpAngleFactor == 0) {
			return true;
		}

		// Otherwise, first find the angle between the wall jump direction and the wall's normal
		float Angle = FMath::RadiansToDegrees(FMath::Acos(WallJumpDirection.CosineAngle2D(OutHit.Normal)));

		// Sign of the angle via cross product
		FVector CrossProduct = FVector::CrossProduct(WallJumpDirection, OutHit.Normal);
		if (CrossProduct.Z > 0) {
			Angle *= -1;
		}

		// Make the angle a multiple of the wanted figure
		// and set the jump direction accordingly
		Angle = FMath::RoundToInt(Angle / WallJumpAngleFactor) * WallJumpAngleFactor;
		WallJumpDirection = OutHit.Normal.RotateAngleAxis(Angle, FVector::ZAxisVector);
		return true;
	}

	// Else, return false
	return false;
}

bool UCustomCharacterMovementComponent::DoJump(bool bReplayingMoves)
{
	if (Super::DoJump(bReplayingMoves)) {
		
		// If a wall jump direction is set, wall jump
		if (WallJumpDirection != FVector::ZeroVector) {

			// Ignore the Z direction so as to not add further vertical velocity
			WallJumpDirection.Z = 0;

			// Set the wall jump speed according to current speed on XY-axis
			// or at the minimum defined speed (whichever is higher)
			float WallJumpSpeed = Velocity.Size2D() > MinWallJumpSpeed ? Velocity.Size2D() : MinWallJumpSpeed;

			// Zero velocity on XY-axis before wall jump
			Velocity = FVector(0, 0, Velocity.Z);

			// Launch the character away from the wall at a set speed
			Velocity += WallJumpDirection * WallJumpSpeed;

			// Set the owner's rotation according to wall jump direction 
			GetOwner()->SetActorRotation(WallJumpDirection.Rotation());
		}
		return true;
	}
	return false;
}

void UCustomCharacterMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();

	// Set a timer to capture data for rewinding
	bool InbLoop = true;
	float InRate = (1.f / RewindCapturesPerSecond);
	int8 InFirstDelay = -1;
	GetOwner()->GetWorldTimerManager().SetTimer(RewindCaptureTimer, this, &UCustomCharacterMovementComponent::CaptureRewindData, InRate, InbLoop, InFirstDelay);
}

void UCustomCharacterMovementComponent::CaptureRewindData()
{
	// If the character has not moved,
	// do not capture current location
	if (!RewindTargetLocations.IsEmpty()
		&& RewindTargetLocations.Last() == UpdatedComponent->GetComponentLocation()) {
		return;
	}
	// If there is already rewind data for the whole length of the rewind,
	// remove the first index
	if (RewindTargetLocations.Num() >= (RewindCaptureLengthSeconds * RewindCapturesPerSecond)) {
		RewindTargetLocations.RemoveAt(0);
	}

	// Add the actor's current data to the rewind data array
	RewindTargetLocations.Add(UpdatedComponent->GetComponentLocation());
}

void UCustomCharacterMovementComponent::SetTargetRewindLocation()
{
	// Get the last index of the rewind target locations
	int16 LastRewindIndex = RewindTargetLocations.Num() - 1;

	// If the current rewind index is greater or equal to the last rewind data index
	// finish rewinding properly
	if (RewindProgress >= LastRewindIndex
		|| RewindFullPerformLengthSeconds == 0)
	{
		// Set the last rewind data (if not reconciliating)
		if (!RewindTargetLocations.IsEmpty()) {
			RewindTargetLocation = RewindTargetLocations[0];
		}

		// Reset rewind progress to zero and stop rewinding
		RewindProgress = 0;
		bWantsToRewind = false;

		// Empty the rewind data array so that the same rewind cannot be performed again
		RewindTargetLocations.Empty();

		// Resume capturing new rewind data
		GetOwner()->GetWorldTimerManager().UnPauseTimer(RewindCaptureTimer);

		return;
	}

	// Get the indexes the elements of which will be interpolated between
	int16 LerpIndexA = FMath::CeilToInt(LastRewindIndex - RewindProgress);

	// Get the interpolation alpha value (decimals of Progress)
	float Alpha = RewindProgress - FMath::Floor(RewindProgress);

	// Set the rewind target location by interpolating between the elements in the set indexes
	RewindTargetLocation = FMath::Lerp(RewindTargetLocations[LerpIndexA], RewindTargetLocations[LerpIndexA - 1], Alpha);

	// Increment RewindProgress so that a full rewind is performed in the desired time
	RewindProgress += GetWorld()->GetDeltaSeconds() * (RewindCapturesPerSecond * RewindCaptureLengthSeconds) / RewindFullPerformLengthSeconds;
}

void UCustomCharacterMovementComponent::Server_SendTeleportDestination_Implementation(FVector SentTeleportDestination)
{
	// Set the teleport destination on the server
	TeleportDestination = SentTeleportDestination;
}

void UCustomCharacterMovementComponent::Server_SendRewindData_Implementation(FVector SentRewindData)
{
	// Set the rewind target destination on server
	RewindTargetLocation = SentRewindData;
}

void UCustomCharacterMovementComponent::Teleport(bool bUsedLineTrace)
{
	// If the teleport destination has not been checked, 
	// check (and possibly adjust) it now
	if (!bUsedLineTrace) {
		bool bFoundTeleportSpot = GetWorld()->FindTeleportSpot(GetOwner(), TeleportDestination, GetOwner()->GetActorRotation());

		// If no valid teleport spot was found,
		// do not teleport
		if (!bFoundTeleportSpot) {
			return;
		}
	}

	// Only send the destination to the server if this is the client
	if (PawnOwner->GetLocalRole() == ENetRole::ROLE_AutonomousProxy) {
		Server_SendTeleportDestination(TeleportDestination);
	}

	// Set the bool to teleport
	bWantsToTeleport = true;
}