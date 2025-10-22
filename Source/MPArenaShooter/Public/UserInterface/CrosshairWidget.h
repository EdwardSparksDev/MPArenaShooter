// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/CrosshairTextures.h"
#include "CrosshairWidget.generated.h"

class UImage;
class UTexture2D;


UCLASS()
class MPARENASHOOTER_API UCrosshairWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	void ToggleReticleVisibility(ESlateVisibility NewVisibility);

	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	void ApplyTextures(const FCrosshairTextures &Textures);

	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	void UpdateSpreadRange(FVector2D SpreadRange);

	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	void UpdateSpread(float Spread, bool bForceUpdate = false);

	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	void SetCrosshairColor(FLinearColor Color);


protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	USoundBase *KillMarkerSound;


	UPROPERTY(BlueprintReadOnly, Transient, Category = "Animations", meta = (BindWidgetAnim))
	UWidgetAnimation *Anim_KillMarker;


	virtual void NativeConstruct() override;


private:

	UPROPERTY(meta = (BindWidget))
	UImage *CrosshairCenter_IMG;

	UPROPERTY(meta = (BindWidget))
	UImage *CrosshairTop_IMG;

	UPROPERTY(meta = (BindWidget))
	UImage *CrosshairBottom_IMG;

	UPROPERTY(meta = (BindWidget))
	UImage *CrosshairRight_IMG;

	UPROPERTY(meta = (BindWidget))
	UImage *CrosshairLeft_IMG;


	FCrosshairTextures CurrentTextures;
	FVector2D CurrentSpreadRange;
	float CurrentSpread = -1.f;

	bool bReticleHidden = false;


	void BindToGameState();

	void SetCrosshairComponentVisibility(UImage *Image, ESlateVisibility NewVisibility);

	void SetCrosshairComponentTexture2D(UImage *Image, UTexture2D *Texture);

	void TranslateCrosshairComponentToLocation(UImage *Image, const FVector2D &Location);

	UFUNCTION()
	void OnKillfeedUpdated(APlayerStateCore *AttackerPlayerStateCore, APlayerStateCore *EliminatedPlayerStateCore,
		AActor *DamageCauser);
};
