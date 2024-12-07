// Copyright (C) 2024 Onni Forsblom


#include "AIManager.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "SuspicionLevelEnum.h"
#include "BlackboardKeyNames.h"
#include "ReportingStatusEnum.h"

void UAIManager::Tick(float DeltaTime)
{
	for (auto AI = AIs.CreateIterator(); AI; ++AI) {

		// Remove invalid AIs
		if (!AI->IsValid()) {
			// In case the AI was investigating,
			// removes the null entry from investigations data
			StopInvestigating(*AI->Get());
			AI.RemoveCurrent();
			continue;
		}

		// If an AI has something to report, act accordingly

		TObjectPtr<UBlackboardComponent> BBComp = AI->Get()->GetBlackboardComponent();
		uint8 ReportingStatus = BBComp->GetValueAsEnum(UBlackboardKeyNames::GetReportingStatusKeyName());
		
		switch (ReportingStatus) {
			case EReportingStatus::Suspicion:
				BBComp->SetValueAsEnum(UBlackboardKeyNames::GetReportingStatusKeyName(), EReportingStatus::None);
				ReportSuspicion(*AI->Get());
				break;
			case EReportingStatus::InvestigationEnd:
				BBComp->SetValueAsEnum(UBlackboardKeyNames::GetReportingStatusKeyName(), EReportingStatus::None);
				StopInvestigating(*AI->Get());
				break;
			default:
				break;
		}
	}
}

bool UAIManager::IsAllowedToTick() const
{
	return !HasAnyFlags(RF_ClassDefaultObject);
}

TStatId UAIManager::GetStatId() const
{
	return TStatId();
}

UWorld* UAIManager::GetTickableGameObjectWorld() const
{
	return GetWorld();
}

void UAIManager::ReportSuspicion(AAIController& AIController)
{
	// Get the location of the detected suspicion

	TObjectPtr<UBlackboardComponent> BBComp = AIController.GetBlackboardComponent();
	TObjectPtr<AActor> DetectedHostileActor = Cast<AActor>(BBComp->GetValueAsObject(UBlackboardKeyNames::GetDetectedHostileActorKeyName()));
	FVector SuspicionLocation = DetectedHostileActor ?
		DetectedHostileActor->GetActorLocation() :
		BBComp->GetValueAsVector(UBlackboardKeyNames::GetMoveToLocationKeyName());

	TObjectPtr<UWorld> World = GetWorld();

	// Go through all the investigations by location
	
	for (auto InvestigationIt = LocationToInvestigationDataMap.CreateIterator(); InvestigationIt; ++InvestigationIt) {

		// If there is already an investigation location close enough to the newly reported location,
		// set that as the AI's investigation location to handle
		if (FVector::Dist(InvestigationIt->Key, SuspicionLocation) <= InvestigationLocationRadius) {
			SuspicionLocation = InvestigationIt->Key;
			continue;
		}

		// Remove this AI and null AIs from the investigation of any other locations
		for (auto InvestigatorIt = InvestigationIt->Value.Investigators.CreateIterator(); InvestigatorIt; ++InvestigatorIt) {
			if (!InvestigatorIt->Get()
				|| InvestigatorIt->Get() == &AIController) {
				InvestigatorIt.RemoveCurrent();
			}
		}

		// If the previous left a location without investigators
		// remove it from the investigations map
		if (InvestigationIt->Value.Investigators.IsEmpty()) {
			World->GetTimerManager().ClearTimer(InvestigationIt->Value.DelegationTimerHandle);
			InvestigationIt.RemoveCurrent();
		}
	}
	
	// Find investigation data for the reported location or add a new investigation entry

	FInvestigationData& InvestigationData = LocationToInvestigationDataMap.FindOrAdd(SuspicionLocation);
		
	// Add the current reporting AI to the array of investigators
	InvestigationData.Investigators.Add(&AIController);

	// If there is no valid timer for counting down to delegating the investigation
	// set one up
	if (!InvestigationData.DelegationTimerHandle.IsValid()) {
		bool InbLoop = false;
		float InFirstDelay = -1;
		World->GetTimerManager().SetTimer(
			InvestigationData.DelegationTimerHandle,
			FTimerDelegate::CreateUObject(this, &UAIManager::DelegateInvestigation, SuspicionLocation),
			InvestigationDelegationRate,
			InbLoop,
			InFirstDelay);
	}
}

void UAIManager::StopInvestigating(AAIController& Investigator)
{
	for (auto LocationToInvestigationDataIt = LocationToInvestigationDataMap.CreateIterator(); LocationToInvestigationDataIt; ++LocationToInvestigationDataIt) {
		TSet<TObjectPtr<AAIController>>& Investigators = LocationToInvestigationDataIt->Value.Investigators;
		if (Investigators.Contains(&Investigator)) {
			if (Investigators.Num() > 1) {
				Investigators.Remove(&Investigator);
			}
			else {
				LocationToInvestigationDataIt.RemoveCurrent();
			}
			return;
		}
	}
}

