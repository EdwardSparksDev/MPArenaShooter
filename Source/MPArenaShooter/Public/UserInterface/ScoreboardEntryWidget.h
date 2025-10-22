// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerStates/PlayerStateCore.h"
#include "ScoreboardEntryWidget.generated.h"

class UTextBlock;
class UImage;


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSortRequestedSignature);


USTRUCT(BlueprintType)
struct FPingIcon
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Threshold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D *Icon = nullptr;
};


UCLASS()
class MPARENASHOOTER_API UScoreboardEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable, Category = "Scoreboard")
	FOnSortRequestedSignature OnSortRequested;


	UFUNCTION(BlueprintCallable, Category = "Scoreboard")
	void InitializeEntry(APlayerState *PlayerState);

	UFUNCTION(BlueprintCallable, Category = "Scoreboard")
	FORCEINLINE APlayerStateCore *GetAssociatedPlayerState() const { return PlayerStateCore; }

	UFUNCTION(BlueprintCallable, Category = "Scoreboard")
	void UpdatePing();


protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scoreboard")
	FSlateColor LocalPlayerHighlightColor = FSlateColor(FColor::Yellow);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scoreboard")
	TArray<FPingIcon> PingIcons;


	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	UTextBlock *PlayerName_TXT;

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	UTextBlock *Score_TXT;

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	UTextBlock *Kills_TXT;

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	UTextBlock *Deaths_TXT;

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	UImage *Ping_IMG;

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	UTextBlock *Ping_TXT;


private:

	APlayerStateCore *PlayerStateCore;

	FTimerHandle TimerHandle_Ping;


	void UpdateEntryHighlightColor();

	UFUNCTION()
	void OnPlayerNameChanged(FString PlayerName);

	UFUNCTION()
	void OnScoreChanged(float Score);

	UFUNCTION()
	void OnKillsChanged(int32 Kills);

	UFUNCTION()
	void OnDeathsChanged(int32 Deaths);

	UTexture2D *GetPingIcon(int32 Ping);
};