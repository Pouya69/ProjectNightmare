// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_GoBackToIdle.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_GoBackToIdle::UBTTask_GoBackToIdle()
{
	NodeName = TEXT("Go back to idle");
}

EBTNodeResult::Type UBTTask_GoBackToIdle::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	BlackboardComp->ClearValue(FName("HasSeenPlayer"));
	BlackboardComp->ClearValue(FName("TimeSinceSeenPlayer"));
	BlackboardComp->ClearValue(FName("TimesAttackedPlayer"));
	// BlackboardComp->ClearValue(FName("TargetDestination"));
	return EBTNodeResult::Succeeded;
}
