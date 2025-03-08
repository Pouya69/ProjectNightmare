// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "CharacterBase.h"
#include "WeaponAnimInstance.h"
#include "Animation/AimOffsetBlendSpace.h"

AWeapon::AWeapon()
{
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool AWeapon::Reload()
{
	if (CurrentBulletsLeft >= MaxBulletsMagazine) return false;
	if (TotalBulletsLeft <= 0) return false;
	WeaponAnimInstance->Reload(ReloadMontage);
	return true;
}

bool AWeapon::Shoot(FVector& EndLocation)
{
	if (CurrentBulletsLeft <= 0 || bIsReloadingWeapon) return false;
	WeaponAnimInstance->Shoot(ShootMontage);
	FVector MuzzleLocation = GetSkeletalMeshComponent()->GetSocketLocation(FName("Muzzle"));
	// TODO: spawn muzzle etc.
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	Params.AddIgnoredActor(this);
	const bool bIsHit = GetWorld()->LineTraceSingleByChannel(HitResult, MuzzleLocation, EndLocation, ECollisionChannel::ECC_Visibility);
	if (!bIsHit) return true;
	if (ACharacterBase* CharacterHit = Cast<ACharacterBase>(HitResult.GetActor())) {
		CharacterHit->HitByWeapon(HitResult.ImpactPoint, HitResult.ImpactNormal, BaseDamage);
	}
	return true;
}

bool AWeapon::WeaponHasOwner() const
{
	return GetOwner() != nullptr;
}
