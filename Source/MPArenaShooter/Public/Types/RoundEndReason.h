#pragma once

#include "RoundEndReason.generated.h"


UENUM(BlueprintType)
enum class ERoundEndReason : uint8
{
	ERER_Undefined			UMETA(DisplayName = "Undefined"),
	ERER_TimeLimitReached	UMETA(DisplayName = "Time Limit Reached"),
	ERER_ScoreLimitReached	UMETA(DisplayName = "Score Limit Reached"),

	ERER_Max				UMETA(Hidden)
};