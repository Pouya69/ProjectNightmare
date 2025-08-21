// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyBaseCharacter.h"
#include "FemaleMutantCharacter.generated.h"

/**
 * 
 */

UCLASS()
class PROJECTNIGHTMARE_API AFemaleMutantCharacter : public AEnemyBaseCharacter
{
	GENERATED_BODY()
	
public:
	AFemaleMutantCharacter();

	virtual bool AttackPlayer() override;
	virtual void Die() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Comps")
		class UMotionWarpingComponent* MotionWarpingComp;
	// Gets matched with the index of Combos array.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combos")
		TArray<UAnimMontage*> Normal_Attack_1_Combo;
	virtual void ChangeAttackComboTypeTo(EEnemyComboType NewComboType) override;
	virtual void ResetCombo(bool bIsDash = false) override;
	UFUNCTION(BlueprintCallable, Category = "Combos")
		void NormalAttack1();
	virtual void Dash(UAnimMontage* DashAnimMontage, FVector TargetLocation, FName OverrideMotionWarp, bool bBackwards = true) override;
};
