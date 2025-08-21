// Fill out your copyright notice in the Description page of Project Settings.


#include "ThirdPersonPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "PlayerCutsceneHandlerComponent.h"
#include "Blueprint/UserWidget.h"
#include "CutsceneTrigger.h"
#include "Weapon.h"
#include "Components/BoxComponent.h"
#include "Animation/AimOffsetBlendSpace.h"
#include "NiagaraFunctionLibrary.h"
#include "WarpCheckerComponent.h"
#include "DronePawn.h"
#include "CharacterGrenadeHandler.h"
#include "SpecialAbilityHandlerComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "InteractableObject.h"
#include "InventoryComponent.h"
#include "NightmareGameMode.h"
#include "Camera/CameraModifier_CameraShake.h"
#include "FourthDimension_Portal.h"
#include "EnemyBaseCharacter.h"
#include "Player Combo System/PlayerComboComponent.h"
#include "CameraLockOn.h"
#include "Widgets/WeaponSelectionWidget.h"
#include "Components/WidgetComponent.h"


AThirdPersonPlayerCharacter::AThirdPersonPlayerCharacter()
{
	InteractOverlapComp = CreateDefaultSubobject<UBoxComponent>(FName("Interaction Box Area"));
	InteractOverlapComp->SetupAttachment(GetRootComponent());
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(FName("Inventory Comp"));
	PlayerComboComponent = CreateDefaultSubobject<UPlayerComboComponent>(FName("Player Combo Comp"));
	CameraLockOnComp = CreateDefaultSubobject<UCameraLockOn>(FName("Camera LockOn Comp"));
	CameraLockOnComp->OwnerPlayerRef = this;
	WeaponSelectionWidgetComp = CreateDefaultSubobject<UWidgetComponent>(FName("Weapon Selection Widget Comp"));
	WeaponSelectionWidgetComp->SetupAttachment(GetMesh(), FName("WeaponSelection"));
}

void AThirdPersonPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(ThirdPersonMappingContext, 0);
		}
	}
	UEnhancedInputComponent* PlayerEnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!PlayerEnhancedInputComponent) return;
	PlayerEnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AThirdPersonPlayerCharacter::Move);
	PlayerEnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AThirdPersonPlayerCharacter::Look);
	PlayerEnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AThirdPersonPlayerCharacter::Jump);
	PlayerEnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &AThirdPersonPlayerCharacter::AimWeapon);

	PlayerEnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Started, this, &AThirdPersonPlayerCharacter::StartShootWeapon);
	PlayerEnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Triggered, this, &AThirdPersonPlayerCharacter::ShootWeapon);
	PlayerEnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Completed, this, &AThirdPersonPlayerCharacter::StopShootWeapon);

	PlayerEnhancedInputComponent->BindAction(PistolShootAction, ETriggerEvent::Started, this, &AThirdPersonPlayerCharacter::StartShootPistol);
	PlayerEnhancedInputComponent->BindAction(PistolShootAction, ETriggerEvent::Triggered, this, &AThirdPersonPlayerCharacter::ShootPistol);
	PlayerEnhancedInputComponent->BindAction(PistolShootAction, ETriggerEvent::Completed, this, &AThirdPersonPlayerCharacter::StopShootPistol);

	PlayerEnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &AThirdPersonPlayerCharacter::Sprint);
	PlayerEnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &AThirdPersonPlayerCharacter::Reload);
	PlayerEnhancedInputComponent->BindAction(EvadeAction, ETriggerEvent::Triggered, this, &AThirdPersonPlayerCharacter::Evade);

	PlayerEnhancedInputComponent->BindAction(WeaponSelectAction, ETriggerEvent::Started, this, &AThirdPersonPlayerCharacter::StartWeaponSelection);
	PlayerEnhancedInputComponent->BindAction(WeaponSelectAction, ETriggerEvent::Triggered, this, &AThirdPersonPlayerCharacter::WeaponSelectionHeld);
	PlayerEnhancedInputComponent->BindAction(WeaponSelectAction, ETriggerEvent::Completed, this, &AThirdPersonPlayerCharacter::EndWeaponSelection);

	PlayerEnhancedInputComponent->BindAction(NextPreviousItemAction, ETriggerEvent::Triggered, this, &AThirdPersonPlayerCharacter::NextItem);

	PlayerEnhancedInputComponent->BindAction(UltimateAction, ETriggerEvent::Started, this, &AThirdPersonPlayerCharacter::Ultimate);
	PlayerEnhancedInputComponent->BindAction(UltimateAction, ETriggerEvent::Completed, this, &AThirdPersonPlayerCharacter::Ultimate);

	PlayerEnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &AThirdPersonPlayerCharacter::Interact);
	PlayerEnhancedInputComponent->BindAction(DroneDeployAction, ETriggerEvent::Triggered, this, &AThirdPersonPlayerCharacter::DeployDrone);

	PlayerEnhancedInputComponent->BindAction(SpecialAction, ETriggerEvent::Started, this, &AThirdPersonPlayerCharacter::SpecialStart);
	PlayerEnhancedInputComponent->BindAction(SpecialAction, ETriggerEvent::Triggered, this, &AThirdPersonPlayerCharacter::SpecialHold);
	PlayerEnhancedInputComponent->BindAction(SpecialAction, ETriggerEvent::Completed, this, &AThirdPersonPlayerCharacter::SpecialRelease);

	PlayerEnhancedInputComponent->BindAction(GrenadeAction, ETriggerEvent::Started, this, &AThirdPersonPlayerCharacter::StartGrenade);
	PlayerEnhancedInputComponent->BindAction(GrenadeAction, ETriggerEvent::Triggered, this, &AThirdPersonPlayerCharacter::AimGrenade);
	PlayerEnhancedInputComponent->BindAction(GrenadeAction, ETriggerEvent::Completed, this, &AThirdPersonPlayerCharacter::ThrowGrenade);

	PlayerEnhancedInputComponent->BindAction(ShieldAction, ETriggerEvent::Started, this, &AThirdPersonPlayerCharacter::ShieldParryAction);
	PlayerEnhancedInputComponent->BindAction(ShieldAction, ETriggerEvent::Triggered, this, &AThirdPersonPlayerCharacter::Shield_Ongoing);
	PlayerEnhancedInputComponent->BindAction(ShieldAction, ETriggerEvent::Completed, this, &AThirdPersonPlayerCharacter::Shield_Completed);

	PlayerEnhancedInputComponent->BindAction(LockOnAction, ETriggerEvent::Triggered, this, &AThirdPersonPlayerCharacter::LockOn);
}

void AThirdPersonPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	NightmareGameMode = Cast<ANightmareGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	CharacterGrenadeHandlerComp = FindComponentByClass<UCharacterGrenadeHandler>();
	SpringArmComp = FindComponentByClass<USpringArmComponent>();
	CameraComp = FindComponentByClass<UCameraComponent>();
	InitialCameraPostEffects = FPostProcessSettings(CameraComp->PostProcessSettings);
	WarpHandlerComponent = FindComponentByClass<UWarpCheckerComponent>();
	SpecialAbilityHandlerComp = FindComponentByClass<USpecialAbilityHandlerComponent>();
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	TargetMovementSpeed = WalkSpeed;
	bIsAimingWeapon = false;
	SpringArmLengthInital = SpringArmComp->TargetArmLength;
	SpringArmLengthTarget = SpringArmLengthInital;
	SpringArmOffsetInit = SpringArmComp->SocketOffset;
	MyPlayerController = GetLocalViewingPlayerController();
	if (!MyPlayerController) {
		SetActorTickEnabled(false);
		return;
	}
	PlayerCameraManager = MyPlayerController->PlayerCameraManager;
	PlayerCutsceneHandler = FindComponentByClass<UPlayerCutsceneHandlerComponent>();
	WidgetsOnScreen.Empty();

	InteractOverlapComp->OnComponentBeginOverlap.AddDynamic(this, &AThirdPersonPlayerCharacter::InteractionRangeOverlap);
	InteractOverlapComp->OnComponentEndOverlap.AddDynamic(this, &AThirdPersonPlayerCharacter::InteractionRangeEndOverlap);
	OnActorHit.AddDynamic(this, &AThirdPersonPlayerCharacter::OnCollisionWithObject);
}

void AThirdPersonPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	HandleTimeDilationInterp(DeltaTime);
	if (IsWalkingOnWall())
		HandleWallWalk();
	if (IsLockedOnEnemy())
		CameraLockOnComp->HandleCameraLockOn(LookForce, DeltaTime);
	if (bIsInUltimate)
		Ultimate_Ongoing(DeltaTime);
	SpecialAbilityHandlerComp->HandleCameraPostEffects(&DeltaTime, &CameraComp->PostProcessSettings);
	bool IsSprinting = GetCharacterMovement()->MaxWalkSpeed > WalkSpeed;
	GetCharacterMovement()->MaxWalkSpeed = FMath::FInterpConstantTo(GetCharacterMovement()->MaxWalkSpeed, TargetMovementSpeed, DeltaTime, SpeedChangeRate);
	SpringArmComp->TargetArmLength = FMath::FInterpTo(SpringArmComp->TargetArmLength, SpringArmLengthTarget, DeltaTime, IsSprinting ? (SpringArmLengthTransitionRate*0.3f) : SpringArmLengthTransitionRate);
	SpringArmComp->SocketOffset = FMath::VInterpTo(SpringArmComp->SocketOffset, TargetSpringArmOffset, DeltaTime, SpringArmOffsetChangeRate);
	
	HandleInteract(DeltaTime);
}

void AThirdPersonPlayerCharacter::Die()
{
	SpringArmComp->bDoCollisionTest = false;
	DisableInput(GetLocalViewingPlayerController());
	Super::Die();
}

void AThirdPersonPlayerCharacter::StartWeaponSelection(const FInputActionInstance& ActionInstance) {
	WeaponSelectionWidget = CreateWidget<UWeaponSelectionWidget>(MyPlayerController, WeaponSelectionWidgetClass, FName("Weapon Selection Widget"));
	if (!WeaponSelectionWidget) return;
	WeaponSelectionWidget->StartMakeWidget_Implementation(this);
	WeaponSelectionWidget->SelectWeapon(GetWeaponByIndex(GetWeaponIndex(CurrentWeapon) + 1));
	WeaponSelectionWidget->StartMakeWidget(this);
	WeaponSelectionWidgetComp->SetWidget(WeaponSelectionWidget);
	WeaponSelectionWidgetComp->SetHiddenInGame(false);
}

