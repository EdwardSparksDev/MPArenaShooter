// Fill out your copyright notice in the Description page of Project Settings.

#include "UserInterface/PostRoundWidget.h"
#include "GameStates/GameStateCore.h"
#include "Types/RoundEndReason.h"
#include "Components/TextBlock.h"
#include "PlayerStates/PlayerStateCore.h"
#include "Kismet/GameplayStatics.h"


void UPostRoundWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UWorld *World = GetWorld();
	if (!World)
	{
		return;
	}

	// Store text blocks in an array for easier access.
	DisplayedWinnersSlots = { FirstPlace_TXT, SecondPlace_TXT, ThirdPlace_TXT };

	AGameStateCore *GameStateCore = World->GetGameState<AGameStateCore>();
	ERoundEndReason EndReason = GameStateCore->GetRoundEndReason();
	OnRoundEndReasonDefined(EndReason);

	TArray<APlayerState *> Winners = GameStateCore->GetMatchWinners();
	OnMatchWinnersSelected(Winners);
}


void UPostRoundWidget::OnRoundEndReasonDefined(ERoundEndReason Reason)
{
	if (Reason == ERoundEndReason::ERER_Undefined)
	{
		RoundEndReason_TXT->SetVisibility(ESlateVisibility::Hidden);

		UWorld *World = GetWorld();
		if (!World)
		{
			return;
		}

		AGameStateCore *GameStateCore = World->GetGameState<AGameStateCore>();
		GameStateCore->OnRoundEndReasonDefined.AddUniqueDynamic(this, &ThisClass::OnRoundEndReasonDefined);
	}
	else
	{
		DisplayRoundEndReason(Reason);

		RoundEndReason_TXT->SetVisibility(ESlateVisibility::Visible);
	}
}


void UPostRoundWidget::DisplayRoundEndReason(ERoundEndReason Reason)
{
	if (Reason == ERoundEndReason::ERER_TimeLimitReached)
	{
		RoundEndReason_TXT->SetText(TimeLimitReachedText);
	}
	else if (Reason == ERoundEndReason::ERER_ScoreLimitReached)
	{
		RoundEndReason_TXT->SetText(ScoreLimitReachedText);
	}
}


void UPostRoundWidget::OnMatchWinnersSelected(TArray<APlayerState *> Winners)
{
	if (Winners.IsEmpty())
	{
		SetVisibility(ESlateVisibility::Hidden);

		UWorld *World = GetWorld();
		if (!World)
		{
			return;
		}

		AGameStateCore *GameStateCore = World->GetGameState<AGameStateCore>();
		GameStateCore->OnMatchWinnersSelected.AddUniqueDynamic(this, &ThisClass::OnMatchWinnersSelected);
	}
	else
	{
		DisplayScoreboard();
		DisplayMatchOutcome(Winners);

		SetVisibility(ESlateVisibility::Visible);
	}
}


void UPostRoundWidget::DisplayScoreboard()
{
	UWorld *World = GetWorld();
	if (!World)
	{
		return;
	}

	// Listen for any player's score that may still be replicating.
	AGameStateCore *GameStateCore = World->GetGameState<AGameStateCore>();
	CachedPlayers.Empty();

	for (APlayerState *PlayerState : GameStateCore->PlayerArray)
	{
		if (IsValid(PlayerState))
		{
			APlayerStateCore *PlayerStateCore = Cast<APlayerStateCore>(PlayerState);
			PlayerStateCore->OnScoreChanged.AddUniqueDynamic(this, &ThisClass::OnPlayerScoreChanged);
			CachedPlayers.Add(PlayerStateCore);
		}
	}

	float StartTime = 0.f;
	int32 NumLoopsToPlay = 1;
	float TimeDilation = UGameplayStatics::GetGlobalTimeDilation(this);
	PlayAnimation(Anim_ShowWinners, StartTime, NumLoopsToPlay, EUMGSequencePlayMode::Forward, TimeDilation);

	RefreshScoreboard();
}


void UPostRoundWidget::OnPlayerScoreChanged(float NewScore)
{
	RefreshScoreboard();
}


void UPostRoundWidget::RefreshScoreboard()
{
	TArray<APlayerStateCore *> TopPlayers = FilterRoundTopPlayers(CachedPlayers);

	for (int Index = 0; Index < MaxDisplayedPlayers; Index++)
	{
		if (Index >= TopPlayers.Num())
		{
			DisplayedWinnersSlots[Index]->SetVisibility(ESlateVisibility::Hidden);
			continue;
		}

		float Score = TopPlayers[Index]->GetScore();

		FString PlacementString = FString::Printf(TEXT("%s %s"), *PlacementStyles[Index].Prefix.ToString(),
			*TopPlayers[Index]->GetPlayerName());

		DisplayedWinnersSlots[Index]->SetText(FText::FromString(PlacementString));
		DisplayedWinnersSlots[Index]->SetColorAndOpacity(PlacementStyles[Index].Color);
		FSlateFontInfo UpdatedPlacementFont = DisplayedWinnersSlots[Index]->GetFont();
		UpdatedPlacementFont.Size = PlacementStyles[Index].FontSize;
		DisplayedWinnersSlots[Index]->SetFont(UpdatedPlacementFont);
	}
}


TArray<APlayerStateCore *> UPostRoundWidget::FilterRoundTopPlayers(const TArray<APlayerStateCore *> &RoundPlayers)
{
	TArray<APlayerStateCore *> TopPlayers = RoundPlayers;

	TopPlayers.RemoveAll([](APlayerStateCore *PlayerStateCore)
		{
			return PlayerStateCore == nullptr;
		});

	TopPlayers.StableSort([](const APlayerStateCore &PlayerStateCoreA, const APlayerStateCore &PlayerStateCoreB)
		{
			if (PlayerStateCoreA.GetScore() != PlayerStateCoreB.GetScore())
			{
				return PlayerStateCoreA.GetScore() > PlayerStateCoreB.GetScore();
			}

			return PlayerStateCoreA.GetScorePriority() < PlayerStateCoreB.GetScorePriority();
		});

	int DisplayedPlayers = FMath::Min(MaxDisplayedPlayers, TopPlayers.Num());
	TopPlayers.SetNum(DisplayedPlayers);

	return TopPlayers;
}


void UPostRoundWidget::DisplayMatchOutcome(TArray<APlayerState *> Winners)
{
	APlayerController *LocalPlayerController = GetOwningPlayer();
	APlayerState *LocalPlayerState = LocalPlayerController->GetPlayerState<APlayerState>();
	if (Winners.Contains(LocalPlayerState))
	{
		MatchOutcome_TXT->SetText(VictoryTextStyle.Text);
		MatchOutcome_TXT->SetColorAndOpacity(VictoryTextStyle.Color);
	}
	else
	{
		MatchOutcome_TXT->SetText(DefeatTextStyle.Text);
		MatchOutcome_TXT->SetColorAndOpacity(DefeatTextStyle.Color);
	}
}