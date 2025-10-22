// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "HUDCore.generated.h"

class UPreRoundWidget;
class UScoreboardWidget;
class UCombatWidget;
class UPostRoundWidget;
class UZoomInScopeWidget;
enum class EAimState : uint8;


UCLASS()
class MPARENASHOOTER_API AHUDCore : public AHUD
{
	GENERATED_BODY()

public:

	AHUDCore();


	UFUNCTION(BlueprintCallable, Category = "Match")
	void ToggleScoreboard(bool bEnable);

	UFUNCTION(BlueprintCallable, Category = "Match")
	void ScrollScoreboard(float Amount);


protected:

	UPROPERTY(EditAnywhere, Category = "Match")
	TSubclassOf<UPreRoundWidget> PreRoundWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Match")
	TSubclassOf<UScoreboardWidget> ScoreboardWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Match", meta = (EditCondition = "ScoreboardWidgetClass != nullptr", EditConditionHides))
	int32 ScoreboardZOrder = 1;

	UPROPERTY(EditAnywhere, Category = "Match")
	TSubclassOf<UCombatWidget> CombatWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Match")
	TSubclassOf<UPostRoundWidget> PostRoundWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Match")
	bool bDisplayCrosshair = true;

	UPROPERTY(EditAnywhere, Category = "Match")
	bool bDisplayHealthBar = true;

	UPROPERTY(EditAnywhere, Category = "Match")
	bool bDisplayPlayerScore = true;

	UPROPERTY(EditAnywhere, Category = "Match")
	bool bDisplayWeaponInfo = true;

	UPROPERTY(EditAnywhere, Category = "Match")
	bool bDisplayMatchTimer = true;


	UPROPERTY()
	UPreRoundWidget *PreRoundWidget;

	UPROPERTY()
	UScoreboardWidget *ScoreboardWidget;

	UPROPERTY()
	UCombatWidget *CombatWidget;

	UPROPERTY()
	UPostRoundWidget *PostRoundWidget;

	UPROPERTY()
	UZoomInScopeWidget *ActiveZoomInScopeWidget;


	virtual void BeginPlay() override;


private:

	UPROPERTY()
	TSubclassOf<UZoomInScopeWidget> ActiveZoomInScopeWidgetClass;

	bool bCanDisplayZoomInScope = false;

	float LastZoomInInterpSpeedMultiplier = 1.f;


	void BindToGameState();

	void OnPreRoundStarted();

	UFUNCTION()
	void OnRoundStarted();

	UFUNCTION()
	void OnRoundEnded();

	void BindToPlayerController();

	UFUNCTION()
	void OnControllerPossessedPawnChanged(APawn *OldPawn, APawn *NewPawn);

	void TogglePawnComponentsBindings(APawn *Pawn, bool bEnable);

	UFUNCTION()
	void OnEquippedWeaponChanged(UCombatComponent *CombatComp);

	void SwitchZoomInScope(TSubclassOf<UZoomInScopeWidget> ZoomInScopeWidgetClass, float AimValue, EAimState AimState);

	UFUNCTION()
	void OnAimStateChanged(EAimState AimState);
};