// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "CharacterBase.h"
#include "WeaponAnimInstance.h"
#include "Kismet/GameplayStatics.h"
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
	if (WeaponAnimInstance)
		WeaponAnimInstance->Reload(SelfReloadMontage);
	return true;
}

bool AWeapon::Shoot(FVector& EndLocation)
{
	if (bIsReloadingWeapon) return false;
	if (CurrentBulletsLeft <= 0) {
		// TODO: Make sound of empty gun
		return false;
	}
	CurrentBulletsLeft -= 1;
	if (WeaponAnimInstance)
		WeaponAnimInstance->Shoot(SelfShootMontage);
	const FVector MuzzleLocation = GetSkeletalMeshComponent()->GetSocketLocation(FName("Muzzle"));
	// TODO: spawn muzzle etc.
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	Params.AddIgnoredActor(this);
	// DrawDebugLine(GetWorld(), MuzzleLocation, EndLocation, FColor::Red, true, -1.f, 0U, 5.f);
	const bool bIsHit = GetWorld()->LineTraceSingleByChannel(HitResult, MuzzleLocation, EndLocation, ECollisionChannel::ECC_WorldStatic, Params);
	
	if (!bIsHit) return true;
	// UE_LOG(LogTemp, Warning, TEXT("HIT %s"), *HitResult.GetActor()->GetName());
	UGameplayStatics::ApplyPointDamage(HitResult.GetActor(), BaseDamage, EndLocation - MuzzleLocation, HitResult, GetInstigatorController(), GetOwner(), WeaponDamageType);
	//if (ACharacterBase* CharacterHit = Cast<ACharacterBase>(HitResult.GetActor())) {
		
		//CharacterHit->HitByWeapon(HitResult.ImpactPoint, HitResult.ImpactNormal, BaseDamage);
	//}



	// TODO: Spawn hit marker
	return true;
}

bool AWeapon::WeaponHasOwner() const
{
	return GetOwner() != nullptr;
}