void AThirdPersonPlayerCharacter::NextItem(const FInputActionInstance& ActionInstance) {
	const float Value = ActionInstance.GetValue().Get<float>();
	if (IsInWeaponSelection()) {
		if (InventoryComponent->InventoryWeapons.IsEmpty()) return;
		WeaponSelectionWidget->SelectWeapon(GetWeaponByIndex(GetWeaponIndex(CurrentWeapon) + (Value > 0 ? 1 : -1)));
		WeaponSelectionWidget->NextItem_Implementation(Value > 0);
		WeaponSelectionWidget->NextItem(Value > 0);
	}
}

int AThirdPersonPlayerCharacter::GetWeaponIndex(const AWeapon* WeaponRef) const
{
	return InventoryComponent->GetWeaponIndex(WeaponRef);
}

AWeapon* AThirdPersonPlayerCharacter::GetWeaponByIndex(const int Index) const {
	if (InventoryComponent->InventoryWeapons.IsEmpty()) return nullptr;
	if (Index >= InventoryComponent->InventoryWeapons.Num())
		return InventoryComponent->InventoryWeapons[0];
	else if (Index < 0)
		return InventoryComponent->InventoryWeapons[InventoryComponent->InventoryWeapons.Num()-1];
	return InventoryComponent->InventoryWeapons[Index];
}

void AThirdPersonPlayerCharacter::WeaponSelectionHeld(const FInputActionInstance& ActionInstance) {
	if (!WeaponSelectionWidget) return;
}

void AThirdPersonPlayerCharacter::EndWeaponSelection(const FInputActionInstance& ActionInstance) {
	if (!WeaponSelectionWidget) return;
	// WeaponSelectionWidget->SelectWeapon(WeaponSelectionWidget->GetSelectedWeapon());
	// WeaponSelectionWidget->RemoveFromParent();
	WeaponSelectionWidgetComp->SetWidget(nullptr);
	WeaponSelectionWidget = nullptr;
	WeaponSelectionWidgetComp->SetHiddenInGame(true);
}

void AThirdPersonPlayerCharacter::DisableCameraLagUntilNextFrame()
{
	SpringArmComp->bEnableCameraLag = false;
	FTimerDelegate Delegate;
	Delegate.BindLambda([&]() {
		SpringArmComp->bEnableCameraLag = true;
	});
	GetWorldTimerManager().SetTimerForNextTick(Delegate);
}

void AThirdPersonPlayerCharacter::StartRagdolling()
{
	if (CurrentWeapon)
		DropWeapon(CurrentWeapon);
	StopAimimg();
	bIsAimingGrenade = false;
	Super::StartRagdolling();
}

void AThirdPersonPlayerCharacter::AddImpulseToCharacter(const FVector& Impulse)
{
	// GetCharacterMovement()->AddImpulse(Impulse * 1000, true);
	GetMesh()->AddImpulse(Impulse * GetMesh()->GetMass() / 10, FName("pelvis"), true);
}

void AThirdPersonPlayerCharacter::StopRagdollingBackToAnimation()
{
	Super::StopRagdollingBackToAnimation();
	// PlayerCameraManager->SetGameCameraCutThisFrame();
}

void AThirdPersonPlayerCharacter::DeployDrone(const FInputActionInstance& ActionInstance)
{
	if (IsDroneInAir()) {
		DroneInAir->TakeControlOfDrone(this);
		return;
	}
	if (!bCanDeployDrone) {
		// TODO: Cooldown notification
		return;
	}
	FHitResult HitResult;
	FVector SpawnLocationTypical = GetActorLocation() + (GetActorForwardVector() * DroneSpawnDistanceFromPlayer);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	bool bCloseToWall = GetWorld()->SweepSingleByChannel(HitResult, GetActorLocation(), SpawnLocationTypical, FQuat::Identity, ECC_WorldStatic, FCollisionShape::MakeSphere(80.f), QueryParams);
	if (bCloseToWall) {
		SpawnLocationTypical = GetActorLocation() + (GetActorUpVector() * DroneSpawnDistanceFromPlayer);
		bCloseToWall = GetWorld()->SweepSingleByChannel(HitResult, GetActorLocation(), SpawnLocationTypical, FQuat::Identity, ECC_WorldStatic, FCollisionShape::MakeSphere(80.f), QueryParams);
		if (bCloseToWall) {
			// TODO: notification cannot deploy drone
			return;
		}
	}
	DroneInAir = GetWorld()->SpawnActor<ADronePawn>(DronePawnClass, SpawnLocationTypical, GetActorForwardVector().Rotation());
	if (DroneInAir == nullptr) {
		return;
	}
	DroneInAir->TakeControlOfDrone(this);
}

void AThirdPersonPlayerCharacter::DroneDestroyed()
{
	bCanDeployDrone = false;
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindLambda([&]() {
		bCanDeployDrone = true;
	});
	GetWorldTimerManager().SetTimer(DroneCooldownTimerHandle, TimerDelegate, DroneCooldownInSeconds, false);
}

FVector AThirdPersonPlayerCharacter::GetCameraLookingDirection() const
{
	return CameraComp->GetForwardVector();
}

bool AThirdPersonPlayerCharacter::IsPlayerInputEnabled() const
{
	return MyPlayerController != nullptr && MyPlayerController->InputEnabled();
}

