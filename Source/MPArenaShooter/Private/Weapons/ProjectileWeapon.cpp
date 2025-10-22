// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Weapons/ProjectileBase.h"


void AProjectileWeapon::PerformFire(const FVector &Target, float AimValue)
{
	Super::PerformFire(Target, AimValue);

	// Only fire the weapon on the server (since it will spawn a replicated projectile).
	if (!HasAuthority())
	{
		return;
	}

	const USkeletalMeshSocket *MuzzleSocket = WeaponMesh->GetSocketByName(MuzzleSocketName);
	if (!MuzzleSocket)
	{
		return;
	}

	// Get the spawn location from the weapon's muzzle.
	FTransform SocketTransform = MuzzleSocket->GetSocketTransform(WeaponMesh);
	FVector ToTarget = Target - SocketTransform.GetLocation();

	if (ProjectileClass && FiringInstigator)
	{
		// Set the projectile's owner and instigator in the SpawnParams.
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = FiringInstigator;

		UWorld *World = GetWorld();
		if (World)
		{
			AProjectileBase *Projectile = World->SpawnActor<AProjectileBase>(ProjectileClass, SocketTransform.GetLocation(),
				ToTarget.Rotation(), SpawnParams);
			Projectile->Initialize(FiringController, this);
		}
	}
}