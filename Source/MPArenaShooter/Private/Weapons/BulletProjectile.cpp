// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/BulletProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"


ABulletProjectile::ABulletProjectile()
{
	// Set up ProjectileMovementComponent.
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->MaxSpeed = 15000.f;
	ProjectileMovementComponent->InitialSpeed = ProjectileMovementComponent->MaxSpeed;
}