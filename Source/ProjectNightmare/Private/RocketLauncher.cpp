// Fill out your copyright notice in the Description page of Project Settings.


#include "RocketLauncher.h"

bool ARocketLauncher::Shoot(const FVector& StartLocation, const FVector& EndLocation, bool bForceShoot, bool bEventShot, float CustomDamage)
{
	BulletsToAddAfterReloadComplete = 0;
	if (!bEventShot) {
		if (!bForceShoot && (bIsReloadingWeapon || CurrentFireRatePoint < 99.f)) return false;
		if (CurrentBulletsLeft <= 0) {
			// TODO: Make sound of empty gun
			return false;
		}
		CurrentBulletsLeft -= RocketsToShoot;
	}
	CurrentFireRatePoint = 0;
	// Will launch projectiles and those are the ones dealing damage.
	return true;
}

void ARocketLauncher::RocketShot()
{
}
