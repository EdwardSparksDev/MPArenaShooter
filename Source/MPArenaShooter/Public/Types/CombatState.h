#pragma once

#include "CombatState.generated.h"


UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_None		UMETA(DisplayName = "None"),
	ECS_Reloading	UMETA(DisplayName = "Reloading"),

	ECS_MAX			UMETA(Hidden)
};