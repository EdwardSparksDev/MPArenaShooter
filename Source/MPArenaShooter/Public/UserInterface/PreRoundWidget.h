// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PreRoundWidget.generated.h"

class UTextBlock;


UCLASS()
class MPARENASHOOTER_API UPreRoundWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RoundTimer", meta = (ClampMin = "-1"))
	int32 CriticalTimeThreshold = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RoundTimer", meta = (ClampMin = "-1"))
	USoundBase *CriticalTimeSound;


	virtual void NativeConstruct() override;


	UFUNCTION()
	void OnPreRoundTimerUpdated(int32 Time);


private:

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation *Anim_PreRoundTimer;

	UPROPERTY(meta = (BindWidget))
	UTextBlock *PreRoundTimer_TXT;


	void BindToGameState();
};