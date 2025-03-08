// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/SkeletalMeshActor.h"
#include "Weapon.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTNIGHTMARE_API AWeapon : public ASkeletalMeshActor
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FName WeaponName;
public:
	// Animation
	UPROPERTY(EditAnywhere)
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
		class UAimOffsetBlendSpace* HIP_AimOffset;
	UPROPERTY(EditAnywhere)
		class UAimOffsetBlendSpace* AIM_AimOffset;

public:
	UPROPERTY(EditAnywhere)
		bool bShouldAimToShoot;
	// Combat
	UPROPERTY(EditAnywhere)
		float BaseDamage;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
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
		bool Shoot(FVector& EndLocation);
	UFUNCTION(BlueprintCallable)
		bool WeaponHasOwner() const;
};
