// Fill out your copyright notice in the Description page of Project Settings.

#include "GameModes/GameModeCore.h"
#include "Components/HealthComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "PlayerStates/PlayerStateCore.h"
#include "GameStates/GameStateCore.h"
#include "PlayerControllers/PlayerControllerCore.h"
#include "GameFramework/SpectatorPawn.h" 

#include "Components/CombatComponent.h"
#include "Weapons/WeaponBase.h"


AGameModeCore::AGameModeCore()
{
	PrimaryActorTick.bCanEverTick = false;

	bDelayedStart = true;
	MinRespawnDelay = 3.f;
}


#if WITH_EDITOR
void AGameModeCore::PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(AGameModeCore, PostMatchDuration) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(AGameModeCore, PostMatchSpectateDelay))
	{
		if (PostMatchSpectateDelay > PostMatchDuration)
		{
			PostMatchSpectateDelay = PostMatchDuration;
		}
	}
}
#endif


void AGameModeCore::InitGameState()
{
	Super::InitGameState();

	AGameStateCore *GameStateCore = GetGameState<AGameStateCore>();
	GameStateCore->PreRoundDuration = PreRoundDuration;
	GameStateCore->RoundDuration = RoundDuration;
	GameStateCore->ScoreLimit = ScoreLimit;
}


void AGameModeCore::HandleMatchIsWaitingToStart()
{
	Super::HandleMatchIsWaitingToStart();

	if (bDelayedStart)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_MatchStates, this, &AGameMode::StartMatch, PreRoundDuration);
	}
}


void AGameModeCore::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	GetWorldTimerManager().SetTimer(TimerHandle_MatchStates, this, &ThisClass::TimeLimitReached, RoundDuration);
}


void AGameModeCore::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();

	if (TimerHandle_MatchStates.IsValid())
	{
		GetWorldTimerManager().ClearTimer(TimerHandle_MatchStates);
	}

	SelectMatchWinners();

	if (bPostMatchSpectate)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_PostMatchSpectate, this, &ThisClass::PostMatchSpectate, PostMatchSpectateDelay);
	}
	GetWorldTimerManager().SetTimer(TimerHandle_MatchStates, this, &AGameMode::RestartGame, PostMatchDuration);
}


void AGameModeCore::BeginPlay()
{
	// Gather the spawn points.
	UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerSpawns);
}


void AGameModeCore::FinishRestartPlayer(AController *NewPlayer, const FRotator &StartRotation)
{
	Super::FinishRestartPlayer(NewPlayer, StartRotation);

	// Bind game mode delegates to newly spawned pawn.
	APawn *NewPawn = NewPlayer->GetPawn();
	if (NewPawn)
	{
		TogglePawnComponentsBindings(NewPawn, true);
	}
}


void AGameModeCore::TogglePawnComponentsBindings(APawn *Pawn, bool bEnable)
{
	if (!IsValid(Pawn))
	{
		return;
	}

	UHealthComponent *HealthComp = Pawn->FindComponentByClass<UHealthComponent>();
	if (HealthComp)
	{
		if (bEnable)
		{
			HealthComp->OnPlayerEliminated.AddUniqueDynamic(this, &ThisClass::OnPlayerEliminated);
		}
		else
		{
			HealthComp->OnPlayerEliminated.RemoveDynamic(this, &ThisClass::OnPlayerEliminated);
		}
	}
}


void AGameModeCore::TimeLimitReached()
{
	if (!IsMatchInProgress())
	{
		return;
	}

	AGameStateCore *GameStateCore = GetGameState<AGameStateCore>();
	GameStateCore->SetRoundEndReason(ERoundEndReason::ERER_TimeLimitReached);

	EndMatch();
}


void AGameModeCore::OnPlayerEliminated(AController *AttackerController, AController *EliminatedController, AActor *DamageCauser)
{
	if (!IsMatchInProgress())
	{
		return;
	}

	if (EliminatedController != AttackerController)
	{
		RegisterPlayerKill(AttackerController);
	}

	RegisterPlayerDeath(EliminatedController);

	// Update players' killfeeds.
	APlayerStateCore *AttackerPlayerStateCore = AttackerController->GetPlayerState<APlayerStateCore>();
	APlayerStateCore *EliminatedPlayerStateCore = EliminatedController->GetPlayerState<APlayerStateCore>();
	AGameStateCore *GameStateCore = GetGameState<AGameStateCore>();
	GameStateCore->NotifyKillfeed(AttackerPlayerStateCore, EliminatedPlayerStateCore, DamageCauser);
}


void AGameModeCore::RegisterPlayerDeath(AController *EliminatedController)
{
	if (!IsValid(EliminatedController))
	{
		return;
	}

	APlayerStateCore *PlayerStateCore = EliminatedController->GetPlayerState<APlayerStateCore>();
	if (PlayerStateCore)
	{
		PlayerStateCore->AddDeath();
	}

	if (HasMatchEnded())
	{
		return;
	}

	RespawnQueue.Enqueue(EliminatedController);

	if (MinRespawnDelay <= 0.f)
	{
		RespawnPlayer();
	}
	else
	{
		FTimerHandle TimerHandle_Respawn;
		GetWorldTimerManager().SetTimer(TimerHandle_Respawn, this, &ThisClass::RespawnPlayer, MinRespawnDelay);
	}
}


