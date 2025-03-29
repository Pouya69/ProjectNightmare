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

bool AWeapon::GetDamageMultiplierBoneHit(const FName& BoneName, float& Damage) const
{
	// default
	// if (BoneName.IsEqual("pelvis") || BoneName.IsEqual("spine_01") || BoneName.IsEqual("spine_02") || BoneName.IsEqual("upperarm_l") || BoneName.IsEqual("upperarm_r") || BoneName.IsEqual("lowerarm_l") || BoneName.IsEqual("lowerarm_r"))
	if (BoneName.IsEqual("head") || BoneName.IsEqual("neck_01")) {
		Damage *= HeadshotDamageMultiplier;
		return true;
	}
	else if (BoneName.IsEqual("clavicle_l") || BoneName.IsEqual("clavicle_r") || BoneName.IsEqual("spine_02") || BoneName.IsEqual("spine_03") || BoneName.IsEqual("upperarm_twist_01_l") || BoneName.IsEqual("upperarm_twist_01_r"))
		Damage *= UpperbodyDamageMultiplier;
	else if (BoneName.IsEqual("spine_01") || BoneName.IsEqual("pelvis"))
		Damage *= LowerbodyDamageMultiplier;
	return false;
}

bool AWeapon::Reload()
{
	if (CurrentBulletsLeft >= MaxBulletsMagazine) return false;
	if (TotalBulletsLeft <= 0) return false;
	if (WeaponAnimInstance)
		WeaponAnimInstance->Reload(SelfReloadMontage);
	return true;
}

bool AWeapon::Shoot(const FVector& StartLocation, const FVector& EndLocation)
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
	const bool bIsHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECollisionChannel::ECC_Visibility, Params);
	
	if (!bIsHit) return true;
	float FinalDamage = BaseDamage;
	if (!HitResult.BoneName.IsNone())
		const bool bWasCriticalHit = GetDamageMultiplierBoneHit(HitResult.BoneName, FinalDamage);
	// UE_LOG(LogTemp, Warning, TEXT("HIT %s"), *HitResult.GetActor()->GetName());

	UGameplayStatics::ApplyPointDamage(HitResult.GetActor(), FinalDamage, EndLocation - StartLocation, HitResult, GetInstigatorController(), GetOwner(), WeaponDamageType);
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
