// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ACharacterBase::ACharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();
	Health = MaxHealth;
}

// Called every frame
void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACharacterBase::AddHealth(float Amount)
{
	Health = FMath::Clamp(Health+Amount, 0, MaxHealth);
}

float ACharacterBase::GetCharacterMass() const
{
	return GetCharacterMovement()->Mass;
}

void ACharacterBase::Die()
{
	OnDeath.Broadcast();
	
}

void ACharacterBase::ReduceHealth(float Amount)
{
	Health -= Amount;
	if (Health <= 0) Die();
}

void ACharacterBase::HitByWeapon(FVector HitLocation, FVector HitNormal, float WeaponBaseDamage)
{
	// TODO: Calculate Damage Deault based on hitlocation
	// FHitResult HitResult;
	// const bool bIsHitHead = GetWorld()->SweepSingleByChannel(HitResult, HitLocation, HitLocation, FQuat::Identity, ECollisionChannel)
	ReduceHealth(WeaponBaseDamage);
}

float ACharacterBase::GetCapsuleRadius() const
{
	return GetCapsuleComponent()->GetScaledCapsuleRadius();
}

float ACharacterBase::GetCapsuleHalfHeight() const
{
	return GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}

