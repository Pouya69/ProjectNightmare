// Fill out your copyright notice in the Description page of Project Settings.


#include "FemaleMutantCharacter.h"
#include "FemaleMutantAIController.h"
#include "MotionWarpingComponent.h"

AFemaleMutantCharacter::AFemaleMutantCharacter()
{
	MotionWarpingComp = CreateDefaultSubobject<UMotionWarpingComponent>(FName("Motion Warp Comp"));
}

bool AFemaleMutantCharacter::AttackPlayer()
{
	// Super::AttackPlayer();
	if (!EnemyAIController->SoftPlayerRef_READONLY) return false;
	switch (CurrentComboType)
	{
		case EEnemyComboType::NORMAL_ATTACK_1:
			break;
		default:
			break;
	}
	FVector Offset = GetActorLocation() - EnemyAIController->SoftPlayerRef_READONLY->GetActorLocation();
	Offset.Z = 0;
	Offset.Normalize();
	MotionWarpingComp->AddOrUpdateWarpTargetFromComponent(FName("Combo_" + FString::FromInt(CurrentComboIndex + 1)),
		EnemyAIController->SoftPlayerRef_READONLY->GetRootComponent(),
		FName(), false, FVector::ZeroVector, FRotator::ZeroRotator);
	PlayAnimMontage(Normal_Attack_1_Combo[CurrentComboIndex]);
	if (CurrentComboIndex >= Normal_Attack_1_Combo.Num() - 1) {
		CurrentComboIndex = 0;
		return true;
	}
	CurrentComboIndex++;
	return false;
}

void AFemaleMutantCharacter::Die()
{
	Super::Die();
	GetMesh()->SetBodySimulatePhysics(FName("root"), true);
	GetMesh()->SetSimulatePhysics(true);
}

void AFemaleMutantCharacter::ChangeAttackComboTypeTo(EEnemyComboType NewComboType)
{
	Super::ChangeAttackComboTypeTo(NewComboType);
}

void AFemaleMutantCharacter::ResetCombo(bool bIsDash)
{
	Super::ResetCombo(bIsDash);
}

void AFemaleMutantCharacter::NormalAttack1()
{
}

void AFemaleMutantCharacter::Dash(UAnimMontage* DashAnimMontage, FVector TargetLocation, FName OverrideMotionWarp, bool bBackwards)
{
	FString NName;
	if (OverrideMotionWarp.IsNone()) {
		NName = FString("Dash_");
		if (bBackwards)
			NName += "Back_";
		NName += CurrentDashIndex % 2 == 0 ? "Right" : "Left";
		if (CurrentDashIndex == 0)
			NName += "_Start";
	}
	UE_LOG(LogTemp, Warning, TEXT("%s"), *NName);
	FVector ToTarget = TargetLocation - GetActorLocation();
	ToTarget.Z = 0;
	ToTarget.Normalize();
	MotionWarpingComp->AddOrUpdateWarpTargetFromLocationAndRotation(OverrideMotionWarp.IsNone() ? FName(NName) : OverrideMotionWarp, TargetLocation,
		bBackwards ? (-ToTarget).Rotation() : ToTarget.Rotation());
	PlayAnimMontage(DashAnimMontage);
	CurrentDashIndex++;
}
