// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ScoreboardWidget.generated.h"

class APlayerState;
class UTextBlock;
class UScrollBox;
class UScoreboardEntryWidget;


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnScoreboardTickSignature);


UCLASS()
class MPARENASHOOTER_API UScoreboardWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Match")
	void ScrollScoreboardWithoutFocus(float ScrollAmount);


protected:

	UPROPERTY(EditDefaultsOnly, Category = "Scoreboard")
	TSubclassOf<UScoreboardEntryWidget> ScoreboardEntryWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scoreboard", meta = (ClampMin = "0.1"))
	float ScoreboardPingUpdateFrequency = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scoreboard")
	FMargin ScoreboardEntryPadding = FMargin(0.f, 0.f, 0.f, 8.f);


	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	UTextBlock *PlayersCounter_TXT;

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	UScrollBox *PlayersList_SCR;


	UPROPERTY(BlueprintReadOnly, Category = "Scoreboard")
	FOnScoreboardTickSignature OnScoreboardTick;

	TArray<UScoreboardEntryWidget *> DisplayedPlayerEntries;


	virtual void NativeConstruct() override;


	UFUNCTION(BlueprintCallable, Category = "Scoreboard")
	void OnPlayerStateAdded(APlayerState *PlayerState);

	UFUNCTION(BlueprintCallable, Category = "Scoreboard")
	void OnPlayerStateRemoved(APlayerState *PlayerState);

	UFUNCTION(BlueprintCallable, Category = "Scoreboard")
	void UpdatePlayersCounter();


private:

	FTimerHandle TimerHandle_ScoreboardTick;


	void BindToGameState();

	UFUNCTION()
	void OnScoreboardVisibilityChanged(ESlateVisibility NewVisibility);

	void ScoreboardTick();

	UFUNCTION()
	void OnEntriesSortRequested();
};