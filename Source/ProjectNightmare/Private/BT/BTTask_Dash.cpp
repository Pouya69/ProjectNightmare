// Fill out your copyright notice in the Description page of Project Settings.


#include "BT/BTTask_Dash.h"
#include "AIController.h"
#include "EnemyBaseCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_Dash::UBTTask_Dash()
{
	NodeName = TEXT("Dash");
}

EBTNodeResult::Type UBTTask_Dash::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AEnemyBaseCharacter* EnemyBaseCharacter = Cast<AEnemyBaseCharacter>(OwnerComp.GetAIOwner()->GetCharacter());
	EnemyBaseCharacter->Dash(DashAnimMontage, OwnerComp.GetBlackboardComponent()->GetValueAsVector(FName("TargetDestination")), OverrideMotionWarp, bBackwards);
	return EBTNodeResult::Succeeded;
}
