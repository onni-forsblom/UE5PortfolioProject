// Copyright (C) 2024 Onni Forsblom


#include "PerceivableNavModVolume.h"
#include "NavModifierComponent.h"
#include "CustomAIStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "NavAreas/NavArea_Null.h"
#include "Components/BoxComponent.h"

APerceivableNavModVolume::APerceivableNavModVolume(const FObjectInitializer& ObjectInitializer)
{
	// Turn of tick for efficiency
	PrimaryActorTick.bCanEverTick = false;

	// Set default values
	AISensesToRegisterFor.Add(UAISense_Sight::StaticClass());
	ParentNavAreaClass = UNavArea_Null::StaticClass();

	// Set the shape component as root 
	// and make sure it interacts with navigation properly
	ShapeComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("ShapeComponent"));
	SetRootComponent(ShapeComponent);
	ShapeComponent->bDynamicObstacle = true;

	// Set up other components
	NavModifierComponent = CreateDefaultSubobject<UNavModifierComponent>(TEXT("NavModifierComponent"));
	AIStimuliSourceComponent = CreateDefaultSubobject<UCustomAIStimuliSourceComponent>(TEXT("AIStimuliSource"));

	// Bind event to when this actor is perceived by an AI
	AIStimuliSourceComponent->OnPerceivedUpdated.AddDynamic(this, &APerceivableNavModVolume::HandleOnPerceivedUpdated);
}


void APerceivableNavModVolume::BeginPlay()
{
	Super::BeginPlay();

	// Register this actor's stimuli source component for the desired senses
	for (auto AISense : AISensesToRegisterFor) {
		AIStimuliSourceComponent->RegisterForSense(AISense);
	}
	AIStimuliSourceComponent->RegisterWithPerceptionSystem();

	SetUpParent();
}

void APerceivableNavModVolume::HandleOnPerceivedUpdated(AActor* ActorPerceivedBy, FAIStimulus Stimulus)
{
	if (!Stimulus.WasSuccessfullySensed()
		|| (ParentActor && ParentActor->GetActorLocation() == GetActorLocation())) {
		return;
	}
	NavModifierComponent->SetAreaClass(NavAreaClassOnPerceived);
}

void APerceivableNavModVolume::HandleParentOnPerceivedUpdated(AActor* ActorPerceivedBy, FAIStimulus Stimulus)
{
	// Whenever this actor is perceived succesfully
	// while it is not already actively perceived by some AI,
	// reattach this actor to the parent and set the right nav area class
	if (AIsActivelyPerceivingParent == 0
		&& Stimulus.WasSuccessfullySensed()) {
		NavModifierComponent->SetAreaClass(ParentNavAreaClass);
		AttachToActor(ParentActor, FAttachmentTransformRules::SnapToTargetIncludingScale);
	}

	// If sensed by sight, update the number of AIs actively
	// perceiving this actor
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>()) {
		AIsActivelyPerceivingParent +=
			(Stimulus.WasSuccessfullySensed()) ?
			1 : -1;
	}
	
	// If at the end this actor is not actively perceived by any AI,
	// detach from the parent
	if (AIsActivelyPerceivingParent == 0) {
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}
}

void APerceivableNavModVolume::SetUpParent()
{
	// If there is no parent, do nothing
	ParentActor = GetParentActor();
	if (!ParentActor) {
		return;
	}

	// Detach from parent while keeping same transform
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	// Try to get AI stimuli source from parent. If they do not have one, create it
	ParentAIStimuliSourceComponent = ParentActor->GetComponentByClass<UCustomAIStimuliSourceComponent>();
	if (!ParentAIStimuliSourceComponent) {
		ParentAIStimuliSourceComponent = Cast<UCustomAIStimuliSourceComponent>(
			ParentActor->AddComponentByClass(UCustomAIStimuliSourceComponent::StaticClass(), false, FTransform(), true)
		);
	}

	// Make sure the parent can be sensed with the desired senses
	for (auto AISense : AISensesToRegisterFor) {
		ParentAIStimuliSourceComponent->RegisterForSense(AISense);
	}
	ParentAIStimuliSourceComponent->RegisterWithPerceptionSystem();

	// Bind an event to when the parent is perceived
	ParentAIStimuliSourceComponent->OnPerceivedUpdated.AddDynamic(this, &APerceivableNavModVolume::HandleParentOnPerceivedUpdated);
}
