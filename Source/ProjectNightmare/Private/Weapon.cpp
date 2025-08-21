// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "CharacterBase.h"
#include "WeaponAnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/AimOffsetBlendSpace.h"
#include "ThirdPersonPlayerCharacter.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(FName("Mesh"));
	Mesh->SetupAttachment(GetRootComponent());
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	CurrentFireRatePoint = 100.f;
	if (IsDefaultWeapon) return;
	if (Mesh->GetAnimInstance() != nullptr)
		WeaponAnimInstance = Cast<UWeaponAnimInstance>(Mesh->GetAnimInstance());

}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bIsFiring) return;
	CurrentFireRatePoint = FMath::FInterpConstantTo(CurrentFireRatePoint, 100.f, DeltaTime, FireRate);
	// UE_LOG(LogTemp, Warning, TEXT("%f"), CurrentFireRatePoint);
}

void AWeapon::SetFocusMaterial(bool bIsFocused)
{
	Mesh->SetOverlayMaterial(bIsFocused ? FocusedMaterial : nullptr);
}

void AWeapon::InteractionComplete(AThirdPersonPlayerCharacter* PlayerCharacterRef)
{
	PlayerCharacterRef->PickupWeapon(this);
}

void AWeapon::PickedUpWeapon()
{
	if (IsDefaultWeapon) return;
	bIsInteractable = false;
	Mesh->SetGenerateOverlapEvents(false);
	Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECR_Ignore);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetInRange(false);
}

void AWeapon::DroppedWeapon()
{
	if (IsDefaultWeapon) return;
	bIsInteractable = true;
	Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECR_Overlap);
	Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECR_Block);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetGenerateOverlapEvents(true);
	SetInRange(false);
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

float AWeapon::GetWeaponDamage(const float InBaseDamage, const float HitDistanceFromShootingPosition) const
{
	return InBaseDamage + ((1 - InBaseDamage) / (DamageFalloffDistanceMax - DamageFalloffDistanceStart)) * (HitDistanceFromShootingPosition - DamageFalloffDistanceStart);
}

bool AWeapon::Reload()
{
	if (CurrentBulletsLeft >= MaxBulletsMagazine) return false;
	if (TotalBulletsLeft <= 0) return false;
	// PlayerCharacterRef->PlayAnimMontage(CharacterReloadMontage);
	// if (IsDefaultWeapon) return true;  // Default weapons attached to skeletal meshes do not need animations. Animations are on characters.
	if (WeaponAnimInstance)
		WeaponAnimInstance->Reload(SelfReloadMontage);
	BulletsToAddAfterReloadComplete = MaxBulletsMagazine - CurrentBulletsLeft > TotalBulletsLeft ? TotalBulletsLeft : MaxBulletsMagazine - CurrentBulletsLeft;
	return true;
}

void AWeapon::ReloadCancelled()
{
	BulletsToAddAfterReloadComplete = 0;
}

void AWeapon::RefillAmmo(int Amount)
{
	TotalBulletsLeft = Amount <= 0 ? MaxBulletsHeld : FMath::Min(TotalBulletsLeft + Amount, MaxBulletsHeld);
}

void AWeapon::ReloadComplete(const uint8 BulletsToAdd)
{
	if (BulletsToAdd <= 0) {
		CurrentBulletsLeft += BulletsToAddAfterReloadComplete;
		TotalBulletsLeft -= BulletsToAddAfterReloadComplete;
		BulletsToAddAfterReloadComplete = 0;
		return;
	}
	BulletsToAddAfterReloadComplete = 0;
	TotalBulletsLeft -= BulletsToAdd;
	CurrentBulletsLeft += BulletsToAdd;
}

bool AWeapon::Shoot(const FVector& StartLocation, const FVector& EndLocation, bool bForceShoot, bool bEventShot, float CustomDamage)
{
	BulletsToAddAfterReloadComplete = 0;
	if (!bEventShot) {
		if (!bForceShoot && (bIsReloadingWeapon || CurrentFireRatePoint < 99.f)) return false;
		if (CurrentBulletsLeft <= 0) {
			// TODO: Make sound of empty gun
			return false;
		}
		CurrentBulletsLeft -= 1;
	}
	CurrentFireRatePoint = 0;
	// if (!IsDefaultWeapon) return true;  // Default weapons attached to skeletal meshes do not need animations. Animations are on characters.
	const FVector MuzzleLocation = Mesh->GetSocketLocation(FName("Muzzle"));
	// TODO: spawn muzzle etc.
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	Params.AddIgnoredActor(this);
	// DrawDebugLine(GetWorld(), MuzzleLocation, EndLocation, FColor::Red, true, -1.f, 0U, 5.f);
	const bool bIsHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECollisionChannel::ECC_Visibility, Params);
	
	if (!bIsHit) return true;
	float FinalDamage = GetWeaponDamage(CustomDamage <= 0 ? BaseDamage : CustomDamage, HitResult.Distance);
	if (!HitResult.BoneName.IsNone())
		const bool bWasCriticalHit = GetDamageMultiplierBoneHit(HitResult.BoneName, FinalDamage);

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
