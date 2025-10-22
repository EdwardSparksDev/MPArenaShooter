// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KillfeedWidget.generated.h"

class UOverlay;
class APlayerControllerCore;
class UKillfeedEntryWidget;


UCLASS()
class MPARENASHOOTER_API UKillfeedWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Killfeed")
	TSubclassOf<UKillfeedEntryWidget> KillfeedEntryWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Killfeed", meta = (ClampMin = "1"))
	int32 MaxDisplayedActiveEntries = 4;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Killfeed", meta = (ClampMin = "1"))
	float ActiveEntriesDisplayDuration = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Killfeed", meta = (ClampMin = "1"))
	float AttackerSlotMaxDesiredWidth = 600.f;


	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	UOverlay *KillfeedPanel_OVR;

	UPROPERTY()
	TArray<UKillfeedEntryWidget *> DisplayedActiveEntries;

	UPROPERTY()
	TArray<UKillfeedEntryWidget *> PooledEntries;


	virtual void NativeConstruct() override;


private:

	void BindToGameState();

	UFUNCTION()
	void OnKillfeedUpdated(APlayerStateCore *AttackerPlayerStateCore, APlayerStateCore *EliminatedPlayerStateCore,
		AActor *DamageCauser);

	UFUNCTION()
	void OnKillfeedEntryDespawned(UKillfeedEntryWidget *DespawnedEntry);

	void PanKillfeed();
};