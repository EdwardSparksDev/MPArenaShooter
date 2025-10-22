// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "KillfeedInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UKillfeedInterface : public UInterface
{
	GENERATED_BODY()
};


class MPARENASHOOTER_API IKillfeedInterface
{
	GENERATED_BODY()

public:

	virtual UTexture2D *GetKillfeedIcon() const = 0;
};
