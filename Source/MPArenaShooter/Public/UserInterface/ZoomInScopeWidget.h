// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ZoomInScopeWidget.generated.h"


class UImage;


UCLASS()
class MPARENASHOOTER_API UZoomInScopeWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	void InitializeScope(bool bZoomIn, float AimValue);

	UFUNCTION(BlueprintCallable)
	void ToggleZoom(bool bEnable, float ZoomSpeed);


protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RoundTimer")
	USoundBase *ZoomInSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RoundTimer")
	USoundBase *ZoomOutSound;


	UPROPERTY(BlueprintReadOnly, Transient, Category = "Animations", meta = (BindWidgetAnim))
	UWidgetAnimation *Anim_ZoomIn;

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	UImage *ScopeCenter_IMG;

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	UImage *ScopeOuter_IMG;


private:

	FWidgetAnimationDynamicEvent OnZoomOutCompleted;

	bool bZoomingOut = false;


	UFUNCTION()
	void HideScope();
};