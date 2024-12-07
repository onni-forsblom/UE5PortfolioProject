// Copyright (C) 2024 Onni Forsblom

#pragma once

UENUM(BlueprintType)
enum EReportingStatus : uint8 {
	None,
	Suspicion,
	Observing,
	InvestigationEnd
};