void AThirdPersonPlayerCharacter::ChangePlayerCollision(bool bNewCollisionState)
{
	GetCapsuleComponent()->SetCollisionEnabled(bNewCollisionState ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
}

void AThirdPersonPlayerCharacter::ChangePlayerMovement(EMovementMode NewMovementMode)
{
	GetCharacterMovement()->SetMovementMode(NewMovementMode);
}

bool AThirdPersonPlayerCharacter::IsLockedOnEnemy() const {
	return CameraLockOnComp->IsLockedOn();
}

void AThirdPersonPlayerCharacter::LockOn()
{
	if (IsLockedOnEnemy())
		CameraLockOnComp->StopLockOn_FORCED();
	else
		CameraLockOnComp->StartLockOn();
}

void AThirdPersonPlayerCharacter::Look(const FInputActionInstance& ActionInstance)
{
	if (!MyPlayerController->InputEnabled()) return;
	UCharacterMovementComponent* CharMovement = GetCharacterMovement();
	if (CharMovement->MovementMode == EMovementMode::MOVE_None) return;
	const FVector2D Axis = ActionInstance.GetValue().Get<FVector2D>() * MouseSensivity;
	LookForce = Axis;
	if (IsLockedOnEnemy()) return;
	AddControllerYawInput(Axis.X);
	//if (!IsWalkingOnWall())
		//AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void AThirdPersonPlayerCharacter::Move(const FInputActionInstance& ActionInstance)
{
	if (!bCanMove || !MyPlayerController->InputEnabled()) return;
	UCharacterMovementComponent* CharMovement = GetCharacterMovement();
	if (CharMovement->MovementMode == EMovementMode::MOVE_None) return;
	if (!IsEvading && !bCanEvade)
		StopAnimMontage();
	MovementDirection = ActionInstance.GetValue().Get<FVector2D>() * GetCurrentMovementSpeed();
	const FRotator ControlRotation = GetControlRotation();
	const FVector ControlForwardVector = UKismetMathLibrary::GetForwardVector(FRotator(0, ControlRotation.Yaw, 0));
	// AddMovementInput(ControlForwardVector, MovementDirection.Y);
	if (IsWalkingOnWall()) {
		if (MovementDirection.Y < 0) return;
		AddMovementInput(GetMovementDirectionOnWallWalking(WallNormal), MovementDirection.Y);
	}
	else {
		AddMovementInput(UKismetMathLibrary::GetRightVector(FRotator(0, ControlRotation.Yaw, ControlRotation.Roll)), MovementDirection.X);
		AddMovementInput(ControlForwardVector, MovementDirection.Y);
	}
	PlayerCameraManager->StartCameraShake(WalkingCameraShake);
	
}

void AThirdPersonPlayerCharacter::EvadeEnd()
{
	ResetMovementVelocity(true);
	FTimerDelegate MyDelegate;
	 
	MyDelegate.BindLambda([&]() {
		// SpringArmComp->CameraLagSpeed = 20.f;
		
		if (TargetMovementSpeed > WalkSpeed) {
			SpringArmLengthTarget = SprintSpringArmLength;
			TargetMovementSpeed = SprintSpeed;
		}
		else if (bIsAimingWeapon) {
			SpringArmLengthTarget = SpringArmLengthAimingMult * SpringArmLengthInital;
			TargetMovementSpeed = WalkSpeed;
			TargetSpringArmOffset = SpringArmOffsetAim;
		}
		
		bCanEvade = true;
		CurrentCollisionEvades = 0;
	});
	// SpringArmComp->bEnableCameraLag = true;
	IsEvading = false;
	GetWorldTimerManager().SetTimer(EvadeTimerHandle, MyDelegate, 1.f, false);
}

UAnimMontage* AThirdPersonPlayerCharacter::GetEvadeMontageFromDirection() const {
	if (MovementDirection.Length() == 0 || !bUseControllerRotationYaw) return GetCharacterMovement()->IsFalling() ? Air_Evade_Fw_AnimMontage : Evade_Fw_AnimMontage;
	const float AbsY = FMath::Abs(MovementDirection.Y);
	const float AbsX = FMath::Abs(MovementDirection.X);
	if (AbsY >= AbsX)
		return MovementDirection.Y >= 0 ? Evade_Fw_AnimMontage : Evade_B_AnimMontage;
	else
		return MovementDirection.X >= 0 ? Evade_R_AnimMontage : Evade_L_AnimMontage;
}

void AThirdPersonPlayerCharacter::Evade(const FInputActionInstance& ActionInstance)
{
	if (!bCanEvade || !MyPlayerController->InputEnabled() || GetMesh()->GetAnimInstance()->Montage_IsPlaying(nullptr)) {
		UE_LOG(LogTemp, Warning, TEXT("CANNOT"));
		return;
	}
	bCanEvade = false;
	
	if (!bIsAimingWeapon) {
		bIsAimingGrenade = false;
		StopAimimg();
		IsEvading = true;
		// SpringArmComp->CameraLagSpeed = 10.f;
		// SpringArmComp->bEnableCameraLag = false;
		PlayAnimMontage(GetEvadeMontageFromDirection());
		return;
	}
	
	bIsAimingGrenade = false;
	//StopAimimg();
	// ChangePlayerMovement(EMovementMode::MOVE_Flying);
	// StopAnimMontage(CurrentWeapon->EquipMontage);
	// StopAnimMontage(CurrentWeapon->UnEquipMontage);
	IsEvading = true;
	PlayAnimMontage(GetEvadeMontageFromDirection());
	
}

void AThirdPersonPlayerCharacter::Ultimate(const FInputActionInstance& ActionInstance)
{
	if (!MyPlayerController->InputEnabled()) return;
	UCharacterMovementComponent* CharMovement = GetCharacterMovement();
	if (CharMovement->MovementMode == EMovementMode::MOVE_None || CharMovement->IsFalling() || IsEvading || bIsFallingStompingGround) return;
	if (ActionInstance.GetValue().Get<bool>() && GetVelocity().Length() <= 10.f && !bIsInUltimate && bCanMove)
		StartUltimate();
	else if (bIsInUltimate)
		EndUltimate();
}

void AThirdPersonPlayerCharacter::Interact(const FInputActionInstance& ActionInstance)
{
	if (!MyPlayerController->InputEnabled()) return;
	// TODO: Check for normal interaction
	const bool bIsInteracting = ActionInstance.GetValue().Get<bool>();
	if (bIsInteracting) {
		if (!InteractingObject) return;
		InteractingObject->StartInteraction();
		return;
	}
	StopInteract();

	const bool bResult = WarpHandlerComponent->SwapLocation();
}

void AThirdPersonPlayerCharacter::Sprint(const FInputActionInstance& ActionInstance)
{
	const bool bValue = ActionInstance.GetValue().Get<bool>();
	UCharacterMovementComponent* CharMovement = GetCharacterMovement();
	if (CharMovement->MovementMode == EMovementMode::MOVE_None || bIsAimingWeapon || IsWalkingOnWall()) return;
	if (!MyPlayerController->InputEnabled()) {
		if (!bValue)
			TargetMovementSpeed = WalkSpeed;
		return;	
	}
	if (bValue) {
		SpringArmComp->CameraLagSpeed = 25.f;
		StopAimimg();
		TargetSpringArmOffset = SpringArmOffsetSprint;
		SpringArmLengthTarget = SprintSpringArmLength;
		TargetMovementSpeed = SprintSpeed;
		// GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	}
	else {
		SpringArmComp->CameraLagSpeed = 20.f;
		TargetSpringArmOffset = SpringArmOffsetInit;
		SpringArmLengthTarget = SpringArmLengthInital;
		TargetMovementSpeed = WalkSpeed;
	}
}

void AThirdPersonPlayerCharacter::StompGroundEnded_Event() {
	bCanEvade = true;
	bCanMove = true;
	bIsFallingStompingGround = false;
	CurrentWeapon->BulletsToAddAfterReloadComplete = 0;
}

void AThirdPersonPlayerCharacter::StompGround_Event() {
	// TODO: AoE Effects and etc.
	/*
	TArray<FHitResult> HitResults;
	const FVector Start = GetActorLocation();
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	const bool bIsHit = WorldRef->SweepMultiByChannel(HitResults, Start, Start, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(StompGroundDamageAreaRadius), QueryParams);
	if (!bIsHit) return;
	*/
	GetCharacterMovement()->GravityScale = 1.75f;
	TArray<AActor*> Ignores;
	if (CurrentWeapon)
		Ignores.Add(CurrentWeapon);
	if (PistolWeapon)
		Ignores.Add(PistolWeapon);
	Ignores.Add(this);
	UGameplayStatics::ApplyRadialDamage(GetWorld(), StompGroundDamage, GetActorLocation() - FVector(0,0, GetCapsuleHalfHeight() - 3.f), StompGroundDamageAreaRadius, StompGroundDamageType, Ignores, this, MyPlayerController);
	ApplyEpicEffect(0.1f, FVector::ZeroVector, 0.1f, false, false, true, 0.3f);
}

void AThirdPersonPlayerCharacter::Jump()
{
	if (!MyPlayerController->InputEnabled() || bIsFallingStompingGround || bIsInUltimate) return;
	bIsAimingGrenade = false;
	if (IsInCutsceneTrigger() && IsPlayerActionAndActionRequiredEqual(JumpAction)) {
		PlayerCutsceneHandler->StartCutscene();
		return;
	}
	UCharacterMovementComponent* CharMovement = GetCharacterMovement();
	// if (CharMovement->IsFalling() || CharMovement->MovementMode == EMovementMode::MOVE_None) return;
	const bool bIsFalling = CharMovement->IsFalling();
	if (CharMovement->MovementMode == EMovementMode::MOVE_None) return;
	// TODO: Collision Check for a climbable object
	if (CurrentWallWalkSide != EWallWalkType::NO_WALL) {
		// Already on wall. We jump to other side.
		JumpToOtherSideFromWallWalking(CharMovement);
		return;
	}
	if (bIsFalling) {
		FHitResult WallWalk_HitResult;
		CurrentWallWalkSide = IsNextToWalkableWall(WallWalk_HitResult);
		if (CurrentWallWalkSide != EWallWalkType::NO_WALL) {
			WallNormal = FVector(WallWalk_HitResult.ImpactNormal.X, WallWalk_HitResult.ImpactNormal.Y, 0);
			switch (CurrentWallWalkSide)
			{
			case EWallWalkType::RIGHT_WALL:
				StartWalkingOnWall(CharMovement);
				break;
			case EWallWalkType::LEFT_WALL:
				StartWalkingOnWall(CharMovement);
				break;
			default:
				break;
			}
			const FRotator FinalRotation = GetMovementDirectionOnWallWalking(
				FVector(WallWalk_HitResult.ImpactNormal.X, WallWalk_HitResult.ImpactNormal.Y, 0)).Rotation();
			MyPlayerController->SetControlRotation(FinalRotation);
			SetActorRotation(FinalRotation);
			CharMovement->Velocity = GetMovementDirectionOnWallWalking(WallNormal) * WallWalkingSpeed;
			return;
		}
	}
	FHitResult HitResult;
	const EClimbType bResult = IsFacingClimbableObject(HitResult);
	// UE_LOG(LogTemp, Warning, TEXT("Is facing climbable: %d"), bResult ? 1 : 0);
	FVector FeetLocation = GetActorLocation();
	if (bResult == EClimbType::NOT_CLIMBABLE || bResult == EClimbType::TOO_TALL_NOT_CLIMBABLE) {
		// bCanEvade = true;
		if (bIsFalling) {
			// TODO: Disable Shooting.
			IsEvading = false;
			bCanEvade = false;
			StopAnimMontage();
			StompGroundZStart = FeetLocation.Z;
			ResetMovementVelocity(true);
			CharMovement->GravityScale = StompFallGravityMult;
			bCanMove = false;
			bIsFallingStompingGround = true;
			return;
		}
		else if (IsEvading) return;
		IsJumping = true;
		// Roll();
		return;
	}
	FeetLocation.Z -= GetCapsuleHalfHeight();
	FVector HitLoc = HitResult.ImpactPoint;
	HitLoc.Z = GetActorLocation().Z;
	// SetActorRotation((HitLoc - GetActorLocation()).Rotation());
	/*
	if (bResult == EClimbType::NORMAL_CLIMB) {
		if (CurrentWeapon && bHasWeaponEquipped) {
			EquipAttachWeaponToHand(false);
			FinishUnequipWeapon();
		}
		//ResetMovementVelocity(true);
		FVector Difference = HitResult.ImpactPoint - FeetLocation;
		FVector Final = GetActorLocation() + Difference - (GetActorForwardVector() * 90);// +FVector(0, 0, 10);
		SetActorLocation(Final);
		//GetCharacterMovement()->Velocity = FVector(GetCharacterMovement()->Velocity.X, GetCharacterMovement()->Velocity.Y, 1500.f);
		PlayAnimMontage(NormalClimbAnimMontage);
	}
	else if (bResult == EClimbType::LEDGE_CLIMB) {
		if (CurrentWeapon && bHasWeaponEquipped) {
			EquipAttachWeaponToHand(false);
			FinishUnequipWeapon();
		}
		ResetMovementVelocity(true);
		FVector Difference = HitResult.ImpactPoint - GetActorLocation();
		FVector Final = GetActorLocation() + Difference - FVector(0,0, 70) - (GetActorForwardVector() * 30);
		CharMovement->SetMovementMode(EMovementMode::MOVE_Flying);
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		MyPlayerController->DisableInput(MyPlayerController);
		SetActorLocation(Final);
		
		PlayAnimMontage(LedgeClimbAnimMontage);
	}
	*/
	// DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 20, 10, FColor::Green, true);
	// TODO: Play Climb Montage
}

float AThirdPersonPlayerCharacter::GetCurrentMovementSpeed() const
{
	return GetCharacterMovement()->MaxWalkSpeed;
}

void AThirdPersonPlayerCharacter::ResetMovementVelocity(bool bShouldResetZ)
{
	const float NewVelZ = bShouldResetZ ? 0 : (GetCharacterMovement()->Velocity.Z / 2);
	GetCharacterMovement()->Velocity = FVector(0, 0, NewVelZ);
}

void AThirdPersonPlayerCharacter::OnCollisionWithObject(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!MyPlayerController->InputEnabled() || !IsEvading) return;
	if (AEnemyBaseCharacter* OtherEnemy = Cast<AEnemyBaseCharacter>(OtherActor)) {
		OtherEnemy->ReduceHealth(30.f);
		FName BoneHit;
		switch (FMath::RandRange(1, 5)) {
			case 1:
				BoneHit = FName("thigh_r");
				break;
			case 2:
				BoneHit = FName("thigh_l");
				break;
			case 3:
				BoneHit = FName("spine_02");
				break;
			case 4:
				BoneHit = FName("upperarm_l");
				break;
			case 5:
				BoneHit = FName("upperarm_r");
				break;
			default:
				break;
		}
		OtherEnemy->ApplyDismembermentToLimb(BoneHit, NormalImpulse * 5000.f, Hit.ImpactPoint, true);
		CurrentCollisionEvades++;
		bCanEvade = false;
		if (CurrentCollisionEvades > CollisionEvadesMax) return;
		if (!bIsAimingWeapon) {
			bIsAimingGrenade = false;
			StopAimimg();
			IsEvading = true;
			// PlayAnimMontage(Evade_FwAnimMontage);
			FTimerDelegate MyDelegate;
			MyDelegate.BindLambda([&]() {
				if (TargetMovementSpeed > WalkSpeed) {
					SpringArmLengthTarget = SprintSpringArmLength;
					TargetMovementSpeed = SprintSpeed;
				}
				else if (bIsAimingWeapon) {
					SpringArmLengthTarget = SpringArmLengthAimingMult * SpringArmLengthInital;
				}
				bCanEvade = true;
				CurrentCollisionEvades = 0;
			});
			GetWorldTimerManager().SetTimer(EvadeTimerHandle, MyDelegate, 1.f, false);
			return;
		}
	}
}

EClimbType AThirdPersonPlayerCharacter::IsFacingClimbableObject(FHitResult& OutHitResult, bool bSameObjectIgnore) const
{
	FVector Start = GetActorLocation();
	FVector End = Start + (GetActorForwardVector() * ClimbableRangeCheck);
	FHitResult FirstHitResult;
	FCollisionQueryParams CollisionParams = FCollisionQueryParams();
	CollisionParams.AddIgnoredActor(this);
	bool bIsHit = GetWorld()->SweepSingleByChannel(FirstHitResult,
		Start,
		End,
		FQuat::Identity,
		ECollisionChannel::ECC_WorldStatic,
		FCollisionShape::MakeSphere(10), CollisionParams);
	if (!bIsHit) {
		UE_LOG(LogTemp, Warning, TEXT("Did not find"));
		return EClimbType::NOT_CLIMBABLE;
	}
	const float CapsuleHalfHeight = GetCapsuleHalfHeight();
	Start = FirstHitResult.ImpactPoint + (GetActorUpVector() * CapsuleHalfHeight);
	End = Start;
	FHitResult SecondHitResult;
	bIsHit = GetWorld()->SweepSingleByChannel(SecondHitResult,
		Start,
		End,
		FQuat::Identity,
		ECollisionChannel::ECC_WorldStatic,
		FCollisionShape::MakeSphere(2), CollisionParams);
	if (bIsHit) {
		if (!bSameObjectIgnore && SecondHitResult.GetActor() != FirstHitResult.GetActor()) {
			// Not same object
			UE_LOG(LogTemp, Warning, TEXT("Not same object"));
			return EClimbType::NOT_CLIMBABLE;
		}
		Start = SecondHitResult.ImpactPoint + (GetActorUpVector() * (CapsuleHalfHeight+10));
		// DrawDebugSphere(GetWorld(), SecondHitResult.ImpactPoint, 20, 10, FColor::White, true);
		End = Start;
		FHitResult ThirdHitResult;
		bIsHit = GetWorld()->SweepSingleByChannel(ThirdHitResult ,
			Start,
			End,
			FQuat::Identity,
			ECollisionChannel::ECC_WorldStatic,
			FCollisionShape::MakeSphere(10), CollisionParams);
		if (bIsHit) {  // Object is too tall
			// DrawDebugSphere(GetWorld(), ThirdHitResult.ImpactPoint, 20, 10, FColor::Red, true);
			return EClimbType::TOO_TALL_NOT_CLIMBABLE;
		}
		FVector StartA = SecondHitResult.ImpactPoint;
		FHitResult PrevHitResult = SecondHitResult;
		for (int i = 0; i < Start.Z; i+=1)
		{
			StartA.Z += i;
			PrevHitResult = SecondHitResult;
			// DrawDebugSphere(GetWorld(), StartA, 5, 10, FColor::Red, true);
			bIsHit = GetWorld()->SweepSingleByChannel(SecondHitResult,
				StartA,
				StartA,
				FQuat::Identity,
				ECollisionChannel::ECC_WorldStatic,
				FCollisionShape::MakeSphere(1), CollisionParams);
			if (!bIsHit) break;
		}
		// Ledge Climb
		UE_LOG(LogTemp, Warning, TEXT("Ledge Climb"));
		// DrawDebugSphere(GetWorld(), PrevHitResult.ImpactPoint, 5, 10, FColor::Blue, true);
		OutHitResult = PrevHitResult;
		return EClimbType::LEDGE_CLIMB;
	}
	// Normal Climb
	FVector StartA = FirstHitResult.ImpactPoint;
	FHitResult PrevHitResult = FirstHitResult;
	for (int i = 0; i < Start.Z; i += 1)
	{
		StartA.Z += i;
		PrevHitResult = FirstHitResult;
		// DrawDebugSphere(GetWorld(), StartA, 5, 10, FColor::Red, true);
		bIsHit = GetWorld()->SweepSingleByChannel(FirstHitResult,
			StartA,
			StartA,
			FQuat::Identity,
			ECollisionChannel::ECC_WorldStatic,
			FCollisionShape::MakeSphere(1), CollisionParams);
		if (!bIsHit) break;
	}
	UE_LOG(LogTemp, Warning, TEXT("Normal Climb"));
	OutHitResult = PrevHitResult;
	return EClimbType::NORMAL_CLIMB;
}

void AThirdPersonPlayerCharacter::StopAimimg(float WaitTimeInSeconds)
{
	if (WaitTimeInSeconds <= 0) {
		bIsAimingWeapon = false;
		bUseControllerRotationYaw = false;
		SpringArmLengthTarget = SpringArmLengthInital;
		TargetSpringArmOffset = SpringArmOffsetInit;
		if (CurrentWeapon)
			CurrentShootingWeaponID = CurrentWeapon->WeaponID;
		return;
	}
	FTimerDelegate Delegate;
	Delegate.BindLambda([&]() {
		if (bIsAimingWeapon) return;
		bIsAimingWeapon = false;
		bUseControllerRotationYaw = false;
		SpringArmLengthTarget = SpringArmLengthInital;
		TargetSpringArmOffset = SpringArmOffsetInit;
		if (CurrentWeapon)
			CurrentShootingWeaponID = CurrentWeapon->WeaponID;
	});
	GetWorldTimerManager().SetTimer(StopAimingTimerHandle, Delegate, WaitTimeInSeconds, false);

}

void AThirdPersonPlayerCharacter::AimWeapon(const FInputActionInstance& ActionInstance)
{
	if (bIsFallingStompingGround || bIsInUltimate || !bCanMove) return;
	const bool bValue = ActionInstance.GetValue().Get<bool>();
	if (IsInCutsceneTrigger() && IsPlayerActionAndActionRequiredEqual(ActionInstance.GetSourceAction())) {
		PlayerCutsceneHandler->StartCutscene();
		return;
	}
	UCharacterMovementComponent* CharMovement = GetCharacterMovement();
	if (CharMovement->MovementMode == EMovementMode::MOVE_None) return;
	if (!MyPlayerController->InputEnabled()) {
		if (!bValue)
			StopAimimg();
		return;
	}
	// if (GetCurrentMovementSpeed() == SprintSpeed) return;
	bUseControllerRotationYaw = IsWalkingOnWall() ? false : bValue;
	bIsAimingWeapon = bValue;
	SpringArmLengthTarget = (bIsAimingWeapon ? SpringArmLengthAimingMult : 1) * SpringArmLengthInital;
	if (bIsAimingWeapon) {
		if (!bCanEvade && !IsEvading) {
			ApplyEpicEffect(0.2f, FVector::ZeroVector, 0.5f, false, false, true, 0.4f);
		}
		TargetSpringArmOffset = SpringArmOffsetAim;
		TargetMovementSpeed = WalkSpeed;
		SpringArmLengthTarget = SpringArmLengthAimingMult * SpringArmLengthInital;
	}
	else {
		TargetSpringArmOffset = SpringArmOffsetInit;
		SpringArmLengthTarget = SpringArmLengthInital;
	}
}

void AThirdPersonPlayerCharacter::WeaponShootEvent(float CustomDamage)
{
	if (!CurrentWeapon) return;
	CurrentShootingWeaponID = CurrentWeapon->WeaponID;
	FVector Start;
	FRotator Rotation;
	MyPlayerController->GetPlayerViewPoint(Start, Rotation);
	FVector End = Start + (CameraComp->GetForwardVector() * 10000.f);
	if (!CurrentWeapon->Shoot(Start, End, true, true, CustomDamage)) return;
	if (!CurrentWeapon->ShootingCameraShake) return;
	PlayerCameraManager->StartCameraShake(CurrentWeapon->ShootingCameraShake);
	if (GetCharacterMovement()->IsFalling()) {
		ShotsDoneGravity++;
		if (ShotsDoneGravity >= CurrentWeapon->ShotsDoneBeforeGravityEnabled) return;
		ResetMovementVelocity(true);
	}
}


void AThirdPersonPlayerCharacter::PistolShootEvent(float CustomDamage)
{
	if (!PistolWeapon) return;
	//CurrentShootingWeaponID = PistolWeapon->WeaponID;
	bUseControllerRotationYaw = true;
	if (!bIsAimingWeapon)
		StopAimimg(4.f);
	FVector Start;
	FRotator Rotation;
	MyPlayerController->GetPlayerViewPoint(Start, Rotation);
	FVector End = Start + (CameraComp->GetForwardVector() * 10000.f);
	if (!PistolWeapon->Shoot(Start, End, true, true, CustomDamage)) return;
	//if (!bIsAimingWeapon)
	if (!PistolWeapon->ShootingCameraShake) return;
	PlayerCameraManager->StartCameraShake(PistolWeapon->ShootingCameraShake);
	if (!IsWalkingOnWall() && GetCharacterMovement()->IsFalling()) {
		ShotsDoneGravity++;
		if (ShotsDoneGravity >= PistolWeapon->ShotsDoneBeforeGravityEnabled) return;
		ResetMovementVelocity(true);
	}
}

void AThirdPersonPlayerCharacter::StartShootPistol(const FInputActionInstance& ActionInstance) {
	if (!MyPlayerController->InputEnabled() || PistolWeapon == nullptr || bIsFallingStompingGround || IsEvading
		|| (!bIsAimingWeapon && IsWalkingOnWall())) return;
	bIsAimingGrenade = false;
	UCharacterMovementComponent* CharMovement = GetCharacterMovement();
	if (CharMovement->MovementMode == EMovementMode::MOVE_None) return;
	const ECombatComboType WeaponComboType = ECombatComboType::LIGHT;
	const FVector Vel = GetVelocity();
	//CurrentShootingWeaponID = PistolWeapon->WeaponID;
	bUseControllerRotationYaw = true;
	if (!bIsAimingWeapon)
		StopAimimg(4.f);
	UAnimMontage* ComboFinishAnimMontage = PlayerComboComponent->CurrentShootComboAdd(WeaponComboType, FMath::Abs(Vel.X + Vel.Y));
	if (ComboFinishAnimMontage == nullptr) {
		PlayAnimMontage(FMath::Abs(Vel.X + Vel.Y) > 5 ? PistolWeapon->CharacterMovingShootMontage : PistolWeapon->CharacterShootMontage);
		return;
	}
	// Combo Execute
	// Shooting raycast happens as an event of the animation.
	PlayAnimMontage(ComboFinishAnimMontage);
}

void AThirdPersonPlayerCharacter::ShootPistol(const FInputActionInstance& ActionInstance) {
	
}

void AThirdPersonPlayerCharacter::StopShootPistol(const FInputActionInstance& ActionInstance) {

}

void AThirdPersonPlayerCharacter::StartShootWeapon(const FInputActionInstance& ActionInstance)
{
	if (!MyPlayerController->InputEnabled() || CurrentWeapon == nullptr || bIsFallingStompingGround || (!bIsAimingWeapon && IsWalkingOnWall())) return;
	bIsAimingGrenade = false;
	if (!bHasWeaponEquipped) return;
	UCharacterMovementComponent* CharMovement = GetCharacterMovement();
	if (CharMovement->MovementMode == EMovementMode::MOVE_None) return;
	if (!bIsAimingWeapon && CurrentWeapon->bShouldAimToShoot) return;
	CurrentWeapon->bIsFiring = true;
}

void AThirdPersonPlayerCharacter::ShootWeapon(const FInputActionInstance& ActionInstance)
{
	if (!MyPlayerController->InputEnabled() || CurrentWeapon == nullptr || bIsFallingStompingGround || (!bIsAimingWeapon && IsWalkingOnWall())) return;
	bIsAimingGrenade = false;
	if (!bHasWeaponEquipped) return;
	UCharacterMovementComponent* CharMovement = GetCharacterMovement();
	if (CharMovement->MovementMode == EMovementMode::MOVE_None) return;
	if ((!bIsAimingWeapon && CurrentWeapon->bShouldAimToShoot) || CurrentWeapon->CurrentFireRatePoint < 95.f) return;
	CurrentShootingWeaponID = CurrentWeapon->WeaponID;
	if (!IsWalkingOnWall())
		bUseControllerRotationYaw = true;
	if (!bIsAimingWeapon)
		StopAimimg(4.f);
	// PlayAnimMontage(ShootPistolAnimMontage);
	FVector Start;
	FRotator Rotation;
	MyPlayerController->GetPlayerViewPoint(Start, Rotation);
	FVector End = Start + (CameraComp->GetForwardVector() * 10000.f);

	// Combo
	const ECombatComboType WeaponComboType = static_cast<ECombatComboType>(CurrentWeapon->WeaponCombatType);
	const FVector Vel = GetVelocity();
	UAnimMontage* ComboFinishAnimMontage = PlayerComboComponent->CurrentShootComboAdd(WeaponComboType, Vel.X + Vel.Y);
	if (WeaponComboType == ECombatComboType::NOT_COMBO_TYPE || ComboFinishAnimMontage == nullptr) {
		if (!CurrentWeapon->Shoot(Start, End)) return;
		// FVector ControlRotation = End - Start;
		PlayAnimMontage(FMath::Abs(Vel.X + Vel.Y) > 5 ? (bIsAimingWeapon ? CurrentWeapon->CharacterMovingADSShootMontage : CurrentWeapon->CharacterMovingShootMontage) : CurrentWeapon->CharacterShootMontage);
		if (!CurrentWeapon->ShootingCameraShake) return;
		PlayerCameraManager->StartCameraShake(CurrentWeapon->ShootingCameraShake);
		if (!IsWalkingOnWall() && CharMovement->IsFalling()) {
			ShotsDoneGravity++;
			if (ShotsDoneGravity >= CurrentWeapon->ShotsDoneBeforeGravityEnabled) return;
			ResetMovementVelocity(true);
		}
		return;
	}
	// Combo Execute
	// Shooting raycast happens as an event of the animation.
	CurrentWeapon->CurrentFireRatePoint = 0.f;
	UE_LOG(LogTemp, Warning, TEXT("Executing..."));
	PlayAnimMontage(ComboFinishAnimMontage);
}

void AThirdPersonPlayerCharacter::StopShootWeapon(const FInputActionInstance& ActionInstance)
{
	if (!CurrentWeapon || bIsFallingStompingGround) return;
	CurrentWeapon->bIsFiring = false;
	CurrentWeapon->CurrentFireRatePoint = 100.f;
}

void AThirdPersonPlayerCharacter::SetCanMove(bool InbCanMove)
{
	EnableInput(MyPlayerController);
	// bCanMove = InbCanMove;
}

void AThirdPersonPlayerCharacter::SpecialStart(const FInputActionInstance& ActionInstance)
{
	if (!bCanMove || !MyPlayerController->InputEnabled()) return;
	UCharacterMovementComponent* CharMovement = GetCharacterMovement();
	if (CharMovement->MovementMode == EMovementMode::MOVE_None) return;
	FinishUnequipWeapon();
	EquipAttachWeaponToHand(false);
	// TODO: Special Hold as well.
	SpecialAbilityHandlerComp->SpecialActionStart();
}

void AThirdPersonPlayerCharacter::SpecialHold(const FInputActionInstance& ActionInstance)
{
	if (!bCanMove || !MyPlayerController->InputEnabled()) return;
	SpecialAbilityHandlerComp->SpecialActionHold();
}

void AThirdPersonPlayerCharacter::SpecialRelease(const FInputActionInstance& ActionInstance)
{
	// ResetCameraPostEffects();
	if (!bCanMove || !MyPlayerController->InputEnabled()) return;
	UCharacterMovementComponent* CharMovement = GetCharacterMovement();
	if (CharMovement->MovementMode == EMovementMode::MOVE_None) return;
	SpecialAbilityHandlerComp->SpecialActionRelease();
}

void AThirdPersonPlayerCharacter::UltimateShoot_Event()
{
	bUseControllerRotationYaw = false;
	PlayerComboComponent->Ultimate_Shoot();
	TargetSpringArmOffset = SpringArmOffsetInit;
	SpringArmLengthTarget = SpringArmLengthInital;
}

void AThirdPersonPlayerCharacter::Ultimate_Ongoing(float& DeltaTime)
{
	FVector Start;
	FRotator Rotation;
	MyPlayerController->GetPlayerViewPoint(Start, Rotation);
	PlayerComboComponent->Ultimate_Ongoing(DeltaTime, this, Start, CameraComp->GetForwardVector());
}

void AThirdPersonPlayerCharacter::UltimateFinished_Event()
{
	bCanEvade = true;
	bCanMove = true;
	CurrentWeapon->BulletsToAddAfterReloadComplete = 0;
}

void AThirdPersonPlayerCharacter::EndUltimate()
{
	bIsInUltimate = false;
}

void AThirdPersonPlayerCharacter::StartUltimate()
{
	TargetSpringArmOffset = SpringArmOffsetUltimate;
	SpringArmLengthTarget = SpringArmLengthAimingMult * SpringArmLengthInital;
	bCanEvade = false;
	bCanMove = false;
	bIsInUltimate = true;
	bUseControllerRotationYaw = true;
	FVector Start;
	FRotator Rotation;
	MyPlayerController->GetPlayerViewPoint(Start, Rotation);
	PlayerComboComponent->Ultimate_Start(this, Start, CameraComp->GetForwardVector());
}

void AThirdPersonPlayerCharacter::ReloadComplete_Event()
{
	if (!CurrentWeapon) return;
	CurrentWeapon->ReloadComplete(CurrentWeapon->DoesReloadAddOneBullet ? 1 : 0);
}

void AThirdPersonPlayerCharacter::FourthDimensionStartOpening(const FInputActionInstance& ActionInstance)
{
	
}

void AThirdPersonPlayerCharacter::GrabbedByFlyingMutant(AEnemyBaseCharacter* InEnemyCharacterGrabbed)
{
	EnemyCharacterThatGrabbedMe = InEnemyCharacterGrabbed;
	if (CurrentWeapon)
		CurrentWeapon->ReloadCancelled();
	bCanMove = false;
	bCanEvade = false;
	MyPlayerController->EnableInput(MyPlayerController);
	StopAnimMontage();
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ChangePlayerMovement(EMovementMode::MOVE_None);
}

void AThirdPersonPlayerCharacter::ReleasedFromGrab()
{
	// TODO: Release and struggle actions.
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	EnemyCharacterThatGrabbedMe = nullptr;
	bCanMove = true;
	bCanEvade = true;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ChangePlayerMovement(EMovementMode::MOVE_Walking);
}

void AThirdPersonPlayerCharacter::OpenFourthDimension_Implementation(FTransform PortalTransform)
{
	// TODO
	SpawnedPortalToFourthDimension = GetWorld()->SpawnActor<AFourthDimension_Portal>(PortalTo4THDimensionClass, PortalTransform);
	// SpawnedPortalToFourthDimension->FinishSpawning(PortalTransform);
	SpawnedPortalToFourthDimension->InitializePortal(NightmareGameMode->PortalToOverworld, CustomTimeDilation);
	// SpawnedPortalToFourthDimension->LinkedPortal
	StartResettingCameraPostEffects();
}

void AThirdPersonPlayerCharacter::TeleportToFourthDimension()
{
	LastOverworldTransform = FTransform(GetActorTransform());
	SetActorTransform(NightmareGameMode->FourthDimensionLocationActor->GetActorTransform(), false, nullptr, ETeleportType::TeleportPhysics);
	bIsInFourthDimension = true;
}

void AThirdPersonPlayerCharacter::ComeBackToOverworld()
{
	SetActorTransform(LastOverworldTransform, false, nullptr, ETeleportType::TeleportPhysics);
	MyPlayerController->SetControlRotation(LastOverworldTransform.Rotator());
	bIsInFourthDimension = false;
}

void AThirdPersonPlayerCharacter::OverworldComebackCompleted()
{
	FTimerDelegate MyDelegate;
	MyDelegate.BindLambda([&]() {
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.f);
		CustomTimeDilation = 1.f;
		PlayerCameraManager->StopAllCameraShakes();
		});
	GetWorldTimerManager().SetTimer(FourthDimensionFreezeTimerHandle, MyDelegate, UnfreezeEverythingAfterSeconds * 0.0001f, false);
	
}

void AThirdPersonPlayerCharacter::ResetCameraPostEffects()
{
	SpecialAbilityHandlerComp->CurrentAbilityCompletionAmount = 0.f;
	SpecialAbilityHandlerComp->CurrentTargetCompletionSeconds = 0.f;
	CameraComp->PostProcessSettings = FPostProcessSettings(InitialCameraPostEffects);
	PlayerCameraManager->StopAllCameraShakes(false);
}

void AThirdPersonPlayerCharacter::StartResettingCameraPostEffects()
{
	SpecialAbilityHandlerComp->bIsMarkedForReset = true;
}

void AThirdPersonPlayerCharacter::StartGrenade(const FInputActionInstance& ActionInstance)
{
	StopAnimMontage();
	bIsAimingGrenade = true;
	TargetMovementSpeed = WalkSpeed;
	if (CurrentWeapon != nullptr)
		CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, WeaponAttachmentSocketName);
	bHasWeaponEquipped = false;
}

