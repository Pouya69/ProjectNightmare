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
	UFUNCTION(BlueprintCallable)
		FVector GetCharacterVelocity() const;
	UFUNCTION(BlueprintCallable)
		void SetCharacterVelocity(const FVector NewVelocity) const;

	bool bIsMarkedForDeath;

public:
	// Ragdoll
	bool bIsMarkedForGettingUp;
	FTimerHandle RagdollStopTimer;
	virtual void UpdateRagdollState();
	FVector PelvisOffset;
	void InitPhysicsSetup();
	UPROPERTY(EditAnywhere, Category = "Ragdoll")
		float StopRagdollingAfterVelocity = 10.f;
	UPROPERTY(EditAnywhere, Category = "Ragdoll")
		float StopRagdollingAfterSeconds = 2.f;
	UPROPERTY(EditAnywhere, Category="Ragdoll")
		bool bShouldRagdollInGravityAreas = true;
	UPROPERTY(BlueprintReadOnly, Category = "Ragdoll")
		bool bIsRagdolling;
	UPROPERTY(BlueprintReadOnly, Category = "Ragdoll")
		bool bShouldGetUpFromFront_ANIMATION_ONLY;

	UFUNCTION(BlueprintCallable, Category = "Ragdoll")
		virtual void StartRagdolling();
	FVector GetPelvisLocation() const;
	UFUNCTION(BlueprintCallable, Category = "Ragdoll")
		virtual void StopRagdollingBackToAnimation();
	UFUNCTION(BlueprintCallable, Category = "Ragdoll")
		virtual void StopRagdollingBackToAnimation_FROM_TIMER();
	UFUNCTION(BlueprintCallable, Category = "Ragdoll")
		virtual void StandingEventDone();
	UFUNCTION(BlueprintCallable, Category = "Ragdoll")
		bool ShouldGetUpFromFront() const;
	UFUNCTION()
		virtual void AfterSnapShot();
	UFUNCTION()
		void TakeSnapShot();

	virtual void AddImpulseToCharacter(const FVector& Impulse);

public:
	// Death and falling.
	UPROPERTY(EditAnywhere, Category = "Death")
		UAnimMontage* DeathMontage_Front;
	UPROPERTY(EditAnywhere, Category = "Death")
		UAnimMontage* DeathMontage_Back;
	UPROPERTY(EditAnywhere, Category = "Death")
		UAnimMontage* FallMontage_Front;
	UPROPERTY(EditAnywhere, Category = "Death")
		UAnimMontage* FallMontage_Back;
	UPROPERTY(EditAnywhere, Category = "Death")
		UAnimMontage* ReactHitMontage_Front;
	UPROPERTY(EditAnywhere, Category = "Death")
		UAnimMontage* ReactHitMontage_Back;
	UPROPERTY(EditAnywhere, Category = "Death")
		UAnimMontage* ReactHitMontage_Left;
	UPROPERTY(EditAnywhere, Category = "Death")
		UAnimMontage* ReactHitMontage_Right;
	// If we are hit, only play front or else just play the additive.
	UPROPERTY(EditAnywhere, Category = "Death")
		bool bShouldOnlyPlayFrontHit = false;

	UPROPERTY(EditAnywhere, Category = "Death | Additive")
		UAnimMontage* ReactHitMontageAdditive_Front;
	UPROPERTY(EditAnywhere, Category = "Death | Additive")
		UAnimMontage* ReactHitMontageAdditive_Back;
	UPROPERTY(EditAnywhere, Category = "Death | Additive")
		UAnimMontage* ReactHitMontageAdditive_Left;
	UPROPERTY(EditAnywhere, Category = "Death | Additive")
		UAnimMontage* ReactHitMontageAdditive_Right;
	// Plays the hit animation based on angle of direction hit.
	UFUNCTION(BlueprintCallable, Category="Death | Additive")
		void PlayHitReactionMontage(const float SignedAngle);

public:
	// Dismemberment

	class ULimbDismemberment* DismembermentComp;

	UFUNCTION(BlueprintCallable)
		void ApplyDismembermentToLimb(const FName& BoneName, FVector Impulse, FVector HitLocation, bool bForced = false);

	UFUNCTION()
		void OnDestroyedDeath(AActor* DestroyedActor);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dismemberment")
		bool bIsCrawling;
	UPROPERTY(EditAnywhere, Category = "Dismemberment")
		float StartCrawlAfterRagdollInSeconds = 1.f;
	UPROPERTY(EditAnywhere, Category = "Dismemberment")
		float CrawlingMovementSpeed = 60.f;
	UFUNCTION()
		virtual void StartCrawling();

	float GetMovementSpeed(EMovementMode InMovementMode) const;

	FTimerHandle CrawlingTimer;

public:
	// Epic Effect
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		class UNiagaraSystem* SlowMotionNiagaraEffect;
	UFUNCTION(BlueprintCallable, Category = "Combat")
		virtual void ApplyEpicEffect(float TimeDilationAmount, FVector Location, float Duration, bool bIsAttached = false, bool bPlayNiagara = true, bool bSlowDownPlayer = true, float PlayerSlowdownCustomRate = 1.f);
	FTimerHandle EpicEffectTimerHandle;
	virtual void StopMyMovement();
public:
	// Texture Paint
	UPROPERTY(EditAnywhere, Category = "Texture Paint")
		float BloodSplatterOnBulletHit = 25.f;
	UPROPERTY(EditAnywhere, Category="Texture Paint")
		class USceneCaptureComponent2D* SceneCaptureComp;

	UPROPERTY(EditAnywhere, Category = "Texture Paint")
		class UMaterialInterface* UnwrapMaterial;

	void PaintBlood(const FVector& ImpactPoint, const float Radius);

	class UTextureRenderTarget2D* Damage_RT;
	class UMaterialInterface* OldMaterial;
	// class UMaterialInstanceDynamic* OldMaterial;

private:
	float Health;
};
