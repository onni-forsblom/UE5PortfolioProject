// Copyright (C) 2024 Onni Forsblom

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AIManager.generated.h"

class AAIController;

struct FInvestigationData {
	TSet<TObjectPtr<AAIController>> Investigators;
	FTimerHandle DelegationTimerHandle;
};

/**
 * 
 */
UCLASS()
class UE5PORTFOLIOPROJECT_API UAIManager : public UObject, public FTickableGameObject
{
	GENERATED_BODY()
	
public:

	// TODO? another container for finding investigators more efficiently?

	// Map of AI investigators and delegation timers by the suspicion location
	TMap<FVector, FInvestigationData> LocationToInvestigationDataMap;

	// The AI agents to manage by this manager
	TArray<TWeakObjectPtr<AAIController>> AIs;

	void Tick(float DeltaTime) override;

	// Allow tick if this is not the class default object
	bool IsAllowedToTick() const override;

	TStatId GetStatId() const override;
	virtual UWorld* GetTickableGameObjectWorld() const override;

	// Sets up the delegation of the suspicion detected by the AI
	void ReportSuspicion(AAIController& AIController);

	// Finds the investigation the AI was assigned to and removes the AI from it.
	// Or if the AI is the last investigator, removes the investigation from the investigations map.
	void StopInvestigating(AAIController& Investigator);

private:

	uint8 InvestigatorsPerLocation = 2;

	const float InvestigationLocationRadius = 300;

	const float InvestigationDelegationRate = 1;

	void DelegateInvestigation(FVector InvestigationLocation);
};