void AThirdPersonPlayerCharacter::Jump_Jump()
{
	IsJumping = false;
	Super::Jump();
}

void AThirdPersonPlayerCharacter::AimGrenade(const FInputActionInstance& ActionInstance)
{
	bUseControllerRotationYaw = true;
	CharacterGrenadeHandlerComp->OngoingGrenadeAim(bIsAimingWeapon);
}

void AThirdPersonPlayerCharacter::ThrowGrenade(const FInputActionInstance& ActionInstance)
{
	StopAnimMontage();
	if (bIsAimingWeapon) {
		bUseControllerRotationYaw = true;
		if (GrenadeAimThrowMontage)
			PlayAnimMontage(GrenadeAimThrowMontage);
	}
	else {
		bUseControllerRotationYaw = false;
		if (GrenadeThrowMontage)
			PlayAnimMontage(GrenadeThrowMontage);
	}
	// bIsAimingGrenade = false;
	// TODO: Throw on Anim Montage End
	
}

void AThirdPersonPlayerCharacter::Reload(const FInputActionInstance& ActionInstance)
{
	if (!MyPlayerController->InputEnabled() || !bHasWeaponEquipped || CurrentWeapon->BulletsToAddAfterReloadComplete > 0) return;
	UCharacterMovementComponent* CharMovement = GetCharacterMovement();
	if (CharMovement->MovementMode == EMovementMode::MOVE_None) return;
	if (CurrentWeapon->Reload()) {
		const FVector Vel = GetVelocity();
		PlayAnimMontage(FMath::Abs(Vel.X + Vel.Y) > 5 ? CurrentWeapon->CharacterReloadMovingMontage : CurrentWeapon->CharacterReloadMontage);
	}
	
}

