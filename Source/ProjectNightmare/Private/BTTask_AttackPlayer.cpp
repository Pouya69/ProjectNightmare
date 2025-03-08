// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_AttackPlayer.h"
#include "AIController.h"
#include "EnemyBaseCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_AttackPlayer::UBTTask_AttackPlayer()
{
	NodeName = TEXT("Attack Player");
}

EBTNodeResult::Type UBTTask_AttackPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AEnemyBaseCharacter* EnemyBaseCharacter = Cast<AEnemyBaseCharacter>(OwnerComp.GetAIOwner()->GetCharacter());
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	BlackboardComp->SetValueAsInt(FName("TimesAttackedPlayer"), BlackboardComp->GetValueAsInt(FName("TimesAttackedPlayer"))+1);
	EnemyBaseCharacter->AttackPlayer();
	return EBTNodeResult::Succeeded;
}