void UAIManager::DelegateInvestigation(FVector InvestigationLocation)
{
	FInvestigationData& InvestigationData = *LocationToInvestigationDataMap.Find(InvestigationLocation);

	// Make sure the timer that called this function is invalidated
	GetWorld()->GetTimerManager().ClearTimer(InvestigationData.DelegationTimerHandle);

	// Get the nav system for calculating paths. If it is invalid, do nothing

	TObjectPtr<UNavigationSystemV1> NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	TObjectPtr<ANavigationData> MainNavData = NavSystem->GetDefaultNavDataInstance();
	if (!MainNavData) {
		return;
	}

	TArray<TPair<AAIController&, FNavPathSharedPtr>> InvestigatorPathPairs;

	// Iterate through all the investigators for the location
	for (auto It = InvestigationData.Investigators.CreateIterator(); It; ++It) {

		// If at this point there are invalid AI controllers or pawns,
		// remove them
		TObjectPtr<AAIController> Investigator = It->Get();
		if (!Investigator
			|| !Investigator->GetPawn()) {
			// If there are no valid investigators, remove the investigation entry
			// and just return
			if (InvestigationData.Investigators.Num() == 1) {
				LocationToInvestigationDataMap.Remove(InvestigationLocation);
				return;
			}
			It.RemoveCurrent();
			continue;
		}

		// Calculate the investigators path to the target location
		FPathFindingQuery Query(Investigator->GetPawn(),
			*NavSystem->GetDefaultNavDataInstance(),
			Investigator->GetPawn()->GetNavAgentLocation(),
			InvestigationLocation,
			UNavigationQueryFilter::GetQueryFilter<UNavigationQueryFilter>(*MainNavData, Investigator->GetDefaultNavigationFilterClass()));
		Query.bAllowPartialPaths = true;
		Query.bRequireNavigableEndLocation = false;
		FPathFindingResult PathFindingResult = NavSystem->FindPathSync(Investigator->GetNavAgentPropertiesRef(), Query);
		InvestigatorPathPairs.Add(TPair<AAIController&, FNavPathSharedPtr>(*Investigator, PathFindingResult.Path));
	}

	// Sort the investigators
	InvestigatorPathPairs.Sort([InvestigationLocation](const TPair<AAIController&, FNavPathSharedPtr>& A, const TPair<AAIController&, FNavPathSharedPtr>& B)
	{
		FNavPathSharedPtr APath = A.Value;
		FNavPathSharedPtr BPath = B.Value;

		// If two agents reach the target, give priority to the one with the least costly path
		if (!APath->IsPartial()
			&& !BPath->IsPartial()) {
			return APath->GetCost() < BPath->GetCost();
		}
		// Else, if two agents cannot reach the target, give priority to the one that can get closer
		else if (APath->IsPartial()
			&& BPath->IsPartial()) {
			float AEndDistanceToTarget = FVector::Dist(APath->GetEndLocation(), InvestigationLocation);
			float BEndDistanceToTarget = FVector::Dist(BPath->GetEndLocation(), InvestigationLocation);
			// If both can get to an equal distance, give priority to the least costly path
			if (AEndDistanceToTarget == BEndDistanceToTarget) {
				return APath->GetCost() < BPath->GetCost();
			}
			return AEndDistanceToTarget < BEndDistanceToTarget;
		}
		// Else, give priority to the agent that can reach the target
		return APath->IsPartial() < BPath->IsPartial();
	});

	// TODO? Cache the calculated path and use it for navigation?

	// Relieve investigators from investigating expect for the closest ones we want

	for (uint8 i = InvestigatorsPerLocation; i < InvestigatorPathPairs.Num(); i++) {
		AAIController& Investigator = InvestigatorPathPairs[i].Key;
		InvestigationData.Investigators.Remove(&Investigator);
		TObjectPtr<UBlackboardComponent> BBComp = Investigator.GetBlackboardComponent();

		// If the investigator has eyes on an enemy, set them to just observe them
		if (BBComp->GetValueAsObject(UBlackboardKeyNames::GetDetectedHostileActorKeyName())) {
			BBComp->SetValueAsEnum(UBlackboardKeyNames::GetReportingStatusKeyName(), EReportingStatus::Observing);
			return;
		}

		// Otherwise, just reset the investigator's suspicion level
		BBComp->SetValueAsEnum(UBlackboardKeyNames::GetSuspicionLevelKeyName(), ESuspicionLevel::Normal);
		
	}
}