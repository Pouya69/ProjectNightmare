// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "RocketLauncher.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTNIGHTMARE_API ARocketLauncher : public AWeapon
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		int RocketsToShoot = 2
;
	//UPROPERTY(EditAnywhere, BlueprintReadOnly)
		//TSubclassOf<>
	virtual bool Shoot(const FVector& StartLocation, const FVector& EndLocation, bool bForceShoot = false, bool bEventShot = false, float CustomDamage = -1.f) override;
	void RocketShot();
};
