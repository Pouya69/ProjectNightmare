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
	NORMAL_CLIMB
};

UENUM(Blueprintable)
enum class EWallWalkType : uint8 {
	RIGHT_WALL,
	LEFT_WALL,
	NO_WALL
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
	virtual void Die() override;

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
	UFUNCTION(BlueprintCallable)
		void DisableCameraLagUntilNextFrame();


private:
	FTimerHandle TimerHandle;
	FTimerHandle EvadeTimerHandle;
	FTimerHandle EpicEffectTimerHandle;
	FTimerHandle DroneCooldownTimerHandle;

public:
	// Ragdolling
	virtual void StartRagdolling() override;
	virtual void StopRagdollingBackToAnimation() override;
	virtual void AddImpulseToCharacter(const FVector& Impulse) override;

public:
	// Weapon Selection
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Selection")
		class UWidgetComponent* WeaponSelectionWidgetComp;
	UPROPERTY(EditAnywhere, Category = "Weapon Selection")
		TSubclassOf<class UWeaponSelectionWidget> WeaponSelectionWidgetClass;
	UPROPERTY(BlueprintReadOnly, Category="Weapon Selection")
		class UWeaponSelectionWidget* WeaponSelectionWidget;
	UFUNCTION(BlueprintCallable, Category = "Weapon Selection")
		bool IsInWeaponSelection() const { return WeaponSelectionWidget != nullptr; }
	UFUNCTION(BlueprintCallable, Category = "Weapon Selection")
		void StartWeaponSelection(const FInputActionInstance& ActionInstance);
	UFUNCTION(BlueprintCallable, Category = "Weapon Selection")
		void WeaponSelectionHeld(const FInputActionInstance& ActionInstance);
	UFUNCTION(BlueprintCallable, Category = "Weapon Selection")
		void EndWeaponSelection(const FInputActionInstance& ActionInstance);
	// For mouse wheel.
	UFUNCTION(BlueprintCallable)
		void NextItem(const FInputActionInstance& ActionInstance);
	// For mouse wheel.
	UFUNCTION(BlueprintCallable)
		int GetWeaponIndex(const class AWeapon* WeaponRef) const;
	UFUNCTION(BlueprintCallable)
		AWeapon* GetWeaponByIndex(const int Index) const;

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
	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
		class UInputAction* NextPreviousItemAction;
	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
		class UInputAction* WeaponSelectAction;
	UPROPERTY(EditAnywhere, Category="EnhancedInput")
		class UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
		class UInputAction* LookAction;
	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
		class UInputAction* JumpAction;
	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
		class UInputAction* EvadeAction;
	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
		class UInputAction* ShieldAction;
	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
		class UInputAction* UltimateAction;
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
		class UInputAction* PistolShootAction;
	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
		class UInputAction* SpecialAction;
	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
		class UInputAction* ReloadAction;
	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
		class UInputAction* DroneDeployAction;
	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
		class UInputAction* LockOnAction;

public:
	// Inventory
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		class UInventoryComponent* InventoryComponent;
public:
	// Movement
	UFUNCTION(BlueprintCallable, Category="Movement")
		void EvadeEnd();
	UFUNCTION(BlueprintCallable, Category = "Movement")
		UAnimMontage* GetEvadeMontageFromDirection() const;
	UPROPERTY(BlueprintReadOnly)
		bool IsEvading = false;
	UPROPERTY(EditAnywhere, Category = "Movement")
		int CollisionEvadesMax = 2;
	int CurrentCollisionEvades = 0;
	FVector2D MovementDirection;
	UPROPERTY(EditAnywhere, Category = "Movement")
		float WalkSpeed;
	UPROPERTY(EditAnywhere, Category = "Movement")
		float SprintSpeed;
	UPROPERTY(EditAnywhere, Category = "Movement")
		FVector SpringArmOffsetSprint;
	UPROPERTY(EditAnywhere, Category = "Movement")
		float SpeedChangeRate = 80.f;
	float TargetMovementSpeed;
	UPROPERTY(EditAnywhere, Category = "Movement")
		float MouseSensivity = 0.5f;
	UPROPERTY(EditAnywhere, Category = "Movement")
		float SprintSpringArmLength = 300.f;
	UFUNCTION()
		void Look(const FInputActionInstance& ActionInstance);
	UFUNCTION()
		void Move(const FInputActionInstance& ActionInstance);
	UFUNCTION()
		void Evade(const FInputActionInstance& ActionInstance);
	UFUNCTION()
		void OnCollisionWithObject(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);
	UFUNCTION()
		void Ultimate(const FInputActionInstance& ActionInstance);
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bCanEvade = true;
	UFUNCTION()
		void Sprint(const FInputActionInstance& ActionInstance);
	UPROPERTY(BlueprintReadWrite)
		bool IsJumping = false;
	virtual void Jump() override;
	UFUNCTION(BlueprintCallable)
		void Jump_Jump();
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
	// Wall Walking
	UPROPERTY(EditAnywhere, Category = "Wall Walking")
		FVector SpringArmOffsetWallWalking;
	UPROPERTY(EditAnywhere, Category = "Wall Walking")
		float SpringArmLengthWallWalking;
	FVector WallNormal;
	UFUNCTION(BlueprintCallable, Category = "Wall Walking")
		FVector GetMovementDirectionOnWallWalking(const FVector InWallNormal) const;
	UPROPERTY(EditAnywhere, Category = "Wall Walking")
		UAnimMontage* WallWalk_JumpFromRightWallMontage;
	UPROPERTY(EditAnywhere, Category = "Wall Walking")
		UAnimMontage* WallWalk_JumpFromLeftWallMontage;
	UPROPERTY(EditAnywhere, Category = "Wall Walking")
		float WallWalkingGravity = 0.3f;
	UPROPERTY(EditAnywhere, Category = "Wall Walking")
		float WallWalkingSpeed = 500.f;
	// For making sure the player sticks to the wall.
	UPROPERTY(EditAnywhere, Category = "Wall Walking")
		float WallWalkingStickToWallForce = 100.f;
	void HandleWallWalk();
	UFUNCTION(BlueprintCallable, Category="Wall Walking")
		EWallWalkType IsNextToWalkableWall(FHitResult& OutHitResult) const;
	UPROPERTY(BlueprintReadOnly, Category = "Wall Walking")
		EWallWalkType CurrentWallWalkSide = EWallWalkType::NO_WALL;
	UFUNCTION(BlueprintCallable, Category = "Wall Walking")
		void StartWalkingOnWall(class UCharacterMovementComponent* CharMovement);
	UFUNCTION(BlueprintCallable, Category = "Wall Walking")
		void StopWalkingOnWall(class UCharacterMovementComponent* CharMovement, const float NewGravityScale = 1.75f);
	UFUNCTION()
		void StopWalkingOnWall_Event(EWallWalkType WallSide);
	UFUNCTION(BlueprintCallable, Category = "Wall Walking")
		void WallWalk_JumpEvent(const bool bIsRight);
	UFUNCTION(BlueprintCallable, Category = "Wall Walking")
		void JumpToOtherSideFromWallWalking(class UCharacterMovementComponent* CharMovement);
	UFUNCTION(BlueprintCallable, Category = "Wall Walking")
		bool IsWalkingOnWall() const { return CurrentWallWalkSide != EWallWalkType::NO_WALL; }

public:
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
		int CurrentShootingWeaponID = 0;
	int ShotsDoneGravity = 0;
	UFUNCTION(BlueprintCallable, Category = "Combat")
		void EnableStopGravityShootingInAir();
	UPROPERTY(EditAnywhere, Category="Combat")
		FVector SpringArmOffsetAim;

