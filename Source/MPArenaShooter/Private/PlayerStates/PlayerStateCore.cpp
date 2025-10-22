// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerStates/PlayerStateCore.h"
#include "Net/UnrealNetwork.h"


APlayerStateCore::APlayerStateCore()
{
	PrimaryActorTick.bCanEverTick = false;
}


void APlayerStateCore::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlayerStateCore, ScorePriority);
	DOREPLIFETIME(APlayerStateCore, Kills);
	DOREPLIFETIME(APlayerStateCore, Deaths);
}


void APlayerStateCore::OnRep_PlayerId()
{
	Super::OnRep_PlayerId();

	int32 NewPlayerId = GetPlayerId();
	OnPlayerIdChanged.Broadcast(NewPlayerId);
}


void APlayerStateCore::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();

	FString NewPlayerName = GetPlayerName();
	OnPlayerNameChanged.Broadcast(NewPlayerName);
}


void APlayerStateCore::SetScorePriority(int32 NewScoreSortingIndex)
{
	if (HasAuthority() && NewScoreSortingIndex > ScorePriority)
	{
		ScorePriority = NewScoreSortingIndex;
	}
}


void APlayerStateCore::OnRep_ScorePriority()
{
	if (bNotifyClientScoreChanged)
	{
		float NewScore = GetScore();
		OnScoreChanged.Broadcast(NewScore);
	}

	bNotifyClientScoreChanged = !bNotifyClientScoreChanged;
}


void APlayerStateCore::AddScore(float Value)
{
	if (HasAuthority() && Value > 0)
	{
		float CurrentScore = GetScore();
		SetScore(CurrentScore + Value);

		OnScoreChanged.Broadcast(GetScore());
	}
}


void APlayerStateCore::OnRep_Score()
{
	Super::OnRep_Score();

	if (bNotifyClientScoreChanged)
	{
		float NewScore = GetScore();
		OnScoreChanged.Broadcast(NewScore);
	}

	bNotifyClientScoreChanged = !bNotifyClientScoreChanged;
}


void APlayerStateCore::AddKill()
{
	if (HasAuthority())
	{
		Kills++;

		OnKillsChanged.Broadcast(Kills);
	}
}


void APlayerStateCore::OnRep_Kills()
{
	OnKillsChanged.Broadcast(Kills);
}


void APlayerStateCore::AddDeath()
{
	if (HasAuthority())
	{
		Deaths++;

		OnDeathsChanged.Broadcast(Deaths);
	}
}


void APlayerStateCore::OnRep_Deaths()
{
	OnDeathsChanged.Broadcast(Deaths);
}