// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "WeaponAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTNIGHTMARE_API UWeaponAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UWeaponAnimInstance();

public:
	virtual void NativeUpdateAnimation(float DeltaTime) override;

	void Shoot(UAnimMontage* ShootMontage);
	void Reload(UAnimMontage* ReloadMontage);

	UPROPERTY(BlueprintReadWrite)
		class AWeapon* OwnerWeapon;
protected:
	virtual void NativeBeginPlay() override;
	
};