	UPROPERTY(EditAnywhere, Category = "Combat")
		float SpringArmOffsetChangeRate = 20.f;
	FVector SpringArmOffsetInit;
	FVector TargetSpringArmOffset;
	FTimerHandle StopAimingTimerHandle;
	UFUNCTION(BlueprintCallable,Category = "Combat")
		void StopAimimg(float WaitTimeInSeconds = -1.f);
	// Combat
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat")
		bool bIsAimingWeapon;
	UFUNCTION(Category = "Combat")
		void AimWeapon(const FInputActionInstance& ActionInstance);
	UFUNCTION(BlueprintCallable, Category = "Combat")
		bool HasWeaponOnBody() const { return CurrentWeapon != nullptr; }
	UFUNCTION(Category = "Combat")
		void StartShootWeapon(const FInputActionInstance& ActionInstance);
	UFUNCTION(Category = "Combat")
		void ShootWeapon(const FInputActionInstance& ActionInstance);
	UFUNCTION(Category = "Combat")
		void StopShootWeapon(const FInputActionInstance& ActionInstance);
	UFUNCTION(Category = "Combat")
		void StartShootPistol(const FInputActionInstance& ActionInstance);
	UFUNCTION(Category = "Combat")
		void ShootPistol(const FInputActionInstance& ActionInstance);
	UFUNCTION(Category = "Combat")
		void StopShootPistol(const FInputActionInstance& ActionInstance);
	UFUNCTION(Category = "Combat")
		void Reload(const FInputActionInstance& ActionInstance);
	UFUNCTION(BlueprintCallable, Category = "Combat")
		void ReloadComplete_Event();
	UPROPERTY(BlueprintReadOnly, Category="Combat")
		class AWeapon* CurrentWeapon;
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
		class AWeapon* PistolWeapon;
	UFUNCTION(BlueprintCallable, Category = "Combat")
		void PistolShootEvent(float CustomDamage = 10.f);
	UFUNCTION(BlueprintCallable, Category = "Combat")
		void WeaponShootEvent(float CustomDamage = -1.f);
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
	UFUNCTION(BlueprintCallable)
		void SetCanMove(bool bCanMove);

