// Fill out your copyright notice in the Description page of Project Settings.

#include "UserInterface/RoundScoreWidget.h"
#include "GameStates/GameStateCore.h"
#include "PlayerStates/PlayerStateCore.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Slider.h"


void URoundScoreWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindToGameState();

	CalculateScore();
}


void URoundScoreWidget::BindToGameState()
{
	GameStateCore = GetWorld()->GetGameState<AGameStateCore>();
	GameStateCore->OnPlayerStateAdded.AddDynamic(this, &ThisClass::OnPlayerStateAdded);
	GameStateCore->OnPlayerStateRemoved.AddDynamic(this, &ThisClass::OnPlayerStateRemoved);

	for (APlayerState *PlayerState : GameStateCore->PlayerArray)
	{
		BindToPlayerStateScore(PlayerState);
	}
}


void URoundScoreWidget::BindToPlayerStateScore(APlayerState *PlayerState)
{
	APlayerStateCore *PlayerStateCore = Cast<APlayerStateCore>(PlayerState);
	if (IsValid(PlayerStateCore))
	{
		PlayerStateCore->OnScoreChanged.AddDynamic(this, &ThisClass::OnScoreChanged);
	}
}


void URoundScoreWidget::OnPlayerStateAdded(APlayerState *PlayerState)
{
	BindToPlayerStateScore(PlayerState);
}


void URoundScoreWidget::OnPlayerStateRemoved(APlayerState *PlayerState)
{
	CalculateScore();
}


void URoundScoreWidget::OnScoreChanged(float Score)
{
	CalculateScore();
}


void URoundScoreWidget::CalculateScore()
{
	if (!IsValid(LocalPlayerState))
	{
		bool bLoaded = LoadLocalPlayerState();
		if (!bLoaded)
		{
#if !UE_BUILD_SHIPPING
			if (GetWorld() && !GetWorld()->bIsTearingDown)
			{
				UE_LOG(LogTemp, Error, TEXT("[RoundScoreWidget] CalculateScore(): failed to load LocalPlayerState!"));
			}
#endif
			return;
		}
	}

	float PlayerScore = GetPlayerScore();
	float EnemyScore = GetEnemyScore();

	UpdatePlayerScore(PlayerScore);
	UpdateEnemyScore(EnemyScore);
	UpdateMatchOutcome(PlayerScore, EnemyScore);
}


bool URoundScoreWidget::LoadLocalPlayerState()
{
	UWorld *World = GetWorld();
	if (!World)
	{
		return false;
	}

	APlayerController *LocalPlayerController = GetOwningPlayer();
	if (!IsValid(LocalPlayerController))
	{
		return false;
	}

	LocalPlayerState = LocalPlayerController->GetPlayerState<APlayerState>();
	return true;
}


float URoundScoreWidget::GetPlayerScore()
{
	float PlayerScore = 0.f;
	if (IsValid(LocalPlayerState))
	{
		PlayerScore = LocalPlayerState->GetScore();
	}

	return PlayerScore;
}


float URoundScoreWidget::GetEnemyScore()
{
	TArray<APlayerState *> EnemyPlayerStates = GameStateCore->PlayerArray;
	EnemyPlayerStates.Remove(LocalPlayerState);

	float BestEnemyScore = 0.f;
	for (APlayerState *EnemyPlayerState : EnemyPlayerStates)
	{
		float EnemyScore = EnemyPlayerState->GetScore();
		if (EnemyScore > BestEnemyScore)
		{
			BestEnemyScore = EnemyScore;
		}
	}

	return BestEnemyScore;
}


void URoundScoreWidget::UpdatePlayerScore(float PlayerScore)
{
	FString PlayerScoreString = FString::Printf(TEXT("%.0f"), PlayerScore);
	PlayerScore_TXT->SetText(FText::FromString(PlayerScoreString));

	float ScoreLimit = GameStateCore->GetScoreLimit();
	float PlayerScorePercent = PlayerScore / ScoreLimit;

	PlayerScore_PGB->SetPercent(PlayerScorePercent);
	PlayerScore_SLD->SetValue(PlayerScorePercent);
}


void URoundScoreWidget::UpdateEnemyScore(float EnemyScore)
{
	FString BestEnemyScoreString = FString::Printf(TEXT("%.0f"), EnemyScore);
	EnemyScore_TXT->SetText(FText::FromString(BestEnemyScoreString));

	float ScoreLimit = GameStateCore->GetScoreLimit();
	float EnemyScorePercent = EnemyScore / ScoreLimit;

	EnemyScore_PGB->SetPercent(EnemyScorePercent);
	EnemyScore_SLD->SetValue(EnemyScorePercent);
}


void URoundScoreWidget::UpdateMatchOutcome(float PlayerScore, float EnemyScore)
{
	FString MatchOutcomeString;
	FSlateColor MathOutcomeColor;

	if (PlayerScore > EnemyScore)
	{
		MatchOutcomeString = VictoryTextStyle.Text.ToString();
		MathOutcomeColor = VictoryTextStyle.Color;
	}
	else
	{
		if (PlayerScore == EnemyScore)
		{
			MatchOutcomeString = TieTextStyle.Text.ToString();
			MathOutcomeColor = TieTextStyle.Color;
		}
		else
		{
			MatchOutcomeString = DefeatTextStyle.Text.ToString();
			MathOutcomeColor = DefeatTextStyle.Color;
		}
	}

	RoundOutcome_TXT->SetText(FText::FromString(MatchOutcomeString));
	RoundOutcome_TXT->SetColorAndOpacity(MathOutcomeColor);
}