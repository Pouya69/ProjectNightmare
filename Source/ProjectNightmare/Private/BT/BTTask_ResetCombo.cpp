// Fill out your copyright notice in the Description page of Project Settings.


#include "BT/BTTask_ResetCombo.h"
#include "AIController.h"
#include "EnemyBaseCharacter.h"

UBTTask_ResetCombo::UBTTask_ResetCombo()
{
	NodeName = TEXT("Reset Combo");
}

EBTNodeResult::Type UBTTask_ResetCombo::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AEnemyBaseCharacter* EnemyBaseCharacter = Cast<AEnemyBaseCharacter>(OwnerComp.GetAIOwner()->GetCharacter());
	EnemyBaseCharacter->ResetCombo(bIsDash);
	return EBTNodeResult::Succeeded;
}