	bool bCanMove = true;

	virtual void ApplyEpicEffect(float TimeDilationAmount, FVector Location, float Duration, bool bIsAttached = false, bool bPlayNiagara = true, bool bSlowDownPlayer = true, float PlayerSlowdownCustomRate = 1.f) override;
	
	UPROPERTY(BlueprintReadOnly, Category="Time Dilation")
		float TargetWorldTimeDilation = 1.f;
	UPROPERTY(BlueprintReadOnly, Category = "Time Dilation")
		float TargetPlayerTimeDilation = 1.f;
	UPROPERTY(BlueprintReadOnly, Category = "Time Dilation")
		float TimeDilationInterpSpeed = -1.f;
	UPROPERTY(BlueprintReadOnly, Category = "Time Dilation")
		float TimeDilationRecoverySpeed = -20.f;
	UPROPERTY(BlueprintReadOnly, Category = "Time Dilation")
		bool bIsInterpolatingTimeDilation = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Dilation")
		float TimeDilationAcceleration = 0.1f;
	UFUNCTION(BlueprintCallable, Category = "Time Dilation")
		void ApplyEpicEffectWithInterpolation(float TimeDilationAmount, float Duration, float RecoverySpeed, FVector Location=FVector::ZeroVector, class UNiagaraSystem* NiagaraToPlay = nullptr, bool bSlowDownPlayer = true, float PlayerSlowdownCustomRate = -20.f, bool bIsInstant = false);
	void HandleTimeDilationInterp(const float& DeltaTime);
public:
	// Melle / Parry / Shield
	UPROPERTY(BlueprintReadOnly, Category = "Melee")
		class AEnemyBaseCharacter* EnemyCharacterMeleeing;
	UPROPERTY(EditAnywhere, Category = "Melee")
		UAnimMontage* MeleeAnimMontage;
	UFUNCTION()
		void ShieldParryAction(const FInputActionInstance& ActionInstance);
	UFUNCTION(BlueprintCallable, Category="Melee")
		void Melee();
	UPROPERTY(EditAnywhere, Category="Melee")
		float MeleeDamage = 120.f;
	UPROPERTY(BlueprintReadWrite, Category = "Melee")
		bool bCanMelee = true;
	UPROPERTY(EditAnywhere, Category = "Melee | Parry")
		float StunSphereRadius = 50.f;
	UPROPERTY(EditAnywhere, Category = "Melee | Parry")
		float ParryDamage = 20.f;
	UPROPERTY(EditAnywhere, Category = "Melee")
		float EnemyNearDistanceCheck = 150.f;
	UPROPERTY(EditAnywhere, Category = "Melee")
		float EnemyNearRadiusCheck = 250.f;
	// For teleporting the player to enemy.
	UPROPERTY(EditAnywhere, Category = "Melee")
		float MeleeDistanceToEnemy = 100.f;
	UFUNCTION(BlueprintCallable, Category="Melee")
		void TeleportToEnemyCharacter(class AEnemyBaseCharacter* EnemyCharacterToTeleportTo);
	UPROPERTY(EditAnywhere, Category = "Melee | Parry")
		UAnimMontage* ParryAnimMontage;
	UFUNCTION(BlueprintCallable, Category = "Melee | Parry")
		void Parry();
	UFUNCTION(BlueprintCallable, Category = "Melee")
		void MeleeGiveDamageEvent();
	UFUNCTION(BlueprintCallable, Category = "Melee")
		void MeleeFinished();
	UFUNCTION(BlueprintCallable, Category = "Melee | Parry")
		void ParryEvent();
	UFUNCTION(BlueprintCallable, Category = "Melee | Parry")
		void ParryFinished();
	UFUNCTION(BlueprintCallable, Category = "Melee")
		class AEnemyBaseCharacter* IsFacingEnemy(FHitResult &OutHitResult, const float DistanceCheck, const float RadiusCheck) const;
	UPROPERTY(BlueprintReadOnly, Category="Shield")
		bool bIsShieldEquipped = false;
	UFUNCTION(BlueprintCallable)
		void EquipShield_Start();
	void Shield_Ongoing();
	void Shield_Completed();
	


public:
	// Combo System
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Player Combo System")
		class UPlayerComboComponent* PlayerComboComponent;
	UPROPERTY(BlueprintReadWrite, Category = "Player Combo System")
		bool bIsExecutingCombo = false;
	UPROPERTY(BlueprintReadWrite, Category = "Player Combo System")
		bool bIsFallingStompingGround = false;
	// TODO: For when landed, we can calculate the distance travelled.
	UPROPERTY(BlueprintReadOnly, Category = "Player Combo System")
		float StompGroundZStart = 0.f;
	UPROPERTY(EditAnywhere, Category = "Player Combo System | Stomp Ground")
		float StompGroundDamageAreaRadius = 150.f;
	UPROPERTY(EditAnywhere, Category = "Player Combo System | Stomp Ground")
		TSubclassOf<UDamageType> StompGroundDamageType;
	UPROPERTY(EditAnywhere, Category = "Player Combo System | Stomp Ground")
		float StompGroundDamage = 150.f;
	UPROPERTY(EditAnywhere, Category = "Player Combo System | Stomp Ground")
		float StompFallGravityMult = 15.f;
	UFUNCTION(BlueprintCallable, Category = "Player Combo System | Stomp Ground")
		void StompGround_Event();
	UFUNCTION(BlueprintCallable, Category = "Player Combo System | Stomp Ground")
		void StompGroundEnded_Event();
	UPROPERTY(BlueprintReadOnly, Category = "Player Combo System | Ultimate")
		bool bIsInUltimate = false;
	UPROPERTY(EditAnywhere, Category = "Player Combo System | Ultimate")
		FVector SpringArmOffsetUltimate;
	UFUNCTION(BlueprintCallable, Category = "Player Combo System | Ultimate")
		void UltimateFinished_Event();
	UFUNCTION(BlueprintCallable, Category = "Player Combo System | Ultimate")
		void UltimateShoot_Event();
	void Ultimate_Ongoing(float& DeltaTime);
	UFUNCTION(BlueprintCallable, Category = "Player Combo System | Ultimate")
		void StartUltimate();
	UFUNCTION(BlueprintCallable, Category = "Player Combo System | Ultimate")
		void EndUltimate();

public:
	// Abilities
	UPROPERTY(BlueprintReadOnly)
		class USpecialAbilityHandlerComponent* SpecialAbilityHandlerComp;
	UFUNCTION(Category = "Combat")
		void SpecialStart(const FInputActionInstance& ActionInstance);
	UFUNCTION(Category = "Combat")
		void SpecialHold(const FInputActionInstance& ActionInstance);
	UFUNCTION(Category = "Combat")
		void SpecialRelease(const FInputActionInstance& ActionInstance);

public:
	// Enemy Lock-On
	UFUNCTION(BlueprintCallable, Category = "Lock On System")
		void LockOn();
	UPROPERTY(EditAnywhere, Category = "Lock On System")
		class UCameraLockOn* CameraLockOnComp;
	UFUNCTION(BlueprintCallable, Category = "Lock On System")
		bool IsLockedOnEnemy() const;
	FVector2D LookForce;
public:
	// Being Grabbed
	// For the setup when being grabbed by the flying mutant
	void GrabbedByFlyingMutant(class AEnemyBaseCharacter* InEnemyCharacterGrabbed);
	class AEnemyBaseCharacter* EnemyCharacterThatGrabbedMe;
	void ReleasedFromGrab();
	FORCEINLINE bool IsGrabbedByFlyingEnemy() const { return EnemyCharacterThatGrabbedMe != nullptr; }

public:
	FTimerHandle FourthDimensionFreezeTimerHandle;
	class ANightmareGameMode* NightmareGameMode;
	UPROPERTY(EditAnywhere, Category = "4TH Dimension")
		float UnfreezeEverythingAfterSeconds = 2.f;
	UPROPERTY(BlueprintReadWrite, Category = "4TH Dimension")
		bool bIsInFourthDimension = false;
	// 4th Dimension
	UFUNCTION(BlueprintCallable, Category = "4TH Dimension")
		void FourthDimensionStartOpening(const FInputActionInstance& ActionInstance);
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "4TH Dimension")
		void OpenFourthDimension(FTransform PortalTransform);
	void OpenFourthDimension_Implementation(FTransform PortalTransform);
	struct FPostProcessSettings InitialCameraPostEffects;
	void StartResettingCameraPostEffects();
	void ResetCameraPostEffects();
	UFUNCTION(BlueprintCallable)
		void PlayCameraShake(TSubclassOf<UCameraShakeBase> CameraShakeToPlay, bool bStopAllShakes = false);
	FTransform LastOverworldTransform;

	UPROPERTY(EditAnywhere, Category = "4TH Dimension")
		TSubclassOf<class AFourthDimension_Portal> PortalTo4THDimensionClass;
	UPROPERTY(BlueprintReadWrite, Category = "4TH Dimension")
		class AFourthDimension_Portal* SpawnedPortalToFourthDimension;
	
	UFUNCTION(BlueprintCallable, Category = "4TH Dimension")
		void TeleportToFourthDimension();
	UFUNCTION(BlueprintCallable, Category = "4TH Dimension")
		void ComeBackToOverworld();
	UFUNCTION(BlueprintCallable, Category = "4TH Dimension")
		void OverworldComebackCompleted();

