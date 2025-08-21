// Fill out your copyright notice in the Description page of Project Settings.


#include "BT/BTTask_ChangeComboType.h"
#include "AIController.h"
#include "EnemyBaseCharacter.h"


UBTTask_ChangeComboType::UBTTask_ChangeComboType()
{
	NodeName = TEXT("Change Combo Type");
}

EBTNodeResult::Type UBTTask_ChangeComboType::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AEnemyBaseCharacter* EnemyBaseCharacter = Cast<AEnemyBaseCharacter>(OwnerComp.GetAIOwner()->GetCharacter());
	EnemyBaseCharacter->ChangeAttackComboTypeTo(static_cast<EEnemyComboType>(NewComboType_UINT8));
	return EBTNodeResult::Succeeded;
}
