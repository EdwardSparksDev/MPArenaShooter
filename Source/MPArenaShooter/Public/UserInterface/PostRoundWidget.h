// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/TextColorPair.h"
#include "PostRoundWidget.generated.h"

class UTextBlock;
class APlayerStateCore;


USTRUCT(BlueprintType)
struct FPlacementStyle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Prefix;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FontSize = 32.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSlateColor Color;
};


UCLASS()
class MPARENASHOOTER_API UPostRoundWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "Scoreboard", meta = (ClampMin = "1", ClampMax = "3"))
	int32 MaxDisplayedPlayers = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scoreboard")
	FTextColorPair VictoryTextStyle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scoreboard")
	FTextColorPair DefeatTextStyle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scoreboard")
	FText TimeLimitReachedText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scoreboard")
	FText ScoreLimitReachedText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scoreboard")
	TArray<FPlacementStyle> PlacementStyles;


	UPROPERTY(BlueprintReadOnly, Transient, Category = "Animations", meta = (BindWidgetAnim))
	UWidgetAnimation *Anim_ShowWinners;

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	UTextBlock *MatchOutcome_TXT;

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	UTextBlock *RoundEndReason_TXT;

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	UTextBlock *FirstPlace_TXT;

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	UTextBlock *SecondPlace_TXT;

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	UTextBlock *ThirdPlace_TXT;


	UPROPERTY(BlueprintReadOnly, Transient)
	TArray<UTextBlock *> DisplayedWinnersSlots;


	virtual void NativeConstruct() override;


	void DisplayRoundEndReason(ERoundEndReason Reason);

	void DisplayScoreboard();

	void RefreshScoreboard();

	TArray<APlayerStateCore *> FilterRoundTopPlayers(const TArray<APlayerStateCore *> &TopPlayers);


private:

	TArray<APlayerStateCore *> CachedPlayers;


	UFUNCTION()
	void OnRoundEndReasonDefined(ERoundEndReason Reason);

	UFUNCTION()
	void OnMatchWinnersSelected(TArray<APlayerState *> Winners);

	UFUNCTION()
	void OnPlayerScoreChanged(float NewScore);

	void DisplayMatchOutcome(TArray<APlayerState *> Winners);
};