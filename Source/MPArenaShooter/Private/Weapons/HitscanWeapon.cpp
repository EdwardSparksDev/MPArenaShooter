// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/HitscanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"


void AHitscanWeapon::PerformFire(const FVector &Target, float AimValue)
{
	Super::PerformFire(Target, AimValue);

	const USkeletalMeshSocket *MuzzleSocket = WeaponMesh->GetSocketByName(MuzzleSocketName);
	if (!MuzzleSocket)
	{
		return;
	}

	FTransform SocketTransform = MuzzleSocket->GetSocketTransform(WeaponMesh);
	FVector Start = SocketTransform.GetLocation();

	FHitResult Hit;
	float Spread = FMath::Lerp(MaxSpreadAngle, MaxAimSpreadAngle, AimValue);
	WeaponTraceHit(Start, Target, Hit, Spread);

	if (Hit.bBlockingHit)
	{
		if (HasAuthority() && IsValid(Hit.GetActor()))
		{
			UGameplayStatics::ApplyDamage(Hit.GetActor(), Damage, FiringController, this, UDamageType::StaticClass());
		}

		if (ImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, Hit.ImpactPoint, Hit.ImpactNormal.Rotation(), true,
				EPSCPoolMethod::AutoRelease);
		}
	}
}


void AHitscanWeapon::WeaponTraceHit(const FVector &TraceStart, const FVector &HitTarget, FHitResult &OutHit, float Spread)
{
	UWorld *World = GetWorld();
	if (!World)
	{
		return;
	}

	FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	if (Spread > 0.f)
	{
		ToTargetNormalized = SpreadStream.VRandCone(ToTargetNormalized, FMath::DegreesToRadians(Spread));
	}
	FVector End = TraceStart + ToTargetNormalized * MaxRange;

	World->LineTraceSingleByChannel(OutHit, TraceStart, End, ECollisionChannel::ECC_Visibility);

	if (BeamParticles)
	{
		UParticleSystemComponent *BeamParticlesSystemComp = UGameplayStatics::SpawnEmitterAtLocation(World, BeamParticles, TraceStart,
			FRotator::ZeroRotator, true, EPSCPoolMethod::AutoRelease);
		if (BeamParticlesSystemComp)
		{
			BeamParticlesSystemComp->SetVectorParameter(FName("Target"), OutHit.bBlockingHit ? OutHit.ImpactPoint : End);
		}
	}
}