EWallWalkType AThirdPersonPlayerCharacter::IsNextToWalkableWall(FHitResult& OutHitResult) const
{
	const FVector Start = GetActorLocation();
	FHitResult FirstHitResult;
	FCollisionQueryParams CollisionParams = FCollisionQueryParams();
	CollisionParams.AddIgnoredActor(this);
	if (CurrentWeapon)
		CollisionParams.AddIgnoredActor(CurrentWeapon);
	if (PistolWeapon)
		CollisionParams.AddIgnoredActor(PistolWeapon);
	// CameraComp->GetRightVector()
	bool bIsHit = GetWorld()->SweepSingleByChannel(FirstHitResult,
		Start,
		Start + GetActorRightVector() * ClimbableRangeCheck,
		FQuat::Identity,
		ECollisionChannel::ECC_WorldStatic,
		FCollisionShape::MakeCapsule(GetCapsuleRadius(), GetCapsuleHalfHeight()), CollisionParams);
	if (bIsHit) {
		OutHitResult = FirstHitResult;
		return EWallWalkType::RIGHT_WALL;
	}
	// Left Check.
	bIsHit = GetWorld()->SweepSingleByChannel(FirstHitResult,
		Start,
		Start - GetActorRightVector() * ClimbableRangeCheck,
		FQuat::Identity,
		ECollisionChannel::ECC_WorldStatic,
		FCollisionShape::MakeCapsule(GetCapsuleRadius(), GetCapsuleHalfHeight()), CollisionParams);
	if (bIsHit) {
		OutHitResult = FirstHitResult;
		return EWallWalkType::LEFT_WALL;
	}
	// TODO: Maybe forward as well? But check with viewpoint first.
	return EWallWalkType::NO_WALL;
}

