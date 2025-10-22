#pragma once

#include "TextColorPair.generated.h"


USTRUCT(BlueprintType)
struct FTextColorPair
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSlateColor Color;
};