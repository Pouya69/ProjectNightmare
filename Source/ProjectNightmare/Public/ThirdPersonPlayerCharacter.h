// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "ThirdPersonPlayerCharacter.generated.h"

/**
 * 
 */
UENUM(Blueprintable)
enum class EClimbType : uint8 {
	NOT_CLIMBABLE,
	LEDGE_CLIMB,
	TOO_TALL_NOT_CLIMBABLE,
	NORMAL_CLIMB,
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFinishedCutscene, bool, bShouldDie);
UCLASS()
class PROJECTNIGHTMARE_API AThirdPersonPlayerCharacter : public ACharacterBase
{
	GENERATED_BODY()
	
public:
	AThirdPersonPlayerCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaTime) override;

public:
	// Delegates
	UPROPERTY(BlueprintAssignable, Category="Events")
		FOnFinishedCutscene OnFinishedCutscene;

public:
	UPROPERTY(BlueprintReadOnly)
		APlayerCameraManager* PlayerCameraManager;
	class USpringArmComponent* SpringArmComp;
	class UCameraComponent* CameraComp;
	class UWarpCheckerComponent* WarpHandlerComponent;
	UPROPERTY(BlueprintReadOnly)
		APlayerController* MyPlayerController;


private:
	FTimerHandle TimerHandle;
	FTimerHandle EvadeTimerHandle;
	FTimerHandle EpicEffectTimerHandle;
	FTimerHandle DroneCooldownTimerHandle;

public:
	// Drone
	UPROPERTY(EditAnywhere, Category = "Drone")
		int DroneRocketsLeft = 4;
	UPROPERTY(EditAnywhere, Category = "Drone")
		int DroneCooldownInSeconds = 20;
	UPROPERTY(EditAnywhere, Category = "Drone")
		float DroneSpawnDistanceFromPlayer = 80.f;
	bool bCanDeployDrone = true;
	class ADronePawn* DroneInAir;

	UFUNCTION(BlueprintCallable)
		void DeployDrone(const FInputActionInstance& ActionInstance);
	void DroneDestroyed();

	UPROPERTY(EditAnywhere, Category="Drone")
		TSubclassOf<ADronePawn> DronePawnClass;

	UFUNCTION(BlueprintCallable, Category = "Drone")
		bool IsDroneInAir() const { return DroneInAir != nullptr; }

public:
	// Aiming
	float SpringArmLengthInital;
	float SpringArmLengthTarget;
	UPROPERTY(EditAnywhere, Category = "Aiming")
		float SpringArmLengthTransitionRate = 0.2f;
	UPROPERTY(EditAnywhere, Category = "Aiming")
		float SpringArmLengthAimingMult = 0.7f;

	UFUNCTION(BlueprintCallable)
		FVector GetCameraLookingDirection() const;
public:
	UFUNCTION(BlueprintCallable)
		bool IsPlayerInputEnabled() const;
	UFUNCTION(BlueprintCallable)
		void ChangePlayerCollision(bool bNewCollisionState);
	UFUNCTION(BlueprintCallable)
		void ChangePlayerMovement(EMovementMode NewMovementMode);
	// EnhancedInput
	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
		class UInputMappingContext* ThirdPersonMappingContext;
	UPROPERTY(EditAnywhere, Category="EnhancedInput")
		class UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
		class UInputAction* LookAction;
	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
		class UInputAction* JumpAction;
	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
		class UInputAction* EvadeAction;
	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
		class UInputAction* MeleeAction;
	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
		class UInputAction* GrenadeAction;
	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
		class UInputAction* InteractAction;
	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
		class UInputAction* SprintAction;
	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
		class UInputAction* AimAction;
	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
		class UInputAction* ShootAction;
	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
		class UInputAction* ClickerAction;
	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
		class UInputAction* ReloadAction;
	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
		class UInputAction* DroneDeployAction;

