// Fill out your copyright notice in the Description page of Project Settings.

#include "GameStates/GameStateCore.h"
#include "PlayerStates/PlayerStateCore.h"
#include "GameFramework/GameMode.h"
#include "Net/UnrealNetwork.h"
#include "Components/HealthComponent.h"
#include "Components/CombatComponent.h"
#include "Kismet/GameplayStatics.h"


AGameStateCore::AGameStateCore()
{
	PrimaryActorTick.bCanEverTick = false;

	ServerWorldTimeSecondsUpdateFrequency = 0.25f;
}


void AGameStateCore::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGameStateCore, PreRoundDuration);
	DOREPLIFETIME(AGameStateCore, RoundDuration);
	DOREPLIFETIME(AGameStateCore, ScoreLimit);

	DOREPLIFETIME(AGameStateCore, ServerRoundStartingTime);
	DOREPLIFETIME(AGameStateCore, ServerRoundEndingTime);

	DOREPLIFETIME(AGameStateCore, RoundEndReason);
	DOREPLIFETIME(AGameStateCore, MatchWinners);
}


void AGameStateCore::PostInitializeComponents()
{
	// Prevents ElapsedTime from updating starting in the AGameState PostInitializeComponents() (ElapsedTime will not be used).
	AGameStateBase::PostInitializeComponents();
}


void AGameStateCore::BeginPlay()
{
	Super::BeginPlay();

	CreateMatchPostProcessMaterials();
}


void AGameStateCore::UpdateServerTimeSeconds()
{
	Super::UpdateServerTimeSeconds();

	if (!HasMatchStarted())
	{
		UpdatePreRoundTimer();
	}
	else if (IsMatchInProgress())
	{
		UpdateRoundTimer();
	}
}


void AGameStateCore::OnRep_ReplicatedWorldTimeSecondsDouble()
{
	if (bServerWorldTimeSecondsSmoothing)
	{
		Super::OnRep_ReplicatedWorldTimeSecondsDouble();
	}
	else
	{
		UWorld *World = GetWorld();
		if (!World)
		{
			return;
		}

		ServerWorldTimeSecondsDelta = ReplicatedWorldTimeSecondsDouble - World->GetTimeSeconds();
	}

	if (!HasMatchStarted())
	{
		UpdatePreRoundTimer();
	}
	else if (IsMatchInProgress())
	{
		UpdateRoundTimer();
	}
}


double AGameStateCore::GetServerWorldTimeSeconds() const
{
	double Time = Super::GetServerWorldTimeSeconds();

	APlayerController *LocalPlayerController = GetGameInstance()->GetFirstLocalPlayerController();
	if (LocalPlayerController)
	{
		APlayerState *LocalPlayerState = LocalPlayerController->PlayerState;
		if (LocalPlayerState)
		{
			Time += LocalPlayerState->GetPingInMilliseconds() / 2000.f;
		}
	}

	return Time;
}


void AGameStateCore::AddPlayerState(APlayerState *PlayerState)
{
	Super::AddPlayerState(PlayerState);

	if (!HasAuthority())
	{
		if (PlayerState->GetPlayerId() == 0)
		{
			APlayerStateCore *PlayerStateCore = Cast<APlayerStateCore>(PlayerState);
			PlayerStateCore->OnPlayerIdChanged.AddDynamic(this, &ThisClass::SortPlayerArray);
		}
		else
		{
			SortPlayerArray();
		}
	}

	OnPlayerStateAdded.Broadcast(PlayerState);
}


void AGameStateCore::SortPlayerArray(int32 PlayerId)
{
	PlayerArray.Sort([](const APlayerState &A, const APlayerState &B)
		{
			return A.GetPlayerId() < B.GetPlayerId();
		});
}


void AGameStateCore::RemovePlayerState(APlayerState *PlayerState)
{
	Super::RemovePlayerState(PlayerState);

	OnPlayerStateRemoved.Broadcast(PlayerState);
}


void AGameStateCore::OnRep_MatchState()
{
	Super::OnRep_MatchState();

	// This if statement is only used to set the correct post process material blend value on client join.
	if (!bClientMatchStateInitialized && !HasAuthority())
	{
		bClientMatchStateInitialized = true;
		CurrentMatchPPMBlendValue = (MatchState == MatchState::InProgress ? 1.f : 0.f);
		MatchPPMDynamic->SetScalarParameterValue(MatchPPMControlParameterName, CurrentMatchPPMBlendValue);
	}

	OnMatchStateChanged.Broadcast(MatchState);
}


void AGameStateCore::HandleMatchIsWaitingToStart()
{
	Super::HandleMatchIsWaitingToStart();

	// Start updating PreRoundTimer.
	PreRoundTimer = PreRoundDuration;
	OnPreRoundTimerUpdated.Broadcast(PreRoundTimer);
	UpdatePreRoundTimer();

	BlendMatchPostProcessMaterial(0.f);
}


