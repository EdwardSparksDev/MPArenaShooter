// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/TextColorPair.h"
#include "RoundScoreWidget.generated.h"

class AGameStateCore;
class APlayerState;
class UTextBlock;
class UProgressBar;
class USlider;


UCLASS()
class MPARENASHOOTER_API URoundScoreWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scoreboard")
	FTextColorPair VictoryTextStyle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scoreboard")
	FTextColorPair TieTextStyle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scoreboard")
	FTextColorPair DefeatTextStyle;


	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	UTextBlock *RoundOutcome_TXT;

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	UTextBlock *PlayerScore_TXT;

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	UTextBlock *EnemyScore_TXT;

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	UProgressBar *PlayerScore_PGB;

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	USlider *PlayerScore_SLD;

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	UProgressBar *EnemyScore_PGB;

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	USlider *EnemyScore_SLD;


	virtual void NativeConstruct() override;


	void CalculateScore();

	bool LoadLocalPlayerState();

	float GetPlayerScore();

	float GetEnemyScore();

	void UpdatePlayerScore(float PlayerScore);

	void UpdateEnemyScore(float EnemyScore);

	void UpdateMatchOutcome(float PlayerScore, float EnemyScore);


private:

	AGameStateCore *GameStateCore;

	APlayerState *LocalPlayerState;


	void BindToGameState();

	void BindToPlayerStateScore(APlayerState *PlayerState);

	UFUNCTION()
	void OnPlayerStateAdded(APlayerState *PlayerState);

	UFUNCTION()
	void OnPlayerStateRemoved(APlayerState *PlayerState);

	UFUNCTION()
	void OnScoreChanged(float Score);
};