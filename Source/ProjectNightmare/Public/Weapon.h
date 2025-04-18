// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableObject.h"
#include "Weapon.generated.h"

/**
 * 
 */

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
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// If <= 0, it is a singleshot weapon. In Miliseconds


	UPROPERTY(EditAnywhere)
		float FireRate = -1;
	bool bIsFiring = false;
	float CurrentFireRatePoint = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FName WeaponName;

	UPROPERTY(EditAnywhere)
		USkeletalMeshComponent* Mesh;

	UFUNCTION(BlueprintCallable)
		FORCEINLINE USkeletalMeshComponent* GetMesh() { return Mesh; }
	
	virtual void SetFocusMaterial(bool bIsFocused) override;
	virtual void InteractionComplete(class AThirdPersonPlayerCharacter* PlayerCharacterRef) override;
	void PickedUpWeapon();
	void DroppedWeapon();
public:
	// Animation
	class UWeaponAnimInstance* WeaponAnimInstance;
	UPROPERTY(EditAnywhere)
		UAnimMontage* ReloadMontage;
	UPROPERTY(EditAnywhere)
		UAnimMontage* ReloadADSMontage;
	UPROPERTY(EditAnywhere)
		UAnimMontage* ShootMontage;
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
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		int CurrentBulletsLeft;
	UPROPERTY(EditAnywhere)
		int MaxBulletsMagazine;
	UPROPERTY(EditAnywhere)
		int TotalBulletsLeft;
	UFUNCTION(BlueprintCallable)
		bool Reload();
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool bIsReloadingWeapon;
	UFUNCTION(BlueprintCallable)
		bool Shoot(const FVector& StartLocation, const FVector& EndLocation);
	UFUNCTION(BlueprintCallable)
		bool WeaponHasOwner() const;
};