void AGameStateCore::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	// Start updating RoundTimer.
	NotifyServerPreRoundTimerEnd();
	RoundTimer = RoundDuration;
	OnRoundTimerUpdated.Broadcast(RoundTimer);
	UpdateRoundTimer();

	BlendMatchPostProcessMaterial(1.f);

	OnRoundStarted.Broadcast();
}


void AGameStateCore::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();

	NotifyServerRoundTimerEnd();

	BlendMatchPostProcessMaterial(0.f);

	OnRoundEnded.Broadcast();
}


void AGameStateCore::UpdatePreRoundTimer()
{
	if (HasMatchStarted() || PreRoundTimer <= 0)
	{
		return;
	}

	double ServerTime = GetServerWorldTimeSeconds();
	double RemainingPreRoundTime = FMath::Clamp(PreRoundDuration - ServerTime, 0.0, PreRoundDuration);
	int32 RemainingPreRoundSeconds = FMath::CeilToInt32(RemainingPreRoundTime);

	if (PreRoundTimer > RemainingPreRoundSeconds)
	{
		PreRoundTimer = RemainingPreRoundSeconds;
		OnPreRoundTimerUpdated.Broadcast(PreRoundTimer);
	}

	ScheduleIntTimerUpdate(TimerHandle_PreRoundTimer, RemainingPreRoundTime, this, &ThisClass::UpdatePreRoundTimer);
}


template<class UserClass>
void AGameStateCore::ScheduleIntTimerUpdate(FTimerHandle &TimerHandle, float RemainingTimerTime, UserClass *InObj,
	typename FTimerDelegate::TMethodPtr<UserClass> InTimerMethod)
{
	FTimerManager &TimerManager = GetWorldTimerManager();
	TimerManager.ClearTimer(TimerHandle);

	if (RemainingTimerTime > 0.f)
	{
		float UpdateDelay = FMath::Frac(RemainingTimerTime);
		if (FMath::IsNearlyZero(UpdateDelay))
		{
			UpdateDelay = 1.f;
		}

		TimerManager.SetTimer<UserClass>(TimerHandle, InObj, InTimerMethod, UpdateDelay);
	}
	else
	{
		TimerHandle.Invalidate();
	}
}


void AGameStateCore::NotifyServerPreRoundTimerEnd()
{
	if (HasAuthority())
	{
		ServerRoundStartingTime = GetServerWorldTimeSeconds();

		if (PreRoundTimer != 0)
		{
			PreRoundTimer = 0;
			OnPreRoundTimerUpdated.Broadcast(PreRoundTimer);
		}
	}
}


void AGameStateCore::OnRep_ServerRoundStartingTime()
{
	if (RoundTimer != RoundDuration)
	{
		RoundTimer = RoundDuration;
		OnRoundTimerUpdated.Broadcast(RoundTimer);
	}

	UpdateRoundTimer();
}


void AGameStateCore::UpdateRoundTimer()
{
	if (!IsMatchInProgress() || ServerRoundStartingTime < 0.f || RoundTimer <= 0)
	{
		return;
	}

	double ServerTime = GetServerWorldTimeSeconds();
	double ServerRoundTime = ServerTime - ServerRoundStartingTime;
	double RemainingRoundTime = FMath::Clamp(RoundDuration - ServerRoundTime, 0.0, RoundDuration);
	int32 RemainingRoundSeconds = FMath::CeilToInt32(RemainingRoundTime);

	if (RoundTimer > RemainingRoundSeconds)
	{
		RoundTimer = RemainingRoundSeconds;
		OnRoundTimerUpdated.Broadcast(RoundTimer);
	}

	ScheduleIntTimerUpdate(TimerHandle_DefaultTimer, RemainingRoundTime, this, &ThisClass::UpdateRoundTimer);
}


void AGameStateCore::NotifyServerRoundTimerEnd()
{
	if (HasAuthority())
	{
		// If the RoundTimer timer handle is still going (time limit reached) round the timer and send it to the clients.
		if (TimerHandle_DefaultTimer.IsValid() && RoundTimer != 0)
		{
			RoundTimer = 0;
			OnRoundTimerUpdated.Broadcast(RoundTimer);

			ServerRoundEndingTime = ServerRoundStartingTime + RoundDuration;
		}
		else
		{
			ServerRoundEndingTime = GetServerWorldTimeSeconds();
		}
	}
}


void AGameStateCore::OnRep_ServerRoundEndingTime()
{
	int32 RoundEndingTime = FMath::FloorToInt32(ServerRoundEndingTime - ServerRoundStartingTime);
	if (RoundTimer != RoundDuration - RoundEndingTime)
	{
		RoundTimer = 0;
		OnRoundTimerUpdated.Broadcast(RoundTimer);
	}
}


