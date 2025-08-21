// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraLockOn.h"
#include "ThirdPersonPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "EnemyBaseCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Weapon.h"

// Sets default values for this component's properties
UCameraLockOn::UCameraLockOn()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UCameraLockOn::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UCameraLockOn::HandleCameraLockOn(const FVector2D& LookForce, const float& DeltaTime)
{
	if (EnemyCharacterLockedOn == nullptr) return;
	if (!EnemyCharacterLockedOn->IsAlive()) {
		StopLockOn_FORCED();
		return;
	}
	if (IsTargetBlocked()) {
		if (GetWorld()->GetTimerManager().IsTimerActive(StopLockOnTimerHandle)) return;
		// UE_LOG(LogTemp, Error, TEXT("Blocked. Setting Timer"));
		GetWorld()->GetTimerManager().SetTimer(StopLockOnTimerHandle, this, &UCameraLockOn::StopLockOn, StopLockOnAfterNotInSightInSeconds, false);
		return;
	}
	if (GetWorld()->GetTimerManager().IsTimerActive(StopLockOnTimerHandle))
		GetWorld()->GetTimerManager().ClearTimer(StopLockOnTimerHandle);
	// const FVector Direction = GetDirectionToTarget();
	// const FVector FinalDirection = FMath::VInterpConstantTo(OwnerPlayerRef->CameraComp->GetForwardVector(), Direction, DeltaTime, 	LockOnCameraRotationSpeed);
	// const FRotator FinalCameraRotation = FinalDirection.Rotation();
	FRotator TargetRotation = GetDirectionToTarget().Rotation();
	FRotator PlayerControlRotation = OwnerPlayerRef->MyPlayerController->GetControlRotation();
	UE_LOG(LogTemp, Warning, TEXT("%f"), (float) LookForce.Length());
	if (LookForce.Length() >= LockOnSwitchForceThreshold) {
		// Switch to a new LockOn Target
		StartLockOn();
	}
	const FRotator FinalRotation = FMath::RInterpConstantTo(PlayerControlRotation, TargetRotation, DeltaTime, LockOnCameraRotationSpeed);
	OwnerPlayerRef->MyPlayerController->SetControlRotation(FinalRotation);
	// OwnerPlayerRef->SpringArmComp->SetWorldRotation(FinalCameraRotation, false);
}

FVector UCameraLockOn::GetDirectionToTarget() const
{
	if (EnemyCharacterLockedOn == nullptr)
		return FVector::ZeroVector;
	FVector Direction = EnemyCharacterLockedOn->GetPelvisLocation() - OwnerPlayerRef->CameraComp->GetComponentLocation();
	Direction.Normalize();
	return Direction;
}

bool UCameraLockOn::IsTargetBlocked() const
{
	if (EnemyCharacterLockedOn == nullptr) return true;
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerPlayerRef);
	if (OwnerPlayerRef->CurrentWeapon)
		QueryParams.AddIgnoredActor(OwnerPlayerRef->CurrentWeapon);
	if (OwnerPlayerRef->PistolWeapon)
		QueryParams.AddIgnoredActor(OwnerPlayerRef->PistolWeapon);
	const bool bIsBlocked = GetWorld()->LineTraceSingleByChannel(HitResult, OwnerPlayerRef->CameraComp->GetComponentLocation(),
		EnemyCharacterLockedOn->GetPelvisLocation(), ECC_Visibility, QueryParams); // && HitResult.GetActor() && !HitResult.GetActor()->IsA(AEnemyBaseCharacter::StaticClass());
	if (bIsBlocked) {
		if (HitResult.GetActor() == nullptr)
			return true;
		return !HitResult.GetActor()->IsA(AEnemyBaseCharacter::StaticClass());
	}
	return false;
}

bool UCameraLockOn::IsTargetBlocked(AEnemyBaseCharacter* TargetToScan) const
{
	if (TargetToScan == nullptr) return true;
	FHitResult HitResult; 
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerPlayerRef);
	if (OwnerPlayerRef->CurrentWeapon)
		QueryParams.AddIgnoredActor(OwnerPlayerRef->CurrentWeapon);
	if (OwnerPlayerRef->PistolWeapon)
		QueryParams.AddIgnoredActor(OwnerPlayerRef->PistolWeapon);
	const bool bIsBlocked = GetWorld()->LineTraceSingleByChannel(HitResult, OwnerPlayerRef->CameraComp->GetComponentLocation(),
		TargetToScan->GetPelvisLocation(), ECC_Visibility, QueryParams); // && HitResult.GetActor() && !HitResult.GetActor()->IsA(AEnemyBaseCharacter::StaticClass());
	if (bIsBlocked) {
		if (HitResult.GetActor() == nullptr)
			return true;
		return !HitResult.GetActor()->IsA(AEnemyBaseCharacter::StaticClass());
	}
	return false;
}

bool UCameraLockOn::StartLockOn()
{
	if (GetWorld()->GetTimerManager().IsTimerActive(LockOnNextTargetCooldownTimerHandle)) return false;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerPlayerRef);
	if (OwnerPlayerRef->CurrentWeapon)
		QueryParams.AddIgnoredActor(OwnerPlayerRef->CurrentWeapon);
	if (OwnerPlayerRef->PistolWeapon)
		QueryParams.AddIgnoredActor(OwnerPlayerRef->PistolWeapon);
	TArray<FHitResult> HitResults;
	const FVector Start = OwnerPlayerRef->CameraComp->GetComponentLocation();
	const FVector End = Start + StartLockOnScanDistance * OwnerPlayerRef->GetCameraLookingDirection().RotateAngleAxis(OwnerPlayerRef->LookForce.X, FVector(0,0,1))
		.RotateAngleAxis(OwnerPlayerRef->LookForce.Y, FVector(0, 1, 0));
	const bool bDidHit = GetWorld()->SweepMultiByChannel(HitResults, Start, End, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(StartLockOnScanRadius), QueryParams);
	if (!bDidHit)
		return false;
	bool bChoseNew = false;
	float ClosestDistance = FLT_MAX;
	for (const FHitResult& HitResult : HitResults) {
		if (AEnemyBaseCharacter* EnemyRef = Cast<AEnemyBaseCharacter>(HitResult.GetActor())) {
			if (EnemyCharacterLockedOn == EnemyRef) continue;  // For switching to a new one.
			if (!IsTargetBlocked(EnemyRef)) {
				if (!bChoseNew || HitResult.Distance < ClosestDistance) {
					bChoseNew = true;
					EnemyCharacterLockedOn = EnemyRef;
					ClosestDistance = HitResult.Distance;
				}
				//OwnerPlayerRef->bUseControllerRotationYaw = true;
			}
		}
	}
	if (bChoseNew)
		GetWorld()->GetTimerManager().SetTimer(LockOnNextTargetCooldownTimerHandle, LockOnNextTargetCooldownInSeconds, false);
	return EnemyCharacterLockedOn != nullptr;
}

void UCameraLockOn::StopLockOn()
{
	if (IsTargetBlocked())
		EnemyCharacterLockedOn = nullptr;
	// OwnerPlayerRef->StopAimimg();
}

void UCameraLockOn::StopLockOn_FORCED()
{
	EnemyCharacterLockedOn = nullptr;
}
