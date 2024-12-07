// Copyright (C) 2024 Onni Forsblom

#include "BTTask_Report.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "ReportingStatusEnum.h"

UBTTask_Report::UBTTask_Report(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NodeName = "Report";
	BlackboardKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_Report, BlackboardKey), StaticEnum<EReportingStatus>());
}

EBTNodeResult::Type UBTTask_Report::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (!BlackboardKey.IsSet()) {
		return EBTNodeResult::Failed;
	}
	OwnerComp.GetBlackboardComponent()->SetValueAsEnum(BlackboardKey.SelectedKeyName, ReportingStatus.GetIntValue());

	// ...

	return EBTNodeResult::Succeeded;
}
