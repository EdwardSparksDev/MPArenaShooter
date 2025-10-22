// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/CrosshairTextures.h"
#include "CombatWidget.generated.h"

class UCrosshairWidget;
class UHealthBarWidget;
class URoundScoreWidget;
class UWeaponInfoWidget;
class UTextBlock;
class UCombatComponent;
class UHealthComponent;
class AWeaponBase;
enum class EAimState : uint8;


UCLASS()
class MPARENASHOOTER_API UCombatWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	void SetCrosshairVisibility(ESlateVisibility NewVisibility);

	void SetHealthBarVisibility(ESlateVisibility NewVisibility);

	void SetPlayerScoreVisibility(ESlateVisibility NewVisibility);

	void SetWeaponInfoVisibility(ESlateVisibility NewVisibility);

	void SetRoundTimerVisibility(ESlateVisibility NewVisibility);


protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RoundTimer", meta = (ClampMin = "-1"))
	int32 CriticalTimeThreshold = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RoundTimer")
	FSlateColor CriticalTimeColor = FSlateColor(FLinearColor::Red);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RoundTimer")
	USoundBase *CriticalTimeSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Match")
	FSlateColor LocalPlayerColor = FSlateColor(FLinearColor::Yellow);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Match")
	FSlateColor EnemyPlayerColor = FSlateColor(FLinearColor::Red);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RoundTimer")
	USoundBase *FirstBloodSound;


	UPROPERTY(BlueprintReadOnly, Transient, Category = "Animations", meta = (BindWidgetAnim))
	UWidgetAnimation *Anim_RoundTimer;

	UPROPERTY(BlueprintReadOnly, Transient, Category = "Animations", meta = (BindWidgetAnim))
	UWidgetAnimation *Anim_FirstBlood;

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	UCrosshairWidget *CrosshairWidget;

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	UHealthBarWidget *HealthBarWidget;

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	URoundScoreWidget *RoundScoreWidget;

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	UWeaponInfoWidget *WeaponInfoWidget;

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	UTextBlock *RoundTimer_TXT;

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	UTextBlock *FirstBloodPlayerName_TXT;


	FVector2D SpreadRange;
	FVector2D AimSpreadRange;

	bool bHideReticleOnZoom = false;


	virtual void NativeConstruct() override;


private:

	AWeaponBase *LastEquippedWeapon;


	void BindToGameState();

	void BindToPlayerController();

	UFUNCTION()
	void OnControllerPossessedPawnChanged(APawn *OldPawn, APawn *NewPawn);

	void TogglePawnComponentsBindings(APawn *Pawn, bool bEnable);

	void LoadInitialValues(UCombatComponent *CombatComp, UHealthComponent *HealthComp);

	UFUNCTION()
	void OnAimValueChanged(float AimValue);

	UFUNCTION()
	void OnAimStateChanged(EAimState AimState);

	UFUNCTION()
	void OnEquippedWeaponChanged(UCombatComponent *CombatComp);

	UFUNCTION()
	void OnCrosshairSpreadChanged(float Value);

	UFUNCTION()
	void OnCrosshairColorChanged(FLinearColor Color);

	UFUNCTION()
	void OnHealthChanged(float Health, float MaxHealth);

	UFUNCTION()
	void OnMagazineAmmoChanged(int32 MagazineAmmo);

	UFUNCTION()
	void OnReserveAmmoChanged(int32 ReserveAmmo);

	UFUNCTION()
	void OnRoundTimerUpdated(int32 Time);

	UFUNCTION()
	void OnFirstBlood(APlayerStateCore *PlayerStateCore);
};