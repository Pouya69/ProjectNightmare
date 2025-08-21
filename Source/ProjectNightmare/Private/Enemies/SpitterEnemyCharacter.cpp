// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemies/SpitterEnemyCharacter.h"
#include "Enemies/Spitter_Projectile.h"

void ASpitterEnemyCharacter::SpawnSpit(const FVector TargetHitLocation)
{
	const FTransform SpawnTransform = FTransform(FRotator::ZeroRotator, GetMesh()->GetSocketLocation("ShootLocation"), FVector::OneVector);
	ASpitter_Projectile* SpawnedSpit = GetWorld()->SpawnActorDeferred<ASpitter_Projectile>(SpitterProjectileClass, SpawnTransform);
	if (!SpawnedSpit) return;
	// TODO
	SpawnedSpit->FinishSpawning(SpawnTransform);
}
