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

AThirdPersonPlayerCharacter::AThirdPersonPlayerCharacter()
{
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
	PlayerEnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Triggered, this, &AThirdPersonPlayerCharacter::ShootWeapon);
	PlayerEnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &AThirdPersonPlayerCharacter::Sprint);
	PlayerEnhancedInputComponent->BindAction(ClickerAction, ETriggerEvent::Triggered, this, &AThirdPersonPlayerCharacter::ClickerClick);
	PlayerEnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &AThirdPersonPlayerCharacter::Reload);
	PlayerEnhancedInputComponent->BindAction(EvadeAction, ETriggerEvent::Triggered, this, &AThirdPersonPlayerCharacter::Evade);
	PlayerEnhancedInputComponent->BindAction(MeleeAction, ETriggerEvent::Triggered, this, &AThirdPersonPlayerCharacter::Melee);
	PlayerEnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &AThirdPersonPlayerCharacter::Interact);
	PlayerEnhancedInputComponent->BindAction(DroneDeployAction, ETriggerEvent::Triggered, this, &AThirdPersonPlayerCharacter::DeployDrone);

	PlayerEnhancedInputComponent->BindAction(GrenadeAction, ETriggerEvent::Started, this, &AThirdPersonPlayerCharacter::StartGrenade);
	PlayerEnhancedInputComponent->BindAction(GrenadeAction, ETriggerEvent::Triggered, this, &AThirdPersonPlayerCharacter::AimGrenade);
	PlayerEnhancedInputComponent->BindAction(GrenadeAction, ETriggerEvent::Completed, this, &AThirdPersonPlayerCharacter::ThrowGrenade);
}

void AThirdPersonPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	

	CharacterGrenadeHandlerComp = FindComponentByClass<UCharacterGrenadeHandler>();
	SpringArmComp = FindComponentByClass<USpringArmComponent>();
	CameraComp = FindComponentByClass<UCameraComponent>();
	WarpHandlerComponent = FindComponentByClass<UWarpCheckerComponent>();
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	bIsAimingWeapon = false;
	SpringArmLengthInital = SpringArmComp->TargetArmLength;
	SpringArmLengthTarget = SpringArmLengthInital;
	MyPlayerController = GetLocalViewingPlayerController();
	if (!MyPlayerController) {
		SetActorTickEnabled(false);
		return;
	}
	PlayerCameraManager = MyPlayerController->PlayerCameraManager;
	PlayerCutsceneHandler = FindComponentByClass<UPlayerCutsceneHandlerComponent>();
	WidgetsOnScreen.Empty();
}

void AThirdPersonPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SpringArmComp->TargetArmLength = FMath::FInterpTo(SpringArmComp->TargetArmLength, SpringArmLengthTarget, DeltaTime, SpringArmLengthTransitionRate);
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

