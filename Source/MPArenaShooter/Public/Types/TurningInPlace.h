#pragma once

#include "TurningInPlace.generated.h"


UENUM(BlueprintType)
enum class ETurningInPlace : uint8
{
	ETIP_None	UMETA(DisplayName = "None"),
	ETIP_Right	UMETA(DisplayName = "Right"),
	ETIP_Left	UMETA(DisplayName = "Left"),

	ETIP_MAX	UMETA(Hidden)
};