void AThirdPersonPlayerCharacter::EnableStopGravityShootingInAir()
{
	ShotsDoneGravity = 0;
}

void AThirdPersonPlayerCharacter::StartWalkingOnWall(UCharacterMovementComponent* CharMovement)
{
	// bUseControllerRotationYaw = true;
	ResetMovementVelocity(true);
	CharMovement->GravityScale = WallWalkingGravity;
	CharMovement->MaxWalkSpeed = WallWalkingSpeed;
	TargetMovementSpeed = WallWalkingSpeed;
	TargetSpringArmOffset = SpringArmOffsetWallWalking;
	SpringArmLengthTarget = SpringArmLengthWallWalking;
}


FVector AThirdPersonPlayerCharacter::GetMovementDirectionOnWallWalking(const FVector InWallNormal) const
{
	return InWallNormal.RotateAngleAxis(90.f * (CurrentWallWalkSide == EWallWalkType::LEFT_WALL ? -1.f : 1.f), FVector(0, 0, 1));
}

void AThirdPersonPlayerCharacter::HandleWallWalk()
{
	FHitResult GroundCheckHitResult;
	const FVector Start = GetActorLocation() - FVector(0,0, GetCapsuleHalfHeight());
	FCollisionQueryParams CollisionParams = FCollisionQueryParams();
	CollisionParams.AddIgnoredActor(this);
	if (CurrentWeapon)
		CollisionParams.AddIgnoredActor(CurrentWeapon);
	if (PistolWeapon)
		CollisionParams.AddIgnoredActor(PistolWeapon);
	if (GetWorld()->LineTraceSingleByChannel(GroundCheckHitResult, Start, Start - FVector(0, 0, 20), ECC_WorldStatic, CollisionParams)) {
		// OnGround
		StopWalkingOnWall(GetCharacterMovement());
		return;
	}
	FHitResult HitResult;
	CurrentWallWalkSide = IsNextToWalkableWall(HitResult);
	if (CurrentWallWalkSide == EWallWalkType::NO_WALL) {
		StopWalkingOnWall(GetCharacterMovement());
		return;
	}
	WallNormal = FVector(HitResult.ImpactNormal.X, HitResult.ImpactNormal.Y, 0);
	
	switch (CurrentWallWalkSide)
		{
		case EWallWalkType::RIGHT_WALL:
			AddActorWorldOffset(GetActorRightVector() * WallWalkingStickToWallForce, true);
			break;
		case EWallWalkType::LEFT_WALL:
			AddActorWorldOffset(-GetActorRightVector() * WallWalkingStickToWallForce, true);
			break;
		default:
			return;
			break;
	}
	
}

void AThirdPersonPlayerCharacter::WallWalk_JumpEvent(const bool bIsRight)
{
	ResetMovementVelocity(true);
	// EWallWalkType WallSide = CurrentWallWalkSide;
	LaunchCharacter(FVector(0, 0, 1000.f) + (GetActorRightVector() * (bIsRight ? 3000.f : -3000.f)), true, true);
	StopWalkingOnWall(GetCharacterMovement());
}

void AThirdPersonPlayerCharacter::JumpToOtherSideFromWallWalking(UCharacterMovementComponent* CharMovement)
{
	
	PlayAnimMontage(CurrentWallWalkSide == EWallWalkType::RIGHT_WALL ? WallWalk_JumpFromRightWallMontage : WallWalk_JumpFromLeftWallMontage);
	
	// AddMovementInput(CurrentWallWalkSide == EWallWalkType::RIGHT_WALL ? -GetActorRightVector() : GetActorRightVector(), 8000.f, false);
}
void AThirdPersonPlayerCharacter::StopWalkingOnWall_Event(EWallWalkType WallSide) {
	PlayAnimMontage(WallSide == EWallWalkType::RIGHT_WALL ? WallWalk_JumpFromRightWallMontage : WallWalk_JumpFromLeftWallMontage);
}

void AThirdPersonPlayerCharacter::StopWalkingOnWall(UCharacterMovementComponent* CharMovement, const float NewGravityScale)
{
	CharMovement->GravityScale = NewGravityScale;
	// CharMovement->MaxWalkSpeed = WalkSpeed;
	CurrentWallWalkSide = EWallWalkType::NO_WALL;
	TargetMovementSpeed = WalkSpeed;
	// bUseControllerRotationYaw = false;
	TargetSpringArmOffset = SpringArmOffsetInit;
	SpringArmLengthTarget = SpringArmLengthInital;
}

void AThirdPersonPlayerCharacter::EquipWeapon()
{
	PlayAnimMontage(CurrentWeapon->EquipMontage);
	// WE DO THIS WHEN ANIMATION IS ENDED bHasWeaponEquipped = true;
}

void AThirdPersonPlayerCharacter::UnequipWeapon()
{
	PlayAnimMontage(CurrentWeapon->UnEquipMontage);
	// WE DO THIS WHEN ANIMATION IS ENDED bHasWeaponEquipped = false;
}

void AThirdPersonPlayerCharacter::EquipAttachWeaponToHand(bool bEquip) {
	if (!CurrentWeapon) return;
	CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, bEquip ? WeaponHandAttachmentSocketName : WeaponAttachmentSocketName);
}

void AThirdPersonPlayerCharacter::FinishUnequipWeapon()
{
	bHasWeaponEquipped = false;
	bIsAimingWeapon = false;
}

void AThirdPersonPlayerCharacter::FinishEquipWeapon()
{
	
	bHasWeaponEquipped = true;
}

void AThirdPersonPlayerCharacter::FinishThrowGrenade(bool bWasAiming)
{
	CharacterGrenadeHandlerComp->ThrowGrenade(bWasAiming);
}

