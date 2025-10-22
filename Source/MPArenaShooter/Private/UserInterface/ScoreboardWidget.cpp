// Fill out your copyright notice in the Description page of Project Settings.

#include "UserInterface/ScoreboardWidget.h"
#include "GameStates/GameStateCore.h"
#include "Components/TextBlock.h"
#include "UserInterface/ScoreboardEntryWidget.h"
#include "Components/ScrollBox.h"


void UScoreboardWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindToGameState();

	OnVisibilityChanged.AddDynamic(this, &ThisClass::OnScoreboardVisibilityChanged);
	ESlateVisibility CurrentVisibility = GetVisibility();
	OnScoreboardVisibilityChanged(CurrentVisibility);

	OnEntriesSortRequested();
}


void UScoreboardWidget::BindToGameState()
{
	UWorld *World = GetWorld();
	if (!World)
	{
		return;
	}

	AGameStateCore *GameStateCore = World->GetGameState<AGameStateCore>();
	GameStateCore->OnPlayerStateAdded.AddDynamic(this, &ThisClass::OnPlayerStateAdded);
	GameStateCore->OnPlayerStateRemoved.AddDynamic(this, &ThisClass::OnPlayerStateRemoved);

	for (APlayerState *PlayerState : GameStateCore->PlayerArray)
	{
		OnPlayerStateAdded(PlayerState);
	}

	UpdatePlayersCounter();
}


void UScoreboardWidget::OnScoreboardVisibilityChanged(ESlateVisibility NewVisibility)
{
	UWorld *World = GetWorld();
	if (!World)
	{
		return;
	}

	FTimerManager &TimerManager = World->GetTimerManager();

	if (NewVisibility == ESlateVisibility::Hidden || NewVisibility == ESlateVisibility::Collapsed)
	{
		TimerManager.ClearTimer(TimerHandle_ScoreboardTick);
		TimerHandle_ScoreboardTick.Invalidate();
	}
	else if (!TimerHandle_ScoreboardTick.IsValid())
	{
		TimerManager.SetTimer(TimerHandle_ScoreboardTick, this, &ThisClass::ScoreboardTick, ScoreboardPingUpdateFrequency, true);
		ScoreboardTick();
	}
}


void UScoreboardWidget::ScoreboardTick()
{
	OnScoreboardTick.Broadcast();
}


void UScoreboardWidget::OnPlayerStateAdded(APlayerState *PlayerState)
{
	UScoreboardEntryWidget *ScoreboardEntry = CreateWidget<UScoreboardEntryWidget>(this, ScoreboardEntryWidgetClass);
	OnScoreboardTick.AddDynamic(ScoreboardEntry, &UScoreboardEntryWidget::UpdatePing);
	ScoreboardEntry->OnSortRequested.AddDynamic(this, &ThisClass::OnEntriesSortRequested);
	ScoreboardEntry->InitializeEntry(PlayerState);

	DisplayedPlayerEntries.Add(ScoreboardEntry);

	ScoreboardEntry->SetPadding(ScoreboardEntryPadding);
	PlayersList_SCR->AddChild(ScoreboardEntry);

	UpdatePlayersCounter();
}


void UScoreboardWidget::OnPlayerStateRemoved(APlayerState *PlayerState)
{
	for (UScoreboardEntryWidget *ScoreboardEntry : DisplayedPlayerEntries)
	{
		if (PlayerState == ScoreboardEntry->GetAssociatedPlayerState())
		{
			OnScoreboardTick.RemoveDynamic(ScoreboardEntry, &UScoreboardEntryWidget::UpdatePing);
			PlayersList_SCR->RemoveChild(ScoreboardEntry);
			DisplayedPlayerEntries.Remove(ScoreboardEntry);
			break;
		}
	}

	UpdatePlayersCounter();
}


void UScoreboardWidget::UpdatePlayersCounter()
{
	UWorld *World = GetWorld();
	if (!World)
	{
		return;
	}

	AGameStateCore *GameStateCore = World->GetGameState<AGameStateCore>();
	FString CounterString = FString::Printf(TEXT("( %d )"), GameStateCore->PlayerArray.Num());
	PlayersCounter_TXT->SetText(FText::FromString(CounterString));
}


void UScoreboardWidget::OnEntriesSortRequested()
{
	DisplayedPlayerEntries.StableSort([](const UScoreboardEntryWidget &EntryA, const UScoreboardEntryWidget &EntryB)
		{
			APlayerStateCore *PlayerStateCoreA = EntryA.GetAssociatedPlayerState();
			APlayerStateCore *PlayerStateCoreB = EntryB.GetAssociatedPlayerState();

			if (PlayerStateCoreA->GetScore() != PlayerStateCoreB->GetScore())
			{
				return PlayerStateCoreA->GetScore() > PlayerStateCoreB->GetScore();
			}

			return PlayerStateCoreA->GetScorePriority() < PlayerStateCoreB->GetScorePriority();
		});

	PlayersList_SCR->ClearChildren();
	for (UScoreboardEntryWidget *PlayerEntry : DisplayedPlayerEntries)
	{
		PlayersList_SCR->AddChild(PlayerEntry);
	}
}


void UScoreboardWidget::ScrollScoreboardWithoutFocus(float ScrollAmount)
{
	float CurrentOffset = PlayersList_SCR->GetScrollOffset();
	float ScrollSensibility = PlayersList_SCR->GetWheelScrollMultiplier();

	PlayersList_SCR->SetScrollOffset(CurrentOffset - ScrollAmount * ScrollSensibility);
}