void AThirdPersonPlayerCharacter::Look(const FInputActionInstance& ActionInstance)
{
	if (!MyPlayerController->InputEnabled()) return;
	UCharacterMovementComponent* CharMovement = GetCharacterMovement();
	if (CharMovement->MovementMode == EMovementMode::MOVE_None) return;
	FVector2D Axis = ActionInstance.GetValue().Get<FVector2D>() * MouseSensivity;
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void AThirdPersonPlayerCharacter::Move(const FInputActionInstance& ActionInstance)
{
	if (!MyPlayerController->InputEnabled()) return;
	UCharacterMovementComponent* CharMovement = GetCharacterMovement();
	if (CharMovement->MovementMode == EMovementMode::MOVE_None) return;
	MovementDirection = ActionInstance.GetValue().Get<FVector2D>() * GetCurrentMovementSpeed();
	AddMovementInput(GetActorForwardVector(), MovementDirection.Y);
	AddMovementInput(GetActorRightVector(), MovementDirection.X);
	PlayerCameraManager->StartCameraShake(WalkingCameraShake);
	
}

void AThirdPersonPlayerCharacter::Evade(const FInputActionInstance& ActionInstance)
{
	if (!bCanEvade || !MyPlayerController->InputEnabled()) {
		UE_LOG(LogTemp, Warning, TEXT("CANNOT"));
		return;
	}
	bCanEvade = false;
	// ChangePlayerMovement(EMovementMode::MOVE_Flying);
	// StopAnimMontage(CurrentWeapon->EquipMontage);
	// StopAnimMontage(CurrentWeapon->UnEquipMontage);
	if (MovementDirection.Y < 0)
		PlayAnimMontage(Evade_BwAnimMontage);
	else if (MovementDirection.Y > 0)
		PlayAnimMontage(Evade_FwAnimMontage);
	else if (MovementDirection.X > 0)
		PlayAnimMontage(Evade_RAnimMontage);
	else if (MovementDirection.X < 0)
		PlayAnimMontage(Evade_LAnimMontage);

	FTimerDelegate MyDelegate;
	MyDelegate.BindLambda([&]() {
		bCanEvade = true;
	});
	GetWorldTimerManager().SetTimer(EvadeTimerHandle, MyDelegate, 0.5f, false);
	
}

void AThirdPersonPlayerCharacter::Melee(const FInputActionInstance& ActionInstance)
{
	if (!MyPlayerController->InputEnabled()) return;
	PlayAnimMontage(Melee_AnimMontage);
}

void AThirdPersonPlayerCharacter::Interact(const FInputActionInstance& ActionInstance)
{
	if (!MyPlayerController->InputEnabled()) return;
	// TODO: Check for normal interaction

	const bool bResult = WarpHandlerComponent->SwapLocation();
}

void AThirdPersonPlayerCharacter::Sprint(const FInputActionInstance& ActionInstance)
{
	const bool bValue = ActionInstance.GetValue().Get<bool>();
	UCharacterMovementComponent* CharMovement = GetCharacterMovement();
	if (CharMovement->MovementMode == EMovementMode::MOVE_None) return;
	if (!MyPlayerController->InputEnabled()) {
		if (!bValue)
			GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		return;
	}
	if (bValue && bIsAimingWeapon) {
		// StopAimimg();
		return;
	}
	GetCharacterMovement()->MaxWalkSpeed = bValue ? SprintSpeed : WalkSpeed;
}

void AThirdPersonPlayerCharacter::Jump()
{
	if (!MyPlayerController->InputEnabled()) return;
	bIsAimingGrenade = false;
	if (IsInCutsceneTrigger() && IsPlayerActionAndActionRequiredEqual(JumpAction)) {
		PlayerCutsceneHandler->StartCutscene();
		return;
	}
	UCharacterMovementComponent* CharMovement = GetCharacterMovement();
	

	// if (CharMovement->IsFalling() || CharMovement->MovementMode == EMovementMode::MOVE_None) return;
	if (CharMovement->MovementMode == EMovementMode::MOVE_None) return;
	// TODO: Collision Check for a climbable object
	FHitResult HitResult;
	const EClimbType bResult = IsFacingClimbableObject(HitResult);
	// UE_LOG(LogTemp, Warning, TEXT("Is facing climbable: %d"), bResult ? 1 : 0);
	if (bResult == EClimbType::NOT_CLIMBABLE || bResult == EClimbType::TOO_TALL_NOT_CLIMBABLE) {
		// bCanEvade = true;
		if (CharMovement->IsFalling()) return;
		Roll();
		return;
	}
	if (CurrentWeapon && bHasWeaponEquipped) {
		EquipAttachWeaponToHand(false);
		FinishUnequipWeapon();
	}

	FVector FeetLocation = GetActorLocation();
	FeetLocation.Z -= GetCapsuleHalfHeight();
	FVector HitLoc = HitResult.ImpactPoint;
	HitLoc.Z = GetActorLocation().Z;
	// SetActorRotation((HitLoc - GetActorLocation()).Rotation());
	
	if (bResult == EClimbType::NORMAL_CLIMB) {
		FVector Difference = HitResult.ImpactPoint - FeetLocation;
		FVector Final = GetActorLocation() + Difference - (GetActorForwardVector() * 90);
		SetActorLocation(Final);
		PlayAnimMontage(NormalClimbAnimMontage);
		ResetMovementVelocity(true);
		bCanEvade = true;
	}
	else if (bResult == EClimbType::LEDGE_CLIMB) {
		ResetMovementVelocity(true);
		FVector Difference = HitResult.ImpactPoint - GetActorLocation();
		FVector Final = GetActorLocation() + Difference - (GetActorUpVector() * 70) - (GetActorForwardVector() * 30);
		CharMovement->SetMovementMode(EMovementMode::MOVE_Flying);
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		MyPlayerController->DisableInput(MyPlayerController);
		SetActorLocation(Final);
		/*
		FTimerDelegate MyDelegate;
		MyDelegate.BindLambda([&]() {
			GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
			GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			MyPlayerController->EnableInput(MyPlayerController);
			bCanEvade = true;
		});
		GetWorldTimerManager().SetTimer(TimerHandle, MyDelegate, 1.7f, false);
		*/
		
		PlayAnimMontage(LedgeClimbAnimMontage);
		bCanEvade = true;
	}
	
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
		Start = SecondHitResult.ImpactPoint + (GetActorUpVector() * (CapsuleHalfHeight+30));
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
		for (int i = 0; i < Start.Z; i+=2)
		{
			StartA.Z += i;
			PrevHitResult = SecondHitResult;
			// DrawDebugSphere(GetWorld(), StartA, 5, 10, FColor::Red, true);
			bIsHit = GetWorld()->SweepSingleByChannel(SecondHitResult,
				StartA,
				StartA,
				FQuat::Identity,
				ECollisionChannel::ECC_WorldStatic,
				FCollisionShape::MakeSphere(2), CollisionParams);
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
	for (int i = 0; i < Start.Z; i += 2)
	{
		StartA.Z += i;
		PrevHitResult = FirstHitResult;
		// DrawDebugSphere(GetWorld(), StartA, 5, 10, FColor::Red, true);
		bIsHit = GetWorld()->SweepSingleByChannel(FirstHitResult,
			StartA,
			StartA,
			FQuat::Identity,
			ECollisionChannel::ECC_WorldStatic,
			FCollisionShape::MakeSphere(2), CollisionParams);
		if (!bIsHit) break;
	}
	UE_LOG(LogTemp, Warning, TEXT("Normal Climb"));
	OutHitResult = PrevHitResult;
	return EClimbType::NORMAL_CLIMB;
}

void AThirdPersonPlayerCharacter::Roll()
{
	MyPlayerController->DisableInput(MyPlayerController);
	if (MovementDirection.X > 0)
		PlayAnimMontage(Roll_RAnimMontage);
	else if (MovementDirection.X < 0)
		PlayAnimMontage(Roll_LAnimMontage);
	else if (MovementDirection.Y < 0)
		PlayAnimMontage(Roll_BwdAnimMontage);
	else
		PlayAnimMontage(Roll_FwAnimMontage);
	/*
	FTimerDelegate MyDelegate;
	MyDelegate.BindLambda([&]() {
		MyPlayerController->EnableInput(MyPlayerController);
	});
	GetWorldTimerManager().SetTimer(TimerHandle, MyDelegate, 0.6f, false);
	*/
}

void AThirdPersonPlayerCharacter::StopAimimg()
{
	bIsAimingWeapon = false;
	SpringArmLengthTarget = SpringArmLengthInital;
}

void AThirdPersonPlayerCharacter::AimWeapon(const FInputActionInstance& ActionInstance)
{
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
	bIsAimingWeapon = bValue;
	SpringArmLengthTarget = (bIsAimingWeapon ? SpringArmLengthAimingMult : 1) * SpringArmLengthInital;
	if (bIsAimingWeapon)
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void AThirdPersonPlayerCharacter::ShootWeapon(const FInputActionInstance& ActionInstance)
{
	if (!MyPlayerController->InputEnabled()) return;
	bIsAimingGrenade = false;
	if (!bHasWeaponEquipped) return;
	UCharacterMovementComponent* CharMovement = GetCharacterMovement();
	if (CharMovement->MovementMode == EMovementMode::MOVE_None) return;
	if (!bIsAimingWeapon && CurrentWeapon->bShouldAimToShoot) return;
	// PlayAnimMontage(ShootPistolAnimMontage);
	FVector Start;
	FRotator Rotation;
	MyPlayerController->GetPlayerViewPoint(Start, Rotation);
	FVector End = Start + (CameraComp->GetForwardVector() * 10000.f);
	if (CurrentWeapon != nullptr) CurrentWeapon->Shoot(End);
	UAnimMontage* ShootAnimMontage = GetShootMontageBasedOnWeapon();
	if (ShootAnimMontage) {
		// UE_LOG(LogTemp, Warning);
		PlayAnimMontage(ShootAnimMontage);
	}
	if (!ShootingCameraShake) return;
	PlayerCameraManager->StartCameraShake(ShootingCameraShake);
}

void AThirdPersonPlayerCharacter::ClickerClick(const FInputActionInstance& ActionInstance)
{
	if (!MyPlayerController->InputEnabled()) return;
	UCharacterMovementComponent* CharMovement = GetCharacterMovement();
	if (CharMovement->MovementMode == EMovementMode::MOVE_None) return;
}

void AThirdPersonPlayerCharacter::StartGrenade(const FInputActionInstance& ActionInstance)
{
	bIsAimingGrenade = true;
	if (CurrentWeapon != nullptr)
		CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, WeaponAttachmentSocketName);
	bHasWeaponEquipped = false;
}

void AThirdPersonPlayerCharacter::AimGrenade(const FInputActionInstance& ActionInstance)
{
	CharacterGrenadeHandlerComp->OngoingGrenadeAim(bIsAimingWeapon);
}

void AThirdPersonPlayerCharacter::ThrowGrenade(const FInputActionInstance& ActionInstance)
{
	StopAnimMontage();
	if (bIsAimingWeapon) {
		if (GrenadeAimThrowMontage)
			PlayAnimMontage(GrenadeAimThrowMontage);
	}
	else {
		if (GrenadeThrowMontage)
			PlayAnimMontage(GrenadeThrowMontage);
	}
	bIsAimingGrenade = false;
	// TODO: Throw on Anim Montage End
	
}

void AThirdPersonPlayerCharacter::Reload(const FInputActionInstance& ActionInstance)
{
	if (!MyPlayerController->InputEnabled() || !bHasWeaponEquipped) return;
	UCharacterMovementComponent* CharMovement = GetCharacterMovement();
	if (CharMovement->MovementMode == EMovementMode::MOVE_None) return;
	//PlayAnimMontage(bIsAimingWeapon ? ReloadPistolAimingAnimMontage : ReloadPistolAnimMontage);
	UAnimMontage* ReloadMontage = GetReloadMontageBasedOnWeapon(bIsAimingWeapon);
	if (ReloadMontage) {
		PlayAnimMontage(ReloadMontage);
	}
	
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

void AThirdPersonPlayerCharacter::ApplyEpicEffect(float TimeDilationAmount, FVector Location, float Duration, bool bPlayNiagara, bool bSlowDownPlayer)
{
	if (bPlayNiagara)
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), SlowMotionNiagaraEffect, Location);
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), TimeDilationAmount);
	FTimerDelegate MyDelegate;
	if (!bSlowDownPlayer) {
		CustomTimeDilation = 1 / TimeDilationAmount;
		MyDelegate.BindLambda([&]() {
			UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.f);
			CustomTimeDilation = 1.f;
		});
	}
	else {
		MyDelegate.BindLambda([&]() {
			UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.f);
		});
	}
	GetWorldTimerManager().SetTimer(EpicEffectTimerHandle, MyDelegate, Duration, false);
}

void AThirdPersonPlayerCharacter::PickupWeapon(AWeapon* WeaponToPickup)
{
	WeaponToPickup->SetOwner(this);
	CurrentWeapon = WeaponToPickup;
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
	return  PlayerCutsceneHandler->IsPlayerActionAndActionRequiredEqual(Action);
}
