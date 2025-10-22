// Fill out your copyright notice in the Description page of Project Settings.

#include "GameModes/LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"


ALobbyGameMode::ALobbyGameMode()
{
	PrimaryActorTick.bCanEverTick = false;
}


void ALobbyGameMode::PostLogin(APlayerController *NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// Currently for testing only. <-------------------
	int32 ConnectedPlayers = GameState.Get()->PlayerArray.Num();
	if (ConnectedPlayers == 2)
	{
		UWorld *World = GetWorld();
		if (World)
		{
			bUseSeamlessTravel = true;
			World->ServerTravel(FString::Printf(TEXT("%s?listen"), *MapPath));
		}
	}
}