void AGameStateCore::NotifyFirstBlood_Implementation(APlayerStateCore *PlayerStateCore)
{
	OnFirstBlood.Broadcast(PlayerStateCore);
}


void AGameStateCore::NotifyKillfeed_Implementation(APlayerStateCore *AttackerPlayerStateCore,
	APlayerStateCore *EliminatedPlayerStateCore, AActor *DamageCauser)
{
	OnKillfeedUpdated.Broadcast(AttackerPlayerStateCore, EliminatedPlayerStateCore, DamageCauser);
}


void AGameStateCore::SetRoundEndReason(ERoundEndReason NewReason)
{
	if (HasAuthority() && RoundEndReason == ERoundEndReason::ERER_Undefined && NewReason != ERoundEndReason::ERER_Undefined)
	{
		RoundEndReason = NewReason;
		OnRoundEndReasonDefined.Broadcast(RoundEndReason);
	}
}


void AGameStateCore::OnRep_RoundEndReason()
{
	OnRoundEndReasonDefined.Broadcast(RoundEndReason);
}


void AGameStateCore::SetMatchWinners(TArray<APlayerState *> NewWinners)
{
	if (HasAuthority() && MatchWinners.IsEmpty())
	{
		MatchWinners = NewWinners;
		OnMatchWinnersSelected.Broadcast(MatchWinners);

		PlayMatchOutcomeTheme();
	}
}


void AGameStateCore::OnRep_MatchWinners()
{
	OnMatchWinnersSelected.Broadcast(MatchWinners);

	PlayMatchOutcomeTheme();
}


void AGameStateCore::CreateMatchPostProcessMaterials()
{
	if (!MatchPostProcessMaterial)
	{
		return;
	}

	UWorld *World = GetWorld();
	if (!World)
	{
		return;
	}

	// Spawn an unbound PostProcessVolume to apply effects globally.
	APostProcessVolume *PostProcessVolume = World->SpawnActor<APostProcessVolume>();
	if (!PostProcessVolume)
	{
		return;
	}
	PostProcessVolume->bUnbound = true;

	// Create a dynamic material instance and add it to the PostProcessVolume.
	if (MatchPostProcessMaterial)
	{
		MatchPPMDynamic = UMaterialInstanceDynamic::Create(MatchPostProcessMaterial, this);
		if (MatchPPMDynamic)
		{
			MatchPPMDynamic->SetScalarParameterValue(MatchPPMControlParameterName, 0.f);
			PostProcessVolume->AddOrUpdateBlendable(MatchPPMDynamic, 1.f);
		}
	}
}


void AGameStateCore::BlendMatchPostProcessMaterial(float TargetValue)
{
	if (!bClientMatchStateInitialized && !HasAuthority())
	{
		return;
	}

	if (MatchPPMDynamic && TargetValue != CurrentMatchPPMBlendValue)
	{
		bMatchPPMBlendForward = TargetValue > CurrentMatchPPMBlendValue;
		UpdateMatchPPMBlendValue();
	}
}


void AGameStateCore::UpdateMatchPPMBlendValue()
{
	UWorld *World = GetWorld();
	if (!World)
	{
		return;
	}

	float DeltaSeconds = World->GetDeltaSeconds() / MatchPPMBlendTime;
	float BlendValueIncrement = bMatchPPMBlendForward ? DeltaSeconds : -DeltaSeconds;
	CurrentMatchPPMBlendValue = FMath::Clamp(CurrentMatchPPMBlendValue + BlendValueIncrement, 0.f, 1.f);
	MatchPPMDynamic->SetScalarParameterValue(MatchPPMControlParameterName, CurrentMatchPPMBlendValue);

	float BlendTargetValue = bMatchPPMBlendForward ? 1.f : 0.f;
	float RemainingValueToBlend = CurrentMatchPPMBlendValue - BlendTargetValue;
	if (MatchPPMDynamic && RemainingValueToBlend != 0.f)
	{
		GetWorldTimerManager().SetTimerForNextTick(this, &ThisClass::UpdateMatchPPMBlendValue);
	}
}


void AGameStateCore::PlayMatchOutcomeTheme()
{
	UWorld *World = GetWorld();
	if (!World)
	{
		return;
	}

	APlayerController *LocalPlayerController = World->GetFirstPlayerController();
	APlayerState *LocalPlayerState = LocalPlayerController->GetPlayerState<APlayerState>();
	if (MatchWinners.Contains(LocalPlayerState))
	{
		if (VictoryTheme)
		{
			UGameplayStatics::PlaySound2D(this, VictoryTheme);
		}
	}
	else if (DefeatTheme)
	{
		UGameplayStatics::PlaySound2D(this, DefeatTheme);
	}
}