#pragma once

#include "Rarity.generated.h"


UENUM(BlueprintType)
enum class ERarity : uint8
{
	ER_None			UMETA(DisplayName = "None"),
	ER_Common		UMETA(DisplayName = "Common"),
	ER_Uncommon		UMETA(DisplayName = "Uncommon"),
	ER_Rare			UMETA(DisplayName = "Rare"),
	ER_Epic			UMETA(DisplayName = "Epic"),
	ER_Legendary	UMETA(DisplayName = "Legendary"),
	ER_Mythic		UMETA(DisplayName = "Mythic"),

	ER_MAX			UMETA(Hidden)
};