// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CharacterBase.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);
UCLASS()
class PROJECTNIGHTMARE_API ACharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACharacterBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
public:
	UPROPERTY(EditAnywhere, Category="Health")
		float MaxHealth = 100.f;
	UFUNCTION(BLueprintCallable)
		float GetHealth() const { return Health; }
	UFUNCTION(BlueprintCallable)
		void AddHealth(float Amount);
	UFUNCTION(BLueprintCallable)
		float GetCharacterMass() const;
	UFUNCTION(BlueprintCallable)
		void Die();
	UFUNCTION(BlueprintCallable)
		void ReduceHealth(float Amount);
	UFUNCTION(BlueprintCallable)
		bool IsAlive() const { return Health > 0; }
	UPROPERTY(BlueprintAssignable, Category="Health")
		FOnDeath OnDeath;
	UFUNCTION(BlueprintCallable)
		void HitByWeapon(FVector HitLocation, FVector HitNormal, float WeaponBaseDamage);

	UFUNCTION(BlueprintCallable)
		float GetCapsuleRadius() const;
	UFUNCTION(BlueprintCallable)
		float GetCapsuleHalfHeight() const;

private:
	float Health;
};