public:
	// Movement
	FVector2D MovementDirection;
	UPROPERTY(EditAnywhere, Category = "Movement")
		float WalkSpeed;
	UPROPERTY(EditAnywhere, Category = "Movement")
		float SprintSpeed;
	UPROPERTY(EditAnywhere, Category = "Movement")
		float MouseSensivity = 0.5f;
	UFUNCTION()
		void Look(const FInputActionInstance& ActionInstance);
	UFUNCTION()
		void Move(const FInputActionInstance& ActionInstance);
	UFUNCTION()
		void Evade(const FInputActionInstance& ActionInstance);
	UFUNCTION()
		void Melee(const FInputActionInstance& ActionInstance);
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bCanEvade = true;
	UFUNCTION()
		void Sprint(const FInputActionInstance& ActionInstance);
	virtual void Jump() override;
	UPROPERTY(EditAnywhere, Category="Movement")
		float ClimbableRangeCheck = 100.f;
	UFUNCTION(BlueprintCallable)
		float GetCurrentMovementSpeed() const;
	UPROPERTY(EditAnywhere, Category = "Movement")
		TSubclassOf<UCameraShakeBase> WalkingCameraShake;
	UFUNCTION(BlueprintCallable)
		void ResetMovementVelocity(bool bShouldResetZ = false);

	UFUNCTION(BlueprintCallable)
		EClimbType IsFacingClimbableObject(FHitResult& OutHitResult, bool bSameObjectIgnore=true) const;
	UFUNCTION(BlueprintCallable)
		void Roll();

public:
	class UCharacterGrenadeHandler* CharacterGrenadeHandlerComp;
	// Grenade
	UFUNCTION(Category = "Grenade")
		void AimGrenade(const FInputActionInstance& ActionInstance);
	UFUNCTION(Category = "Grenade")
		void ThrowGrenade(const FInputActionInstance& ActionInstance);
	UFUNCTION(Category = "Grenade")
		void StartGrenade(const FInputActionInstance& ActionInstance);

	UPROPERTY(BlueprintReadOnly)
		bool bIsAimingGrenade;
	UPROPERTY(EditAnywhere, Category="Grenade")
		UAnimMontage* GrenadeThrowMontage;
	UPROPERTY(EditAnywhere, Category = "Grenade")
		UAnimMontage* GrenadeAimThrowMontage;
	UFUNCTION(BlueprintCallable, Category="Grenade")
		void FinishThrowGrenade(bool bWasAiming);

public:
	UFUNCTION(BlueprintCallable,Category = "Combat")
		void StopAimimg();
	// Combat
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat")
		bool bIsAimingWeapon;
	UFUNCTION(Category = "Combat")
		void AimWeapon(const FInputActionInstance& ActionInstance);
	UFUNCTION(BlueprintCallable, Category = "Combat")
		bool HasWeaponOnBody() const { return CurrentWeapon != nullptr; }
	UFUNCTION(Category = "Combat")
		void ShootWeapon(const FInputActionInstance& ActionInstance);
	UFUNCTION(Category = "Combat")
		void ClickerClick(const FInputActionInstance& ActionInstance);
	UFUNCTION(Category = "Combat")
		void Reload(const FInputActionInstance& ActionInstance);
	UPROPERTY(EditAnywhere, Category="Combat")
		TSubclassOf<UCameraShakeBase> ShootingCameraShake;
	UPROPERTY(EditAnywhere, Category="Combat")
		class AWeapon* CurrentWeapon;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		bool bHasWeaponEquipped;
	UFUNCTION(BlueprintCallable, Category = "Combat")
		void EquipWeapon();
	UFUNCTION(BlueprintCallable, Category = "Combat")
		void EquipAttachWeaponToHand(bool bEquip);
	UFUNCTION(BlueprintCallable, Category = "Combat")
		void UnequipWeapon();
	UFUNCTION(BlueprintCallable, Category = "Combat")
		void FinishEquipWeapon();
	UFUNCTION(BlueprintCallable, Category = "Combat")
		void FinishUnequipWeapon();
	UFUNCTION(BlueprintCallable, Category = "Combat")
		UAnimMontage* GetReloadMontageBasedOnWeapon(bool bIsAiming = false) const;
	UFUNCTION(BlueprintCallable, Category = "Combat")
		UAnimMontage* GetShootMontageBasedOnWeapon() const;
	UFUNCTION(BlueprintCallable, Category = "Combat")
		class UAnimationAsset* GetCurrentAimOffset() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		class UNiagaraSystem* SlowMotionNiagaraEffect;
	UFUNCTION(BlueprintCallable, Category = "Combat")
		void ApplyEpicEffect(float TimeDilationAmount, FVector Location, float Duration, bool bPlayNiagara = true, bool bSlowDownPlayer = true);
		