UAnimMontage* AThirdPersonPlayerCharacter::GetReloadMontageBasedOnWeapon(bool bIsAiming) const
{
	/*if (CurrentWeapon->WeaponName.IsEqual(FName("AK47")))
		return ReloadAk47AnimMontage;
	if (CurrentWeapon->WeaponName.IsEqual(FName("PISTOL")))
		return ReloadPistolAnimMontage;*/
	if (CurrentWeapon == nullptr) return nullptr;
	return bIsAiming ? CurrentWeapon->ReloadADSMontage : CurrentWeapon->ReloadMontage;
}

UAnimMontage* AThirdPersonPlayerCharacter::GetShootMontageBasedOnWeapon() const
{
	/*if (CurrentWeapon->WeaponName.IsEqual(FName("AK47")))
		return ShootAk47AnimMontage;
	if (CurrentWeapon->WeaponName.IsEqual(FName("PISTOL")))
		return ShootPistolAnimMontage;*/
	if (CurrentWeapon == nullptr) return nullptr;
	return CurrentWeapon->ShootMontage;
}

UAnimationAsset* AThirdPersonPlayerCharacter::GetCurrentAimOffset() const
{
	if (CurrentWeapon == nullptr || !bHasWeaponEquipped) return nullptr;
	return bIsAimingWeapon ? CurrentWeapon->AIM_AimOffset : CurrentWeapon->HIP_AimOffset;
}

void AThirdPersonPlayerCharacter::ApplyEpicEffect(float TimeDilationAmount, FVector Location, float Duration, bool bIsAttached, bool bPlayNiagara, bool bSlowDownPlayer, float PlayerSlowdownCustomRate)
{
	if (GetWorldTimerManager().IsTimerActive(EpicEffectTimerHandle)) return;
	if (bPlayNiagara)
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), SlowMotionNiagaraEffect, Location);
	const float PreviousDilation = UGameplayStatics::GetGlobalTimeDilation(GetWorld());
	if (PreviousDilation > TimeDilationAmount) {
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), TimeDilationAmount);
		FTimerDelegate MyDelegate;
		if (!bSlowDownPlayer) {
			CustomTimeDilation = 1 / TimeDilationAmount;
			PlayerCameraManager->CustomTimeDilation = CustomTimeDilation;
			MyDelegate.BindLambda([&]() {
				UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.f);
				CustomTimeDilation = 1.f;
				PlayerCameraManager->CustomTimeDilation = 1;
			});
		}
		else {
			CustomTimeDilation = 1 / TimeDilationAmount * PlayerSlowdownCustomRate;
			PlayerCameraManager->CustomTimeDilation = CustomTimeDilation;
			MyDelegate.BindLambda([&]() {
				UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.f);
				CustomTimeDilation = 1.f;
				PlayerCameraManager->CustomTimeDilation = 1;
			});
		}
		GetWorldTimerManager().SetTimer(EpicEffectTimerHandle, MyDelegate, Duration, false);
	}
}

void AThirdPersonPlayerCharacter::ApplyEpicEffectWithInterpolation(float TimeDilationAmount, float Duration, float RecoverySpeed, FVector Location, class UNiagaraSystem* NiagaraToPlay, bool bSlowDownPlayer, float PlayerSlowdownCustomRate, bool bIsInstant)
{
	if (GetWorldTimerManager().IsTimerActive(EpicEffectTimerHandle)) return;
	if (NiagaraToPlay)
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), NiagaraToPlay, Location);
	if (!bIsInstant) {
		// Interpolation mode.
		bIsInterpolatingTimeDilation = true;
		TimeDilationRecoverySpeed = RecoverySpeed;
		TimeDilationInterpSpeed = Duration;
		TargetWorldTimeDilation = TimeDilationAmount;
		TargetPlayerTimeDilation = 1 / TimeDilationAmount * PlayerSlowdownCustomRate;
		return;
	}
	// Instant Mode.
	ApplyEpicEffect(TimeDilationAmount, Location, Duration, false, false, bSlowDownPlayer, PlayerSlowdownCustomRate);

}

void AThirdPersonPlayerCharacter::HandleTimeDilationInterp(const float& DeltaTime) {
	if (!bIsInterpolatingTimeDilation) return;
	float NewWorldTimeDilation = FMath::FInterpTo(UGameplayStatics::GetGlobalTimeDilation(GetWorld()),
		TargetWorldTimeDilation, DeltaTime, TimeDilationInterpSpeed);
	if (FMath::Abs(NewWorldTimeDilation - TargetWorldTimeDilation) <= 0.001f) {
		NewWorldTimeDilation = TargetWorldTimeDilation;
		if (TargetWorldTimeDilation == 1.f)
			bIsInterpolatingTimeDilation = false;  // Disabling the interpolation.
		else
			TargetWorldTimeDilation = 1.f, TargetPlayerTimeDilation = 1.f, TimeDilationInterpSpeed = TimeDilationRecoverySpeed;
	}
	else
		TimeDilationInterpSpeed += FMath::Sign(TimeDilationInterpSpeed) * TimeDilationAcceleration;
	float NewMyTimeDilation = bIsInterpolatingTimeDilation ? FMath::FInterpTo(CustomTimeDilation, TargetPlayerTimeDilation, DeltaTime, TimeDilationInterpSpeed) : 1.f;
	if (bIsInterpolatingTimeDilation && FMath::Abs(NewMyTimeDilation - TargetPlayerTimeDilation) <= 0.001f)
		NewMyTimeDilation = TargetPlayerTimeDilation;

	// UE_LOG(LogTemp, Warning, TEXT("World: %f, Player: %f"), NewWorldTimeDilation, NewMyTimeDilation);
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), NewWorldTimeDilation);
	CustomTimeDilation = NewMyTimeDilation;

}


void AThirdPersonPlayerCharacter::PickupWeapon(AWeapon* WeaponToPickup, bool bEquip)
{
	WeaponToPickup->SetOwner(this);
	WeaponToPickup->SetInstigator(this);
	if (bEquip)
		CurrentWeapon = WeaponToPickup;
	WeaponToPickup->PickedUpWeapon();
	InventoryComponent->AddItemToInventory(WeaponToPickup);
}

void AThirdPersonPlayerCharacter::DropWeapon(AWeapon* WeaponToDrop) {
	InventoryComponent->DropItem(WeaponToDrop);
	WeaponToDrop->SetOwner(nullptr);
	WeaponToDrop->SetInstigator(nullptr);
	WeaponToDrop->DroppedWeapon();
	if (CurrentWeapon == WeaponToDrop) CurrentWeapon = nullptr;  // TODO: For other weapons (multiple weapons -> equip the next weapon instead).
}

void AThirdPersonPlayerCharacter::StopInteract()
{
	if (!InteractingObject) return;
	InteractingObject->StopInteraction();
	InteractingObject = nullptr;
}

void AThirdPersonPlayerCharacter::PlayCameraShake(TSubclassOf<UCameraShakeBase> CameraShakeToPlay, bool bStopAllShakes)
{
	if (CameraShakeToPlay) {
		if (bStopAllShakes)
			PlayerCameraManager->StopAllCameraShakes();
		PlayerCameraManager->StartCameraShake(CameraShakeToPlay);
	}
		
}

void AThirdPersonPlayerCharacter::InteractionRangeOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndexbool, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AInteractableObject* Interactable = Cast<AInteractableObject>(OtherActor)) {
		Interactable->SetInRange(true);
	}
	
}

void AThirdPersonPlayerCharacter::InteractionRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndexbool)
{
	if (AInteractableObject* Interactable = Cast<AInteractableObject>(OtherActor)) {
		Interactable->SetInRange(false);
		Interactable->SetIsFocused(false);
		Interactable->StopInteraction();
		if (Interactable == InteractingObject) InteractingObject = nullptr;
	}
}

AInteractableObject* AThirdPersonPlayerCharacter::GetClosestInteractable() const
{
	TArray<AActor*> Overlaps;
	InteractOverlapComp->GetOverlappingActors(Overlaps, AInteractableObject::StaticClass());
	if (Overlaps.IsEmpty()) return nullptr;
	AInteractableObject* ClosestInteractable = Cast<AInteractableObject>(Overlaps[0]);
	float SmallestDistance = 8000.f;
	const FVector PlayerLocation = GetActorLocation();
	for (int i = 0; i < Overlaps.Num(); i++) {
		AInteractableObject* Interactable = Cast<AInteractableObject>(Overlaps[i]);
		if (!Interactable || IsInteractableBlocked(Interactable) || !Interactable->bIsInteractable) continue;
		const float NewDistance = FVector::Dist(PlayerLocation, Interactable->GetActorLocation());
		if (NewDistance < SmallestDistance) {
			ClosestInteractable = Interactable;
			SmallestDistance = NewDistance;
		}
	}
	return SmallestDistance > InteractDistance ? nullptr : ClosestInteractable;
}

bool AThirdPersonPlayerCharacter::IsInteractableBlocked(AInteractableObject* Interactable) const
{
	FHitResult HitResult;
	const FVector Start = GetActorLocation();
	const FVector End = Interactable->GetActorLocation();
	FCollisionQueryParams CParams;
	CParams.AddIgnoredActor(this);
	if (CurrentWeapon)
		CParams.AddIgnoredActor(CurrentWeapon);
	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility, CParams)) {
		AActor* HitResultActor = HitResult.GetActor();
		if (HitResultActor == nullptr || HitResultActor == Interactable) return false;
		return true;
	}
	return false;
}

bool AThirdPersonPlayerCharacter::IsInteracting() const
{
	if (!InteractingObject) return false;
	return InteractingObject->bIsInteracting;
}

void AThirdPersonPlayerCharacter::HandleInteract(float DeltaTime)
{
	AInteractableObject* ClosestInteractable = GetClosestInteractable();
	if (!ClosestInteractable && InteractingObject != nullptr) {
		InteractingObject->SetIsFocused(false);
		InteractingObject->StopInteraction();
		InteractingObject = nullptr;
		return;
	}
	if (ClosestInteractable != InteractingObject) {
		if (InteractingObject) {
			InteractingObject->SetIsFocused(false);
			InteractingObject->StopInteraction();
		}
		InteractingObject = ClosestInteractable;
		InteractingObject->SetIsFocused(true);
	}
	if (InteractingObject != nullptr) {
		InteractingObject->HoldInteract(this, DeltaTime);
	}
}

