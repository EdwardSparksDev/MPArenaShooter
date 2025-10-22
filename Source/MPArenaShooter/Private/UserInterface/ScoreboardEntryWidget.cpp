// Fill out your copyright notice in the Description page of Project Settings.

#include "UserInterface/ScoreboardEntryWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"
#include "Components/Image.h"


void UScoreboardEntryWidget::InitializeEntry(APlayerState *PlayerState)
{
	if (!IsValid(PlayerState))
		return;
	PlayerStateCore = Cast<APlayerStateCore>(PlayerState);
	if (!IsValid(PlayerStateCore))
	{
#if !UE_BUILD_SHIPPING
		UE_LOG(LogTemp, Error, TEXT("[ScoreboardEntryWidget] InitializeEntry(): failed to convert PlayerState to PlayerStateCore!"));
#endif
		return;
	}

	// PlayerName.
	PlayerStateCore->OnPlayerNameChanged.AddDynamic(this, &ThisClass::OnPlayerNameChanged);
	FString PlayerName = PlayerStateCore->GetPlayerName();
	PlayerName_TXT->SetText(FText::FromString(PlayerName));

	// Score.
	PlayerStateCore->OnScoreChanged.AddDynamic(this, &ThisClass::OnScoreChanged);
	float Score = PlayerStateCore->GetScore();
	Score_TXT->SetText(FText::AsNumber(Score));

	// Kills.
	PlayerStateCore->OnKillsChanged.AddDynamic(this, &ThisClass::OnKillsChanged);
	int32 Kills = PlayerStateCore->GetKills();
	Kills_TXT->SetText(FText::AsNumber(Kills));

	//Deaths.
	PlayerStateCore->OnDeathsChanged.AddDynamic(this, &ThisClass::OnDeathsChanged);
	int32 Deaths = PlayerStateCore->GetDeaths();
	Deaths_TXT->SetText(FText::AsNumber(Deaths));

	UpdateEntryHighlightColor();
	UpdatePing();
}


void UScoreboardEntryWidget::OnPlayerNameChanged(FString PlayerName)
{
	PlayerName_TXT->SetText(FText::FromString(PlayerName));
}


void UScoreboardEntryWidget::OnScoreChanged(float Score)
{
	Score_TXT->SetText(FText::AsNumber(Score));

	OnSortRequested.Broadcast();
}


void UScoreboardEntryWidget::OnKillsChanged(int32 Kills)
{
	Kills_TXT->SetText(FText::AsNumber(Kills));
}


void UScoreboardEntryWidget::OnDeathsChanged(int32 Deaths)
{
	Deaths_TXT->SetText(FText::AsNumber(Deaths));
}


void UScoreboardEntryWidget::UpdateEntryHighlightColor()
{
	APlayerController *LocalPlayerController = GetOwningPlayer();
	APlayerStateCore *LocalPlayerState = LocalPlayerController->GetPlayerState<APlayerStateCore>();

	if (IsValid(LocalPlayerState) && LocalPlayerState == PlayerStateCore)
	{
		PlayerName_TXT->SetColorAndOpacity(LocalPlayerHighlightColor);
		Score_TXT->SetColorAndOpacity(LocalPlayerHighlightColor);
		Kills_TXT->SetColorAndOpacity(LocalPlayerHighlightColor);
		Deaths_TXT->SetColorAndOpacity(LocalPlayerHighlightColor);
	}
}


void UScoreboardEntryWidget::UpdatePing()
{
	int32 Ping = FMath::RoundToInt32(PlayerStateCore->GetPingInMilliseconds());
	Ping_TXT->SetText(FText::AsNumber(Ping));

	UTexture2D *PingIcon = GetPingIcon(Ping);
	Ping_IMG->SetBrushFromTexture(PingIcon);
}


UTexture2D *UScoreboardEntryWidget::GetPingIcon(int32 Ping)
{
	for (const FPingIcon &PingIcon : PingIcons)
	{
		if (Ping <= PingIcon.Threshold)
		{
			return PingIcon.Icon;
		}
	}

	if (PingIcons.Num() > 0)
	{
		return PingIcons.Last().Icon;
	}

	return nullptr;
}