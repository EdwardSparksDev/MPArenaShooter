// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "Types/RoundEndReason.h"
#include "GameStateCore.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerStateUpdatedSignature, APlayerState *, PlayerState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerStateCoreUpdatedSignature, APlayerStateCore *, PlayerStateCore);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchStateChangedSignature, FName, State);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRoundStateChangedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundTimerUpdatedSignature, int32, RoundTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnKillfeedUpdatedSignature, APlayerStateCore *, AttackerPlayerStateCore,
	APlayerStateCore *, EliminatedPlayerStateCore, AActor *, DamageCauser);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundEndReasonDefinedSignature, ERoundEndReason, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchWinnersSelectedSignature, TArray<APlayerState *>, Winners);


UCLASS()
class MPARENASHOOTER_API AGameStateCore : public AGameState
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable, Category = "GameState")
	FOnPlayerStateUpdatedSignature OnPlayerStateAdded;

	UPROPERTY(BlueprintAssignable, Category = "GameState")
	FOnPlayerStateUpdatedSignature OnPlayerStateRemoved;

	UPROPERTY(BlueprintAssignable, Category = "GameState")
	FOnMatchStateChangedSignature OnMatchStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Match")
	FOnRoundStateChangedSignature OnRoundStarted;

	UPROPERTY(BlueprintAssignable, Category = "Match")
	FOnRoundStateChangedSignature OnRoundEnded;

	UPROPERTY(BlueprintAssignable, Category = "Match")
	FOnRoundTimerUpdatedSignature OnPreRoundTimerUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Match")
	FOnRoundTimerUpdatedSignature OnRoundTimerUpdated;

	UPROPERTY(BlueprintAssignable, Category = "GameState")
	FOnKillfeedUpdatedSignature OnKillfeedUpdated;

	UPROPERTY(BlueprintAssignable, Category = "GameState")
	FOnPlayerStateCoreUpdatedSignature OnFirstBlood;

	UPROPERTY(BlueprintAssignable, Category = "Match")
	FOnRoundEndReasonDefinedSignature OnRoundEndReasonDefined;

	UPROPERTY(BlueprintAssignable, Category = "Match")
	FOnMatchWinnersSelectedSignature OnMatchWinnersSelected;


	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Match")
	uint8 PreRoundDuration = -1; // Will get initialized at runtime by the GameModeCore.

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Match");
	int32 RoundDuration = -1; // Will get initialized at runtime by the GameModeCore.

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Match");
	int32 ScoreLimit = -1; // Will get initialized at runtime by the GameModeCore.


	AGameStateCore();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

	virtual void PostInitializeComponents() override;

	virtual void BeginPlay() override;

	virtual void UpdateServerTimeSeconds() override;

	virtual void OnRep_ReplicatedWorldTimeSecondsDouble() override;

	// Overridden to return the ServerWorldTimeSeconds + half of the player's ping (half the RTT).
	virtual double GetServerWorldTimeSeconds() const override;

	virtual void AddPlayerState(APlayerState *PlayerState) override;

	virtual void RemovePlayerState(APlayerState *PlayerState) override;

	virtual void OnRep_MatchState() override;


	UFUNCTION(BlueprintCallable, Category = "Match")
	FORCEINLINE int32 GetPreRoundTime() const { return PreRoundTimer; }

	UFUNCTION(BlueprintCallable, Category = "Match")
	FORCEINLINE int32 GetRoundTime() const { return RoundTimer; };

	UFUNCTION(BlueprintCallable, Category = "Match")
	FORCEINLINE int32 GetScoreLimit() const { return ScoreLimit; };

	UFUNCTION(NetMulticast, Reliable, Category = "Match")
	void NotifyKillfeed(APlayerStateCore *AttackerPlayerStateCore, APlayerStateCore *EliminatedPlayerStateCore, AActor *DamageCauser);

	UFUNCTION(NetMulticast, Reliable, Category = "Match")
	void NotifyFirstBlood(APlayerStateCore *PlayerStateCore);

	UFUNCTION(BlueprintCallable, Category = "Match")
	FORCEINLINE ERoundEndReason GetRoundEndReason() const { return RoundEndReason; };

	void SetRoundEndReason(ERoundEndReason NewReason);

	UFUNCTION(BlueprintCallable, Category = "Match")
	FORCEINLINE TArray<APlayerState *> GetMatchWinners() const { return MatchWinners; };

	void SetMatchWinners(TArray<APlayerState *> NewWinners);