public:
	// Interaction
	UPROPERTY(EditAnywhere)
		class UBoxComponent* InteractOverlapComp;
	UFUNCTION()
		void Interact(const FInputActionInstance& ActionInstance);
	UFUNCTION(BlueprintCallable)
		void PickupWeapon(AWeapon* WeaponToPickup, bool bEquip = false);
	UFUNCTION(BlueprintCallable)
		void DropWeapon(AWeapon* WeaponToDrop);
	UPROPERTY(EditAnywhere)
		FName WeaponAttachmentSocketName;
	UPROPERTY(EditAnywhere)
		FName WeaponHandAttachmentSocketName;
	UPROPERTY(EditAnywhere)
		FName PistolAttachmentSocketName;
	UPROPERTY(EditAnywhere)
		float InteractDistance = 100.f;

	void StopInteract();
	UFUNCTION()
		void InteractionRangeOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndexbool, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
		void InteractionRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndexbool);
	class AInteractableObject* GetClosestInteractable() const;
	bool IsInteractableBlocked(class AInteractableObject* Interactable) const;

	UFUNCTION(BlueprintCallable)
		bool IsInteracting() const;
	class AInteractableObject* InteractingObject;
	void HandleInteract(float DeltaTime);
	UFUNCTION(BlueprintCallable)
		void RefillAmmo(class AWeapon* WeaponToFill = nullptr, int Amount = -1);

public:
	UPROPERTY(EditAnywhere, Category = "Animation Montages")
		UAnimMontage* LedgeClimbAnimMontage;
	UPROPERTY(EditAnywhere, Category = "Animation Montages")
		UAnimMontage* NormalClimbAnimMontage;
	UPROPERTY(EditAnywhere, Category = "Animation Montages")
		UAnimMontage* EquipPistolAnimMontage;
	UPROPERTY(EditAnywhere, Category = "Animation Montages")
		UAnimMontage* UnequipPistolAnimMontage;
	UPROPERTY(EditAnywhere, Category = "Animation Montages")
		UAnimMontage* Evade_Fw_AnimMontage;
	UPROPERTY(EditAnywhere, Category = "Animation Montages")
		UAnimMontage* Air_Evade_Fw_AnimMontage;
	UPROPERTY(EditAnywhere, Category = "Animation Montages")
		UAnimMontage* Evade_B_AnimMontage;
	UPROPERTY(EditAnywhere, Category = "Animation Montages")
		UAnimMontage* Evade_R_AnimMontage;
	UPROPERTY(EditAnywhere, Category = "Animation Montages")
		UAnimMontage* Evade_L_AnimMontage;

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
