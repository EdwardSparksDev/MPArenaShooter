// Fill out your copyright notice in the Description page of Project Settings.

#include "UserInterface/OverheadWidget.h"
#include "Components/TextBlock.h"


void UOverheadWidget::NativeDestruct()
{
	RemoveFromParent();

	Super::NativeDestruct();
}


void UOverheadWidget::SetDisplayText(FString Text)
{
	if (DisplayText_TXT)
	{
		DisplayText_TXT->SetText(FText::FromString(Text));
	}
}


void UOverheadWidget::DisplayPlayerNetRole(APawn *Pawn, bool bLocalRole)
{
	ENetRole SwitchRole = bLocalRole ? Pawn->GetLocalRole() : Pawn->GetRemoteRole();
	FString Role;

	switch (SwitchRole)
	{
	case ENetRole::ROLE_Authority:
		Role = FString("Authority");
		break;
	case ENetRole::ROLE_AutonomousProxy:
		Role = FString("Autonomous Proxy");
		break;
	case ENetRole::ROLE_SimulatedProxy:
		Role = FString("Simulated Proxy");
		break;
	case ENetRole::ROLE_None:
		Role = FString("None");
		break;
	}

	FString DisplayText = FString::Printf(TEXT("%s Role: %s"), bLocalRole ? TEXT("Local") : TEXT("Remote"), *Role);
	SetDisplayText(DisplayText);
}