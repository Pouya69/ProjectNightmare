// Fill out your copyright notice in the Description page of Project Settings.


#include "Player Combo System/PlayerComboComponent.h"
#include "Player Combo System/Ultimate/UltimateAoESphere.h"

// Sets default values for this component's properties
UPlayerComboComponent::UPlayerComboComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UPlayerComboComponent::BeginPlay()
{
	Super::BeginPlay();
	// ...
	
}

UAnimMontage* UPlayerComboComponent::CurrentShootComboAdd(ECombatComboType CombatType, float VelocityLengthXY)
{
	if (CombatType == ECombatComboType::NOT_COMBO_TYPE) {
		ResetCombatCombo();
		return nullptr; 
	}
	if (CurrentShootCombo.IsEmpty()) {
		CurrentShootCombo.Add(CombatType);
		SetResetTimer();
		return nullptr;
	}
	if (CurrentShootCombo.Num() < CombatComboLimit) {
		CurrentShootCombo.Add(CombatType);
		const int AddedIndex = CurrentShootCombo.Num() - 1;
		SetResetTimer();
		// Heavy Shot Combo Check
		UAnimMontage* ComboToExecuteAnimMontage = VelocityLengthXY > 5 ? HeavyShotAnimMontage : HeavyShotMovingAnimMontage;
		if (!bIsHeavyShotOkay || !CombatComboShotgunHeavyShot.IsValidIndex(AddedIndex) || CombatComboShotgunHeavyShot[AddedIndex] != CombatType) {
			ComboToExecuteAnimMontage = nullptr;
			bIsHeavyShotOkay = false;
		}
		if (CombatComboShotgunHeavyShot.Num() != AddedIndex + 1)
			ComboToExecuteAnimMontage = nullptr;
		if (ComboToExecuteAnimMontage != nullptr) {
			if (bDoesComboResetAfterHeavyShot)
				ResetCombatCombo();
			UE_LOG(LogTemp, Warning, TEXT("Heavy Shot..."));
			return ComboToExecuteAnimMontage;
		}

		// Blast Shot Combo Check
		ComboToExecuteAnimMontage = VelocityLengthXY > 5 ? BlastShotMovingAnimMontage : BlastShotAnimMontage;
		if (!bIsBlastShotOkay || !CombatComboBlastShot.IsValidIndex(AddedIndex) || CombatComboBlastShot[AddedIndex] != CombatType) {
			bIsBlastShotOkay = false;
			ComboToExecuteAnimMontage = nullptr;
		}
		if (CombatComboBlastShot.Num() != AddedIndex + 1)
			ComboToExecuteAnimMontage = nullptr;
		if (ComboToExecuteAnimMontage != nullptr) {
			if (bDoesComboResetAfterBlastShot)
				ResetCombatCombo();
			UE_LOG(LogTemp, Warning, TEXT("Blast Shot..."));
			return ComboToExecuteAnimMontage;
		}


	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Combo Array too big. Resetting..."));
		ResetCombatCombo();
	}
	return nullptr;
}

void UPlayerComboComponent::ResetCombatCombo()
{
	bIsBlastShotOkay = true;
	bIsHeavyShotOkay = true;
	CurrentShootCombo.Empty();
	UE_LOG(LogTemp, Warning, TEXT("Resetting Combo..."));
}

void UPlayerComboComponent::SetResetTimer()
{
	GetWorld()->GetTimerManager().ClearTimer(ResetCombatComboTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(ResetCombatComboTimerHandle, this, &UPlayerComboComponent::ResetCombatCombo, ResetCombatComboAfterSeconds, false);
}

void UPlayerComboComponent::Ultimate_Start(const AActor* PlayerRef, const FVector& StartPoint, const FVector& Direction)
{
	if (UltimateSphere) {
		UltimateSphere->Destroy();
		UltimateSphere = nullptr;
	}
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(PlayerRef);
	const FVector End = StartPoint + UltimateRaycastRange * Direction;
	FHitResult HitResult;
	bIsOkayToShoot = GetWorld()->LineTraceSingleByChannel(HitResult, StartPoint, End, ECC_Visibility, Params);
	const FTransform SpawnTransform = FTransform(FRotator::ZeroRotator, bIsOkayToShoot ? HitResult.ImpactPoint : End, FVector::OneVector);
	UltimateSphere = GetWorld()->SpawnActorDeferred<AUltimateAoESphere>(UltimateSphereClass, SpawnTransform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	UltimateSphere->FinishSpawning(SpawnTransform);

}

void UPlayerComboComponent::Ultimate_Shoot()
{
	UltimateSphere->bShouldDestroyWithoutExplosion = false;
	UltimateSphere->Destroy();
	UltimateSphere = nullptr;
}

void UPlayerComboComponent::Ultimate_Ongoing(float& DeltaTime, const AActor* PlayerRef, const FVector& StartPoint, const FVector& Direction)
{
	if (!UltimateSphere) return;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(UltimateSphere);
	Params.AddIgnoredActor(PlayerRef);
	const FVector End = StartPoint + UltimateRaycastRange * Direction;
	FHitResult HitResult;
	bIsOkayToShoot = GetWorld()->LineTraceSingleByChannel(HitResult, StartPoint, End, ECC_Visibility, Params);

	// UltimateSphere->TargetLocation = FMath::VInterpConstantTo(UltimateSphere->GetActorLocation(), bIsOkayToShoot ? HitResult.ImpactPoint : End, DeltaTime, UltimateSphere->LocationTransitionSpeed);
	UltimateSphere->SetActorLocation(bIsOkayToShoot ? HitResult.ImpactPoint : End);
	// UltimateSphere->SetActorLocation(HitResult.ImpactPoint);
	UltimateSphere->UpdateSphere(DeltaTime);
}


/*
// Called every frame
void UPlayerComboComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}
*/
