// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "EnemyBaseCharacter.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTNIGHTMARE_API AEnemyBaseCharacter : public ACharacterBase
{
	GENERATED_BODY()
	
public:
	AEnemyBaseCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadOnly)
		class AEnemyBaseAIController* EnemyAIController;

	UFUNCTION(BlueprintCallable)
		virtual void AttackPlayer();

	virtual void Die() override;
	virtual void TakePointDamage(AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation, class UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser) override;
	virtual void TakeRadialDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, FVector Origin, const FHitResult& HitInfo, class AController* InstigatedBy, AActor* DamageCauser) override;
	virtual void StopMyMovement() override;
	UFUNCTION(BlueprintCallable)
		void AllowAIMovement();

	FTimerHandle CrawlingTimer;

	UPROPERTY(EditAnywhere, Category = "Death")
		float DisappearAfterDeathInSeconds = 20.f;

	FTimerHandle ShouldRunAITimer;

public:
	// Anim Montages
	UPROPERTY(EditAnywhere, Category="Animations")
		UAnimMontage* AttackMontage;
};