protected:

	// Mitigates the effect of lag spikes on clients.
	// WARNING: if enabled, may cause temporary world time desync between server and clients when time dilation gets modified.
	UPROPERTY(EditDefaultsOnly, Category = "GameState")
	bool bServerWorldTimeSecondsSmoothing = true;


	UPROPERTY(EditDefaultsOnly, Category = "MatchSettings")
	UMaterial *MatchPostProcessMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "MatchSettings", meta = (EditCondition = "MatchPostProcessMaterial != nullptr",
		EditConditionHides))
	FName MatchPPMControlParameterName = FName("BlendWeight");

	UPROPERTY(EditDefaultsOnly, Category = "MatchSettings", meta = (ClampMin = "0", EditCondition = "MatchPostProcessMaterial != nullptr",
		EditConditionHides))
	float MatchPPMBlendTime = .5f;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MatchSettings")
	USoundBase *VictoryTheme;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MatchSettings")
	USoundBase *DefeatTheme;


	int32 PreRoundTimer = -1; // Will get initialized at runtime.

	int32 RoundTimer = -1; // Will get initialized at runtime.

	UPROPERTY(ReplicatedUsing = OnRep_ServerRoundStartingTime)
	float ServerRoundStartingTime = -1.f;

	UPROPERTY(ReplicatedUsing = OnRep_ServerRoundEndingTime)
	float ServerRoundEndingTime = -1.f;

	UPROPERTY(ReplicatedUsing = OnRep_MatchWinners)
	TArray<APlayerState *> MatchWinners;


	FTimerHandle TimerHandle_PreRoundTimer;


	virtual void HandleMatchIsWaitingToStart() override;

	virtual void HandleMatchHasStarted() override;

	virtual void HandleMatchHasEnded() override;


	virtual void UpdatePreRoundTimer();

	template<class UserClass>
	void ScheduleIntTimerUpdate(FTimerHandle &TimerHandle, float RemainingTimerTime, UserClass *InObj,
		typename FTimerDelegate::TMethodPtr<UserClass> InTimerMethod);

	UFUNCTION()
	virtual void OnRep_ServerRoundStartingTime();

	virtual void UpdateRoundTimer();

	UFUNCTION()
	virtual void OnRep_ServerRoundEndingTime();

	UFUNCTION()
	virtual void OnRep_RoundEndReason();

	UFUNCTION()
	virtual void OnRep_MatchWinners();


private:

	UPROPERTY(ReplicatedUsing = OnRep_RoundEndReason, VisibleInstanceOnly, Category = "Match");
	ERoundEndReason RoundEndReason = ERoundEndReason::ERER_Undefined; // Will get initialized at runtime by the GameModeCore.


	float CurrentMatchPPMBlendValue = 0.f;

	bool bMatchPPMBlendForward = false;

	UPROPERTY()
	UMaterialInstanceDynamic *MatchPPMDynamic;


	bool bClientMatchStateInitialized = false;


	UFUNCTION()
	void SortPlayerArray(int32 PlayerId = -1);


	void NotifyServerPreRoundTimerEnd();

	void NotifyServerRoundTimerEnd();


	void CreateMatchPostProcessMaterials();

	void BlendMatchPostProcessMaterial(float TargetValue);

	void UpdateMatchPPMBlendValue();


	void PlayMatchOutcomeTheme();
};