void AThirdPersonPlayerCharacter::RefillAmmo(AWeapon* WeaponToFill, int Amount)
{
	if (WeaponToFill == nullptr || Amount < 0) {
		for (AWeapon* WeaponInInvetory : InventoryComponent->InventoryWeapons)
			WeaponInInvetory->RefillAmmo(Amount);
		return;
	}
	WeaponToFill->RefillAmmo(Amount);
}

void AThirdPersonPlayerCharacter::SetCutsceneController(ACutsceneController* NewCutsceneController)
{
	PlayerCutsceneHandler->CurrentCutsceneController = NewCutsceneController;
}

bool AThirdPersonPlayerCharacter::IsInCutscene() const
{
	return PlayerCutsceneHandler->IsInCutscene();
}

void AThirdPersonPlayerCharacter::FinishedCutscene(FTransform NewPlayerTransform, bool bTeleport, bool bFailedCutscene)
{
	PlayerCutsceneHandler->FinishedCutscene(NewPlayerTransform, bTeleport, bFailedCutscene);
	OnFinishedCutscene.Broadcast(bFailedCutscene);
}

void AThirdPersonPlayerCharacter::SetInCutsceneTrigger(ACutsceneTrigger* NewCutsceneTrigger, TSubclassOf<class UUserWidget> WidgetClass)
{
	PlayerCutsceneHandler->SetInCutsceneTrigger(NewCutsceneTrigger);
	if (NewCutsceneTrigger == nullptr) {
		ClearWidget();
	}
	else {
		if (WidgetClass == nullptr) {
			UE_LOG(LogTemp, Warning, TEXT("The following CutsceneTrigger has no WidgetClass assigned to it but has a starting action trigger: %s"), *NewCutsceneTrigger->GetFullName());
			return;
		}
		CreateWidgetAndShow(WidgetClass);
	}
}

bool AThirdPersonPlayerCharacter::IsInCutsceneTrigger() const
{
	return PlayerCutsceneHandler->IsInCutsceneTrigger();
}

UUserWidget* AThirdPersonPlayerCharacter::GetCurrentWidgetOnScreen() const
{
	if (WidgetsOnScreen.IsEmpty() || !WidgetsOnScreen.IsValidIndex(0)) return nullptr;
	return WidgetsOnScreen.Last();
}

void AThirdPersonPlayerCharacter::CreateWidgetAndShow(TSubclassOf<class UUserWidget> WidgetClass)
{
	UUserWidget* CreatedWidget = CreateWidget<UUserWidget>(MyPlayerController, WidgetClass);
	// if (CreatedWidget == nullptr) return;
	WidgetsOnScreen.Add(CreatedWidget);
	CreatedWidget->AddToViewport();
}

void AThirdPersonPlayerCharacter::ClearWidget()
{
	UUserWidget* CurrentWidget = GetCurrentWidgetOnScreen();
	if (CurrentWidget == nullptr) return;
	CurrentWidget->RemoveFromParent();
	WidgetsOnScreen.RemoveAt(WidgetsOnScreen.Num() - 1);
	CurrentWidget = nullptr;
}

bool AThirdPersonPlayerCharacter::IsPlayerActionAndActionRequiredEqual(const UInputAction* Action) const
{
	return PlayerCutsceneHandler->IsPlayerActionAndActionRequiredEqual(Action);
}

void AThirdPersonPlayerCharacter::ShieldParryAction(const FInputActionInstance& ActionInstance) {
	if (!MyPlayerController->InputEnabled() || bIsFallingStompingGround || !bCanMelee) return;
	FHitResult HitResult;
	AEnemyBaseCharacter* EnemyCharacterInFront = IsFacingEnemy(HitResult, EnemyNearDistanceCheck, EnemyNearRadiusCheck);
	// If no enemy is in front of player, we just play the animation for the melee.
	if (!EnemyCharacterInFront) {
		EquipShield_Start();
		// PlayAnimMontage(MeleeAnimMontage);
		return;
	}
	EnemyCharacterMeleeing = EnemyCharacterInFront;
	if (EnemyCharacterInFront->CanPlayerParryMyAttack()) {
		FaceRotation((EnemyCharacterInFront->GetActorLocation() - GetActorLocation()).GetSafeNormal().Rotation());
		Parry();
	}
	else {
		//TeleportToEnemyCharacter(EnemyCharacterInFront);
		//Melee();
		EquipShield_Start();
	}
}

void AThirdPersonPlayerCharacter::EquipShield_Start()
{
	bIsShieldEquipped = true;
}

void AThirdPersonPlayerCharacter::Shield_Ongoing()
{
	if (!bIsShieldEquipped) return;
}

void AThirdPersonPlayerCharacter::Shield_Completed()
{
	bIsShieldEquipped = false;
	// if (!bIsShieldEquipped) return;
}

void AThirdPersonPlayerCharacter::Melee()
{
	if (EnemyCharacterMeleeing == nullptr) return;
	bCanMelee = false;
	bCanMove = false;
	bCanEvade = false;
	CurrentWeapon->BulletsToAddAfterReloadComplete = 0;
	PlayAnimMontage(MeleeAnimMontage);
	EnemyCharacterMeleeing->PlayerStartMeleeMe();
}

void AThirdPersonPlayerCharacter::Parry()
{
	if (EnemyCharacterMeleeing == nullptr) return;
	bCanMelee = false;
	bCanMove = false;
	bCanEvade = false;
	CurrentWeapon->BulletsToAddAfterReloadComplete = 0;
	PlayAnimMontage(ParryAnimMontage);
	EnemyCharacterMeleeing->PlayerStartParryMe();
}

void AThirdPersonPlayerCharacter::TeleportToEnemyCharacter(AEnemyBaseCharacter* EnemyCharacterToTeleportTo)
{
	const FVector TeleportLocation = EnemyCharacterToTeleportTo->GetActorLocation();
	const FVector DirectionToMe = (GetActorLocation() - TeleportLocation).GetSafeNormal();
	FRotator TeleportRotation = (TeleportLocation - GetActorLocation()).GetSafeNormal().Rotation();
	TeleportRotation.Pitch = 0;
	TeleportRotation.Roll = 0;
	ResetMovementVelocity(true);
	TeleportTo(TeleportLocation + DirectionToMe * MeleeDistanceToEnemy, TeleportRotation);
	FaceRotation(TeleportRotation);
	TeleportRotation = DirectionToMe.Rotation();
	TeleportRotation.Pitch = 0;
	TeleportRotation.Roll = 0;
	EnemyCharacterToTeleportTo->FaceRotation(TeleportRotation);
	EnemyCharacterToTeleportTo->SetActorRotation(TeleportRotation);
}

void AThirdPersonPlayerCharacter::MeleeGiveDamageEvent()
{
	if (EnemyCharacterMeleeing == nullptr) return;
	EnemyCharacterMeleeing->PlayerMeleeEvent(MeleeDamage);
}

void AThirdPersonPlayerCharacter::ParryEvent()
{
	if (EnemyCharacterMeleeing == nullptr) return;
	// EnemyCharacterMeleeing->PlayerParryEvent(ParryDamage);
	TArray<FHitResult> HitResults;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	if (PistolWeapon)
		CollisionParams.AddIgnoredActor(PistolWeapon);
	if (CurrentWeapon)
		CollisionParams.AddIgnoredActor(CurrentWeapon);
	const bool bIsHit = GetWorld()->SweepMultiByChannel(HitResults, GetActorLocation(), GetActorLocation(), FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(StunSphereRadius), CollisionParams);
	if (bIsHit) {
		for (const FHitResult& HitResult : HitResults) {
			if (AEnemyBaseCharacter* EnemyCharacterInStunRadius = Cast<AEnemyBaseCharacter>(HitResult.GetActor()))
				EnemyCharacterInStunRadius->PlayerParryEvent(ParryDamage);
		}
	}
	else
		EnemyCharacterMeleeing->PlayerParryEvent(ParryDamage);
	ApplyEpicEffectWithInterpolation(0.1f, -1.8f, 0.3f, FVector::ZeroVector, nullptr, true, 0.3f);
	// ApplyEpicEffect(0.2f, FVector::ZeroVector, 0.6f, false, false, true, 0.3f);
}

void AThirdPersonPlayerCharacter::MeleeFinished()
{
	bCanMelee = true;
	bCanMove = true;
	bCanEvade = true;
	if (EnemyCharacterMeleeing == nullptr) return;
	EnemyCharacterMeleeing->PlayerMeleeFinished();
	EnemyCharacterMeleeing = nullptr;
}

void AThirdPersonPlayerCharacter::ParryFinished()
{
	bCanMelee = true;
	bCanMove = true;
	bCanEvade = true;
	if (EnemyCharacterMeleeing == nullptr) return;
	EnemyCharacterMeleeing->PlayerParryFinished();
	EnemyCharacterMeleeing = nullptr;
}

AEnemyBaseCharacter* AThirdPersonPlayerCharacter::IsFacingEnemy(FHitResult& OutHitResult, const float DistanceCheck, const float RadiusCheck) const
{
	const FVector Start = GetActorLocation();
	const FVector End = Start + (GetActorForwardVector() * DistanceCheck);
	FCollisionQueryParams CollisionParams = FCollisionQueryParams();
	CollisionParams.AddIgnoredActor(this);
	if (PistolWeapon)
		CollisionParams.AddIgnoredActor(PistolWeapon);
	if (CurrentWeapon)
		CollisionParams.AddIgnoredActor(CurrentWeapon);
	const bool bIsHit = GetWorld()->SweepSingleByChannel(OutHitResult,
		Start,
		End,
		FQuat::Identity,
		ECollisionChannel::ECC_Pawn,
		FCollisionShape::MakeSphere(RadiusCheck), CollisionParams);
	// DrawDebugSphere(GetWorld(), Start, 20.f, 10, FColor::Red, false, 3.f);
	if (bIsHit) {
		UE_LOG(LogTemp, Warning, TEXT("Facing: %s"), *OutHitResult.GetActor()->GetFullName());
		if (AEnemyBaseCharacter* EnemyCharacterInFront = Cast<AEnemyBaseCharacter>(OutHitResult.GetActor()))
			return EnemyCharacterInFront;
	}
	return nullptr;
}