public:
	// Interaction
	UPROPERTY(EditAnywhere)
		class UBoxComponent* InteractionBoxComp;
	UFUNCTION()
		void Interact(const FInputActionInstance& ActionInstance);
	UFUNCTION(BlueprintCallable)
		void PickupWeapon(AWeapon* WeaponToPickup);
	UPROPERTY(EditAnywhere)
		FName WeaponAttachmentSocketName;
	UPROPERTY(EditAnywhere)
		FName WeaponHandAttachmentSocketName;
	UPROPERTY(EditAnywhere)
		FName PistolAttachmentSocketName;
	

public:
	// Animation Montages
	UPROPERTY(EditAnywhere, Category = "Animation Montages")
		UAnimMontage* LedgeClimbAnimMontage;
	UPROPERTY(EditAnywhere, Category = "Animation Montages")
		UAnimMontage* NormalClimbAnimMontage;
	UPROPERTY(EditAnywhere, Category = "Animation Montages")
		UAnimMontage* EquipPistolAnimMontage;
	UPROPERTY(EditAnywhere, Category = "Animation Montages")
		UAnimMontage* UnequipPistolAnimMontage;
	UPROPERTY(EditAnywhere, Category = "Animation Montages")
		UAnimMontage* Roll_FwAnimMontage;
	UPROPERTY(EditAnywhere, Category = "Animation Montages")
		UAnimMontage* Roll_BwdAnimMontage;
	UPROPERTY(EditAnywhere, Category = "Animation Montages")
		UAnimMontage* Roll_RAnimMontage;
	UPROPERTY(EditAnywhere, Category = "Animation Montages")
		UAnimMontage* Roll_LAnimMontage;
	UPROPERTY(EditAnywhere, Category = "Animation Montages")
		UAnimMontage* Evade_FwAnimMontage;
	UPROPERTY(EditAnywhere, Category = "Animation Montages")
		UAnimMontage* Evade_BwAnimMontage;
	UPROPERTY(EditAnywhere, Category = "Animation Montages")
		UAnimMontage* Evade_RAnimMontage;
	UPROPERTY(EditAnywhere, Category = "Animation Montages")
		UAnimMontage* Evade_LAnimMontage;
	UPROPERTY(EditAnywhere, Category = "Animation Montages")
		UAnimMontage* Melee_AnimMontage;

public:
	// Cutscenes
	class UPlayerCutsceneHandlerComponent* PlayerCutsceneHandler;
	void SetCutsceneController(class ACutsceneController* NewCutsceneController);
	UFUNCTION(BlueprintCallable)
		bool IsInCutscene() const;
	UFUNCTION(BlueprintCallable)
		void FinishedCutscene(FTransform NewPlayerTransform, bool bTeleport, bool bFailedCutscene);
	UFUNCTION(BlueprintCallable)
		void SetInCutsceneTrigger(class ACutsceneTrigger* NewCutsceneTrigger, TSubclassOf<class UUserWidget> WidgetClass = nullptr);
	UFUNCTION(BlueprintCallable)
		bool IsInCutsceneTrigger() const;

public:
	// Widgets
	TArray<class UUserWidget*> WidgetsOnScreen;
	UFUNCTION(BlueprintCallable)
		class UUserWidget* GetCurrentWidgetOnScreen() const;
	UFUNCTION(BlueprintCallable)
		void CreateWidgetAndShow(TSubclassOf<class UUserWidget> WidgetClass);
	UFUNCTION(BlueprintCallable)
		void ClearWidget();
	UFUNCTION(BlueprintCallable)
		bool IsPlayerActionAndActionRequiredEqual(const class UInputAction* Action) const;
};
