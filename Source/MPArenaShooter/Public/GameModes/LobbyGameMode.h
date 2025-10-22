// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LobbyGameMode.generated.h"


UCLASS()
class MPARENASHOOTER_API ALobbyGameMode : public AGameMode
{
	GENERATED_BODY()

public:

	ALobbyGameMode();

	virtual void PostLogin(APlayerController *NewPlayer) override;


private:

	UPROPERTY(EditAnywhere, Category = "MatchSettings")
	FString MapPath = FString("/Game/Levels/Build/L_Arena");
};