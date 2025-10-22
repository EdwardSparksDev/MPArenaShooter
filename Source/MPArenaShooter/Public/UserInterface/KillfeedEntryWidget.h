// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KillfeedEntryWidget.generated.h"

class USizeBox;
class UTextBlock;
class UImage;
class APlayerStateCore;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKillfeedEntryDespawnedSignature, UKillfeedEntryWidget *, DespawnedEntry);


UCLASS()
class MPARENASHOOTER_API UKillfeedEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable, Category = "Killfeed")
	FOnKillfeedEntryDespawnedSignature OnKillfeedEntryDespawned;


	void InitializeEntry(APlayerStateCore *AttackerPlayerStateCore, APlayerStateCore *EliminatedPlayerStateCore, AActor *DamageCauser,
		float DisplayDuration, float AttackerSlotMaxDesiredWidth);

	void DespawnEntry();


protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Killfeed")
	FSlateColor LocalPlayerColor = FSlateColor(FColor::Yellow);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Killfeed")
	FSlateColor EnemyPlayerColor = FSlateColor(FColor::Red);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Killfeed")
	FText UnknownPlayerName;


	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation *Anim_FadeIn;

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	USizeBox *AttackerSlot_SBX;

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	UTextBlock *AttackerPlayerName_TXT;

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	UImage *KillIcon_IMG;

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	UTextBlock *EliminatedPlayerName_TXT;


	float ActiveDisplayDuration;

	bool bIsActive = false;

	bool bFadingIn = false;

	bool bFadingOut = false;


	void SetPlayerText(UTextBlock *PlayerTextBlock, APlayerStateCore *PlayerStateCore, APlayerStateCore *LocalPlayerStateCore);

	void SetKillIcon(AActor *DamageCauser);


private:

	FTimerHandle TimerHandle_Despawn;

	FWidgetAnimationDynamicEvent OnFadeInCompleted;

	FWidgetAnimationDynamicEvent OnFadeOutCompleted;


	UFUNCTION()
	void StartActiveLifetime();

	UFUNCTION()
	void OnEntryDespawned();

	void StartFadeIn();

	void StartFadeOut();
};