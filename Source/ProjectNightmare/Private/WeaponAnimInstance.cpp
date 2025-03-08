// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponAnimInstance.h"
#include "Weapon.h"

UWeaponAnimInstance::UWeaponAnimInstance()
{
}

void UWeaponAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);
}

void UWeaponAnimInstance::Shoot(UAnimMontage* ShootMontage)
{
	Montage_Play(ShootMontage);
}

void UWeaponAnimInstance::Reload(UAnimMontage* ReloadMontage)
{
	Montage_Play(ReloadMontage);
}

void UWeaponAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	OwnerWeapon = Cast<AWeapon>(GetOwningActor());
	if (!OwnerWeapon) {
		UE_LOG(LogTemp, Warning, TEXT("Owner Weapon Invalid For animation instance: %s"), *GetFullName());
		return;
	}
	
}
