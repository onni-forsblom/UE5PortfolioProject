// Copyright (C) 2024 Onni Forsblom

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_Report.generated.h"

/**
 * 
 */
UCLASS()
class UE5PORTFOLIOPROJECT_API UBTTask_Report : public UBTTask_BlackboardBase
{
	GENERATED_UCLASS_BODY()
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:

	UPROPERTY(Category = "Blackboard", EditAnywhere)
	TEnumAsByte<enum EReportingStatus> ReportingStatus;
};
