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
	UFUNCTION()
		virtual void TakePointDamage(AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation, class UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser);
	UFUNCTION()
		virtual void TakeRadialDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, FVector Origin, const FHitResult& HitInfo, class AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION()
		virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;

	UFUNCTION()
		void TakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

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
		virtual void Die();
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

public:
	// Dismemberment

	class ULimbDismemberment* DismembermentComp;

	UFUNCTION(BlueprintCallable)
		void ApplyDismembermentToLimb(const FName& BoneName, FVector Impulse, FVector HitLocation);

private:
	float Health;
};