void AGameModeCore::RespawnPlayer()
{
	if (HasMatchEnded())
	{
		return;
	}

	AController *RespawnController;
	RespawnQueue.Dequeue(RespawnController);

	APawn *EliminatedPawn = RespawnController->GetPawn();
	if (EliminatedPawn)
	{
		EliminatedPawn->Destroy();
	}

	if (IsValid(RespawnController) && !PlayerSpawns.IsEmpty())
	{
		int SelectedSpawn = FMath::RandRange(0, PlayerSpawns.Num() - 1);
		RestartPlayerAtPlayerStart(RespawnController, PlayerSpawns[SelectedSpawn]);
	}
}


void AGameModeCore::RegisterPlayerKill(AController *AttackerController)
{
	if (!IsValid(AttackerController))
	{
		return;
	}

	APlayerStateCore *PlayerStateCore = AttackerController->GetPlayerState<APlayerStateCore>();
	if (!PlayerStateCore)
	{
		return;
	}

	PlayerStateCore->AddKill();
	if (!bFirstBlood)
	{
		bFirstBlood = true;
		AGameStateCore *GameStateCore = GetGameState<AGameStateCore>();
		GameStateCore->NotifyFirstBlood(PlayerStateCore);
	}

	UpdatePlayerScore(PlayerStateCore);
}


void AGameModeCore::UpdatePlayerScore(APlayerStateCore *PlayerStateCore)
{
	PlayerStateCore->SetScorePriority(ScorePriorityIndex);
	if (ScorePriorityIndex < TNumericLimits<int32>::Max())
	{
		ScorePriorityIndex++;
	}
	else
	{
		ScorePriorityIndex = 0;
		UE_LOG(LogTemp, Error, TEXT("[GameModeCore] RegisterPlayerKill(): had to wrap the ScoreSortIndex, this probably shouldn't have happened."));
	}

	float AttackerScore = PlayerStateCore->GetScore();
	float NewAttackerScore = FMath::Clamp(AttackerScore + ScorePerKill, 0.f, ScoreLimit);
	float ScoreDelta = NewAttackerScore - AttackerScore;
	PlayerStateCore->AddScore(ScoreDelta);

	if (NewAttackerScore >= ScoreLimit)
	{
		ScoreLimitReached();
	}
}


void AGameModeCore::ScoreLimitReached()
{
	if (!IsMatchInProgress())
	{
		return;
	}

	if (TimerHandle_MatchStates.IsValid())
	{
		GetWorldTimerManager().ClearTimer(TimerHandle_MatchStates);
		TimerHandle_MatchStates.Invalidate();
	}

	AGameStateCore *GameStateCore = GetGameState<AGameStateCore>();
	GameStateCore->SetRoundEndReason(ERoundEndReason::ERER_ScoreLimitReached);

	EndMatch();
}


void AGameModeCore::SelectMatchWinners()
{
	UWorld *World = GetWorld();
	if (!World)
	{
		return;
	}

	// Only select new winners if none currently exist.
	AGameStateCore *GameStateCore = World->GetGameState<AGameStateCore>();
	if (!GameStateCore->GetMatchWinners().IsEmpty())
	{
		return;
	}

	// Sort players by Score and ScorePriority.
	TArray<APlayerStateCore *> TopPlayers;
	for (APlayerState *PlayerState : GameStateCore->PlayerArray)
	{
		APlayerStateCore *PlayerStateCore = Cast<APlayerStateCore>(PlayerState);
		TopPlayers.Add(PlayerStateCore);
	}

	TopPlayers.StableSort([](const APlayerStateCore &PlayerStateCoreA, const APlayerStateCore &PlayerStateCoreB)
		{
			if (PlayerStateCoreA.GetScore() != PlayerStateCoreB.GetScore())
			{
				return PlayerStateCoreA.GetScore() > PlayerStateCoreB.GetScore();
			}

			return PlayerStateCoreA.GetScorePriority() < PlayerStateCoreB.GetScorePriority();
		});

	// Select only the winners between the top players.
	TArray<APlayerState *> Winners;
	int32 WinnersCount = FMath::Min(TopPlayers.Num(), MaxMatchWinners);
	for (int Index = 0; Index < WinnersCount; Index++)
	{
		Winners.Add(TopPlayers[Index]);
	}

	GameStateCore->SetMatchWinners(Winners);
}


void AGameModeCore::PostMatchSpectate()
{
	APlayerController *PlayerController = nullptr;
	for (APlayerState *PlayerState : GameState->PlayerArray)
	{
		if (!IsValid(PlayerState))
		{
			continue;
		}

		PlayerController = PlayerState->GetPlayerController();
		if (IsValid(PlayerController))
		{
			APawn *Pawn = PlayerController->GetPawn();
			if (IsValid(Pawn))
			{
				Pawn->Destroy();
			}

			APawn *SpectatorPawn = GetWorld()->SpawnActor<APawn>(SpectatorClass);
			PlayerController->Possess(SpectatorPawn);
		}
	}
}