// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlayerControllerCore.generated.h"

class UInputAction;
class UInputMappingContext;
class AHUDCore;
struct FInputActionValue;


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerStateReplicatedSignature);


UCLASS()
class MPARENASHOOTER_API APlayerControllerCore : public APlayerController
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable, Category = "PlayerController")
	FOnPlayerStateReplicatedSignature OnPlayerStateReplicated;


	virtual void OnRep_PlayerState() override;


protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputMappingContext *UIControls;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction *ToggleScoreboardAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction *ScrollScoreboardAction;


	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	AHUDCore *HUDCore;

	bool bDisplayingScoreboard = false;


	virtual void BeginPlay() override;

	virtual void SetupInputComponent() override;


private:

	void ToggleScoreboard(const FInputActionValue &Value);

	void ScrollScoreboard(const FInputActionValue &Value);
};