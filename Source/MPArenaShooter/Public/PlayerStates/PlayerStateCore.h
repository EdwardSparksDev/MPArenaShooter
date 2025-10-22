// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "PlayerStateCore.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStringChangedSignature, FString, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFloatChangedSignature, float, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FIntChangedSignature, int32, Value);


UCLASS()
class MPARENASHOOTER_API APlayerStateCore : public APlayerState
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable, Category = "PlayerState")
	FIntChangedSignature OnPlayerIdChanged;

	UPROPERTY(BlueprintAssignable, Category = "PlayerState")
	FStringChangedSignature OnPlayerNameChanged;

	UPROPERTY(BlueprintAssignable, Category = "PlayerState")
	FFloatChangedSignature OnScoreChanged;

	UPROPERTY(BlueprintAssignable, Category = "PlayerState")
	FIntChangedSignature OnKillsChanged;

	UPROPERTY(BlueprintAssignable, Category = "PlayerState")
	FIntChangedSignature OnDeathsChanged;


	APlayerStateCore();

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > &OutLifetimeProps) const override;

	virtual void OnRep_PlayerId() override;

	virtual void OnRep_PlayerName() override;

	virtual void OnRep_Score() override;


	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	void AddScore(float Value);

	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	FORCEINLINE int32 GetScorePriority() const { return ScorePriority; }

	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	void SetScorePriority(int32 NewScorePriority);

	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	FORCEINLINE int32 GetKills() const { return Kills; }

	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	void AddKill();

	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	FORCEINLINE int32 GetDeaths() const { return Deaths; }

	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	void AddDeath();


protected:

	UFUNCTION()
	void OnRep_ScorePriority();

	UFUNCTION()
	void OnRep_Kills();

	UFUNCTION()
	void OnRep_Deaths();


private:

	UPROPERTY(ReplicatedUsing = OnRep_ScorePriority)
	int32 ScorePriority = -1;

	UPROPERTY(ReplicatedUsing = OnRep_Kills)
	int32 Kills = 0;

	UPROPERTY(ReplicatedUsing = OnRep_Deaths)
	int32 Deaths = 0;


	// This variable is only used by clients to notify a Score change only when both Score and ScorePriority have been both replicated.
	bool bNotifyClientScoreChanged = false;
};