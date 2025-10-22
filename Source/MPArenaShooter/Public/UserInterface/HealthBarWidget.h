// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthBarWidget.generated.h"

class UProgressBar;
class UTextBlock;


UCLASS()
class MPARENASHOOTER_API UHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "HealthBar")
	void UpdateValue(float Health, float MaxHealth);


private:

	UPROPERTY(meta = (BindWidget))
	UProgressBar *HealthBarFill_PRG;

	UPROPERTY(meta = (BindWidget))
	UTextBlock *HealthBarValue_TXT;
};
