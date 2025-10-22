// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/HitscanShotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"


void AHitscanShotgun::PerformFire(const FVector &Target, float AimValue)
{
	AWeaponBase::PerformFire(Target, AimValue);

	const USkeletalMeshSocket *MuzzleSocket = WeaponMesh->GetSocketByName(MuzzleSocketName);
	if (!MuzzleSocket)
	{
		return;
	}

	FTransform SocketTransform = MuzzleSocket->GetSocketTransform(WeaponMesh);
	FVector Start = SocketTransform.GetLocation();

	ActorsHit.Empty();
	for (uint32 Index = 0; Index < PelletsPerShot; Index++)
	{
		FHitResult Hit;
		float Spread = FMath::Lerp(MaxSpreadAngle, MaxAimSpreadAngle, AimValue);
		WeaponTraceHit(Start, Target, Hit, Spread);

		if (HasAuthority() && Hit.bBlockingHit && IsValid(Hit.GetActor()))
		{
			if (ActorsHit.Contains(Hit.GetActor()))
			{
				ActorsHit[Hit.GetActor()]++;
			}
			else
			{
				ActorsHit.Emplace(Hit.GetActor(), 1);
			}
		}
	}

	if (HasAuthority())
	{
		for (TPair<AActor *, uint32> HitPair : ActorsHit)
		{
			UGameplayStatics::ApplyDamage(HitPair.Key, Damage * HitPair.Value, FiringController, this, UDamageType::StaticClass());
		}
	}
}