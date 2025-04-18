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
#include "Camera/CameraModifier_CameraShake.h"

AThirdPersonPlayerCharacter::AThirdPersonPlayerCharacter()
{
	InteractOverlapComp = CreateDefaultSubobject<UBoxComponent>(FName("Interaction Box Area"));
	InteractOverlapComp->SetupAttachment(GetRootComponent());
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(FName("Inventory Comp"));
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

	PlayerEnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &AThirdPersonPlayerCharacter::Sprint);
	PlayerEnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &AThirdPersonPlayerCharacter::Reload);
	PlayerEnhancedInputComponent->BindAction(EvadeAction, ETriggerEvent::Triggered, this, &AThirdPersonPlayerCharacter::Evade);
	PlayerEnhancedInputComponent->BindAction(MeleeAction, ETriggerEvent::Triggered, this, &AThirdPersonPlayerCharacter::Melee);
	PlayerEnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &AThirdPersonPlayerCharacter::Interact);
	PlayerEnhancedInputComponent->BindAction(DroneDeployAction, ETriggerEvent::Triggered, this, &AThirdPersonPlayerCharacter::DeployDrone);

	PlayerEnhancedInputComponent->BindAction(SpecialAction, ETriggerEvent::Started, this, &AThirdPersonPlayerCharacter::SpecialStart);
	PlayerEnhancedInputComponent->BindAction(SpecialAction, ETriggerEvent::Triggered, this, &AThirdPersonPlayerCharacter::SpecialHold);
	PlayerEnhancedInputComponent->BindAction(SpecialAction, ETriggerEvent::Completed, this, &AThirdPersonPlayerCharacter::SpecialRelease);

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
	SpecialAbilityHandlerComp = FindComponentByClass<USpecialAbilityHandlerComponent>();
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

	InteractOverlapComp->OnComponentBeginOverlap.AddDynamic(this, &AThirdPersonPlayerCharacter::InteractionRangeOverlap);
	InteractOverlapComp->OnComponentEndOverlap.AddDynamic(this, &AThirdPersonPlayerCharacter::InteractionRangeEndOverlap);
}

void AThirdPersonPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SpringArmComp->TargetArmLength = FMath::FInterpTo(SpringArmComp->TargetArmLength, SpringArmLengthTarget, DeltaTime, SpringArmLengthTransitionRate);
	HandleInteract(DeltaTime);
}

void AThirdPersonPlayerCharacter::Die()
{
	SpringArmComp->bDoCollisionTest = false;
	Super::Die();
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
	if (!bCanMove || !MyPlayerController->InputEnabled()) return;
	UCharacterMovementComponent* CharMovement = GetCharacterMovement();
	if (CharMovement->MovementMode == EMovementMode::MOVE_None) return;
	MovementDirection = ActionInstance.GetValue().Get<FVector2D>() * GetCurrentMovementSpeed();
	const FRotator ControlRotation = GetControlRotation();
	AddMovementInput(UKismetMathLibrary::GetRightVector(FRotator(0, ControlRotation.Yaw, ControlRotation.Roll)), MovementDirection.X);
	AddMovementInput(UKismetMathLibrary::GetForwardVector(FRotator(0, ControlRotation.Yaw, 0)), MovementDirection.Y);
	PlayerCameraManager->StartCameraShake(WalkingCameraShake);
	
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
		//StopAimimg();
		PlayAnimMontage(Evade_FwAnimMontage);
		FTimerDelegate MyDelegate;
		MyDelegate.BindLambda([&]() {
			bCanEvade = true;
		});
		GetWorldTimerManager().SetTimer(EvadeTimerHandle, MyDelegate, 0.5f, false);
		return;
	}
	
	bIsAimingGrenade = false;
	//StopAimimg();
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
		FVector Final = GetActorLocation() + Difference - (GetActorForwardVector() * 90) + (FVector::UpVector * 5);
		SetActorLocation(Final);
		PlayAnimMontage(NormalClimbAnimMontage);
		ResetMovementVelocity(true);
		bCanEvade = true;
	}
	else if (bResult == EClimbType::LEDGE_CLIMB) {
		ResetMovementVelocity(true);
		FVector Difference = HitResult.ImpactPoint - GetActorLocation();
		FVector Final = GetActorLocation() + Difference - (GetActorUpVector() * 68) - (GetActorForwardVector() * 30);
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
	if (!bIsAimingWeapon) {
		PlayAnimMontage(Roll_FwAnimMontage);
		return;
	}
	
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
	bUseControllerRotationYaw = false;
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
	bUseControllerRotationYaw = bValue;
	bIsAimingWeapon = bValue;
	SpringArmLengthTarget = (bIsAimingWeapon ? SpringArmLengthAimingMult : 1) * SpringArmLengthInital;
	if (bIsAimingWeapon)
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void AThirdPersonPlayerCharacter::StartShootWeapon(const FInputActionInstance& ActionInstance)
{
	if (!MyPlayerController->InputEnabled() || CurrentWeapon == nullptr) return;
	bIsAimingGrenade = false;
	if (!bHasWeaponEquipped) return;
	UCharacterMovementComponent* CharMovement = GetCharacterMovement();
	if (CharMovement->MovementMode == EMovementMode::MOVE_None) return;
	if (!bIsAimingWeapon && CurrentWeapon->bShouldAimToShoot) return;
	CurrentWeapon->bIsFiring = true;
}

void AThirdPersonPlayerCharacter::ShootWeapon(const FInputActionInstance& ActionInstance)
{
	if (!MyPlayerController->InputEnabled() || CurrentWeapon == nullptr) return;
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
	if (!CurrentWeapon->Shoot(Start, End)) return;
	// FVector ControlRotation = End - Start;
	bUseControllerRotationYaw = true;
	if (!bIsAimingWeapon)
		GetWorldTimerManager().SetTimerForNextTick(this, &AThirdPersonPlayerCharacter::StopAimimg);
	if (CurrentWeapon->ShootMontage) {
		// UE_LOG(LogTemp, Warning);
		PlayAnimMontage(CurrentWeapon->ShootMontage);
	}
	if (!ShootingCameraShake) return;
	PlayerCameraManager->StartCameraShake(ShootingCameraShake);
}

void AThirdPersonPlayerCharacter::StopShootWeapon(const FInputActionInstance& ActionInstance)
{
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
	if (!bCanMove || !MyPlayerController->InputEnabled()) return;
	UCharacterMovementComponent* CharMovement = GetCharacterMovement();
	if (CharMovement->MovementMode == EMovementMode::MOVE_None) return;
}

void AThirdPersonPlayerCharacter::StartGrenade(const FInputActionInstance& ActionInstance)
{
	StopAnimMontage();
	bIsAimingGrenade = true;
	if (CurrentWeapon != nullptr)
		CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, WeaponAttachmentSocketName);
	bHasWeaponEquipped = false;
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

void AThirdPersonPlayerCharacter::ApplyEpicEffect(float TimeDilationAmount, FVector Location, float Duration, bool bIsAttached, bool bPlayNiagara, bool bSlowDownPlayer)
{
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
			MyDelegate.BindLambda([&]() {
				UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.f);
			});
		}
		GetWorldTimerManager().SetTimer(EpicEffectTimerHandle, MyDelegate, Duration, false);
	}
}

void AThirdPersonPlayerCharacter::PickupWeapon(AWeapon* WeaponToPickup)
{
	WeaponToPickup->SetOwner(this);
	WeaponToPickup->SetInstigator(this);
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
