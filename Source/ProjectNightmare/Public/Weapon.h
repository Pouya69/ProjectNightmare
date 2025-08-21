// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableObject.h"
#include "Weapon.generated.h"

/**
 * 
 */


USTRUCT(BlueprintType)
struct FWeaponDataTable : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int WeaponID = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString Description;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* WeaponImage;
};

UCLASS()
class PROJECTNIGHTMARE_API AWeapon : public AInteractableObject
{
	GENERATED_BODY()
	
public:
	// Sets default values for this character's properties
	AWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		int WeaponID = 0;
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	// This matches with the Player Combo Component ECombatComboType.
	UPROPERTY(EditAnywhere)
		uint8 WeaponCombatType;
	UPROPERTY(EditAnywhere)
		bool IsDefaultWeapon = false;
	UPROPERTY(EditAnywhere)
		bool DoesReloadAddOneBullet = false;

	UPROPERTY(EditAnywhere)
		float FireRate = -1;
	UPROPERTY(EditAnywhere)
		int ShotsDoneBeforeGravityEnabled = 5;
	bool bIsFiring = false;
	float CurrentFireRatePoint = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FName WeaponName;

	UPROPERTY(EditAnywhere)
		USkeletalMeshComponent* Mesh;

	UPROPERTY(EditAnywhere)
		TSubclassOf<UCameraShakeBase> ShootingCameraShake;
	UFUNCTION(BlueprintCallable)
		FORCEINLINE USkeletalMeshComponent* GetMesh() { return Mesh; }
	
	virtual void SetFocusMaterial(bool bIsFocused) override;
	virtual void InteractionComplete(class AThirdPersonPlayerCharacter* PlayerCharacterRef) override;
	virtual void PickedUpWeapon();
	virtual void DroppedWeapon();
public:
	// Animation
	class UWeaponAnimInstance* WeaponAnimInstance;
	UPROPERTY(EditAnywhere)
		UAnimMontage* ReloadMontage;
	UPROPERTY(EditAnywhere)
		UAnimMontage* CharacterReloadMontage;
	UPROPERTY(EditAnywhere)
		UAnimMontage* CharacterReloadMovingMontage;
	UPROPERTY(EditAnywhere)
		UAnimMontage* ReloadADSMontage;
	UPROPERTY(EditAnywhere)
		UAnimMontage* CharacterReloadADSMontage;
	UPROPERTY(EditAnywhere)
		UAnimMontage* ShootMontage;
	UPROPERTY(EditAnywhere)
		UAnimMontage* CharacterShootMontage;
	UPROPERTY(EditAnywhere)
		UAnimMontage* CharacterMovingShootMontage;
	UPROPERTY(EditAnywhere)
		UAnimMontage* CharacterMovingADSShootMontage;
	UPROPERTY(EditAnywhere)
		UAnimMontage* CharacterShootMontage_ComboEnd;
	UPROPERTY(EditAnywhere)
		UAnimMontage* EquipMontage;
	UPROPERTY(EditAnywhere)
		UAnimMontage* UnEquipMontage;
	UPROPERTY(EditAnywhere)
		UAnimMontage* SelfReloadMontage;
	UPROPERTY(EditAnywhere)
		UAnimMontage* SelfReloadADSMontage;
	UPROPERTY(EditAnywhere)
		UAnimMontage* SelfShootMontage;
	UPROPERTY(EditAnywhere)
		UAnimMontage* SelfEquipMontage;
	UPROPERTY(EditAnywhere)
		UAnimMontage* SelfUnEquipMontage;
	UPROPERTY(EditAnywhere)
		class UAimOffsetBlendSpace* HIP_AimOffset;
	UPROPERTY(EditAnywhere)
		class UAimOffsetBlendSpace* AIM_AimOffset;

public:
	// Damage multiplier AND dismemberment
	UFUNCTION(BlueprintCallable, Category="Damage multiplier AND dismemberment")
		bool GetDamageMultiplierBoneHit(const FName& BoneName, float& Damage) const;

	UPROPERTY(EditAnywhere, Category="Damage multiplier AND dismemberment")
		float HeadshotDamageMultiplier = 2.f;
	UPROPERTY(EditAnywhere, Category = "Damage multiplier AND dismemberment")
		float UpperbodyDamageMultiplier = 1.5f;
	UPROPERTY(EditAnywhere, Category = "Damage multiplier AND dismemberment")
		float LowerbodyDamageMultiplier = 1.2f;
	

public:
	UPROPERTY(EditAnywhere)
		TSubclassOf<UDamageType> WeaponDamageType;
	UPROPERTY(EditAnywhere)
		bool bShouldAimToShoot;
	// Combat
	UPROPERTY(EditAnywhere)
		float BaseDamage;
	UPROPERTY(EditAnywhere)
		float DamageFalloffDistanceStart = 100.f;
	// If distance is equal or more than this, the damage will be 1.
	UPROPERTY(EditAnywhere)
		float DamageFalloffDistanceMax = 800.f;
	// If less equal 0, that means no falloff.
	// ShootLocation is when it got shot. JUST 
	UFUNCTION(BlueprintCallable)
		FORCEINLINE float GetWeaponDamage(const float InBaseDamage, const float HitDistanceFromShootingPosition) const;
	UPROPERTY(BlueprintReadWrite)
		int BulletsToAddAfterReloadComplete = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		int CurrentBulletsLeft;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		int MaxBulletsMagazine;
	// Max amount that can be held in inventory.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		int MaxBulletsHeld = 30;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		int TotalBulletsLeft;
	UFUNCTION(BlueprintCallable)
		virtual bool Reload();
	UFUNCTION(BlueprintCallable)
		void RefillAmmo(int Amount = -1);
	UFUNCTION(BlueprintCallable)
		virtual void ReloadCancelled();
	UFUNCTION(BlueprintCallable)
		virtual void ReloadComplete(const uint8 BulletsToAdd = 0);
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool bIsReloadingWeapon;
	UFUNCTION(BlueprintCallable)
		virtual bool Shoot(const FVector& StartLocation, const FVector& EndLocation, bool bForceShoot = false, bool bEventShot = false, float CustomDamage=-1.f);
	UFUNCTION(BlueprintCallable)
		virtual bool WeaponHasOwner() const;
};
