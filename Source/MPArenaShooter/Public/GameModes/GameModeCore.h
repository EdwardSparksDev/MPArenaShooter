// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GameModeCore.generated.h"

class APlayerControllerCore;
class APlayerStateCore;


UCLASS()
class MPARENASHOOTER_API AGameModeCore : public AGameMode
{
	GENERATED_BODY()

public:

	AGameModeCore();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent) override;
#endif

	virtual void InitGameState() override;

	virtual void HandleMatchIsWaitingToStart() override;

	virtual void HandleMatchHasStarted() override;

	virtual void HandleMatchHasEnded() override;

	virtual void FinishRestartPlayer(AController *NewPlayer, const FRotator &StartRotation) override;


protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MatchSettings", meta = (ClampMin = "1"))
	uint8 PreRoundDuration = 15;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MatchSettings", meta = (ClampMin = "1"))
	int32 RoundDuration = 600;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MatchSettings", meta = (ClampMin = "1"))
	uint8 PostMatchDuration = 15;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MatchSettings")
	bool bPostMatchSpectate = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MatchSettings", meta = (ClampMin = "1", EditCondition = "bPostMatchSpectate",
		EditConditionHides))
	uint8 PostMatchSpectateDelay = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MatchSettings", meta = (ClampMin = "0"))
	int32 ScoreLimit = 3000;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MatchSettings", meta = (ClampMin = "0"))
	int32 ScorePerKill = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MatchSettings", meta = (ClampMin = "1"))
	int32 MaxMatchWinners = 1;


	TArray<AActor *> PlayerSpawns;

	TQueue<AController *> RespawnQueue;


	virtual void BeginPlay() override;


	void TimeLimitReached();

	UFUNCTION()
	void OnPlayerEliminated(AController *AttackerController, AController *EliminatedController, AActor *DamageCauser);

	UFUNCTION()
	void RespawnPlayer();

	void RegisterPlayerDeath(AController *EliminatedController);

	void RegisterPlayerKill(AController *AttackerController);

	void ScoreLimitReached();

	void SelectMatchWinners();


private:

	FTimerHandle TimerHandle_MatchStates;

	FTimerHandle TimerHandle_PostMatchSpectate;

	// This value increases whenever a player changes their score. 
	// It helps tracking which player has scored sooner/later than others with the same score.
	int32 ScorePriorityIndex = 0;

	bool bFirstBlood = false;


	void TogglePawnComponentsBindings(APawn *Pawn, bool bEnable);

	void UpdatePlayerScore(APlayerStateCore *ScoringPlayerStateCore);

	void PostMatchSpectate();
};