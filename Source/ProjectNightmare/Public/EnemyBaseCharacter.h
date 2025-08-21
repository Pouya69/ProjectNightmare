// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "EnemyBaseCharacter.generated.h"

/**
 * 
 */


UENUM(Blueprintable)
enum class EEnemyComboType : uint8
{
	NORMAL_ATTACK_1 = 0,
};


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

	UPROPERTY(EditAnywhere, Category = "Spawn")
		bool bSpawned = false;
	UPROPERTY(EditAnywhere, Category="Spawn")
		UAnimMontage* JustSpawnedAnim;
	UFUNCTION(BlueprintCallable, Category = "Spawn")
		void EnemySpawned();
	UFUNCTION(BlueprintCallable, Category = "Spawn")
		void SpawnFinished();

	UPROPERTY(BlueprintReadOnly)
		class AEnemyBaseAIController* EnemyAIController;

	UFUNCTION(BlueprintCallable, Category = "Attack")
		virtual bool AttackPlayer();

	UFUNCTION(BlueprintCallable, Category = "Stun")
		void StunStart(const float StunTimer = 2.f, const bool bShouldStopAnims = false);
	UFUNCTION(BlueprintCallable, Category = "Stun")
		void StopStun();
	UPROPERTY(EditAnywhere, Category = "Stun")
		float ParryStunTimerInSeconds = 2.f;
	FTimerHandle StunTimerHandle;
	// Used for parrying.
	UPROPERTY(BlueprintReadWrite, Category="Attack")
		bool bIsTryingToDamagePlayer = false;
	UPROPERTY(EditAnywhere, Category = "Attack")
		UAnimMontage* PlayerMeleeResponseAnimMontage;
	UFUNCTION(BlueprintCallable, Category = "Attack")
		virtual void PlayerParryAllow(const bool bIsAllowed);
	UFUNCTION(BlueprintCallable, Category = "Attack")
		bool CanPlayerParryMyAttack() const;
	UFUNCTION(BlueprintCallable, Category = "Attack")
		void PlayerStartMeleeMe();
	UFUNCTION(BlueprintCallable, Category = "Attack")
		void PlayerStartParryMe();
	UFUNCTION(BlueprintCallable, Category = "Attack")
		void PlayerMeleeEvent(const float Damage = 150.f);
	UFUNCTION(BlueprintCallable, Category = "Attack")
		void PlayerParryEvent(const float Damage = 20.f);
	UFUNCTION(BlueprintCallable, Category = "Attack")
		void PlayerMeleeFinished();
	UFUNCTION(BlueprintCallable, Category = "Attack")
		void PlayerParryFinished();


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
	// Ragdoll
	// This is for not playing it always when we are hit.
	FTimerHandle ReactHitDelayTimerHandle;
	virtual void AfterSnapShot() override;
	virtual void StopRagdollingBackToAnimation_FROM_TIMER() override;
	virtual void UpdateRagdollState() override;
	virtual void StartRagdolling() override;
	virtual void StandingEventDone() override;
	UPROPERTY(EditAnywhere, Category="Ragdoll")
		float DamageThresholdRagdoll = 40.f;
	// We will not play the react hit animations after X seconds of already have played it.
	UPROPERTY(EditAnywhere, Category = "Ragdoll")
		float ReactHitDelayInSeconds = 3.f;
		
public:
	// Anim Montages
	UPROPERTY(EditAnywhere, Category="Animations")
		UAnimMontage* AttackMontage;

public:
	// Combos
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combos")
		EEnemyComboType CurrentComboType = EEnemyComboType::NORMAL_ATTACK_1;
	UPROPERTY(BlueprintReadWrite, Category = "Combos")
		int CurrentComboIndex = 0;
	UFUNCTION(BlueprintCallable, Category = "Combos")
		virtual void ChangeAttackComboTypeTo(EEnemyComboType NewComboType);
	UFUNCTION(BlueprintCallable, Category = "Combos")
		virtual void ResetCombo(bool bIsDash = false);
	UPROPERTY(BlueprintReadWrite, Category = "Combos")
		uint8 CurrentDashIndex = 0U;
	UFUNCTION(BlueprintCallable, Category = "Combos")
		virtual void Dash(UAnimMontage* DashAnimMontage, FVector TargetLocation, FName OverrideMotionWarp, bool bBackwards = true);
};
