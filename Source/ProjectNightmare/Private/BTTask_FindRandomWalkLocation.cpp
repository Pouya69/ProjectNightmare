// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_FindRandomWalkLocation.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_FindRandomWalkLocation::UBTTask_FindRandomWalkLocation()
{
	NodeName = TEXT("Find Random Walk Location");
}

EBTNodeResult::Type UBTTask_FindRandomWalkLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	AAIController* OwnerAIController = Cast<AAIController>(OwnerComp.GetAIOwner());
	ACharacter* OwnerPawn = OwnerAIController->GetCharacter();
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(this);
	FNavLocation NavLocationResult;
	const bool bFoundLocation = NavSystem->GetRandomReachablePointInRadius(OwnerPawn->GetActorLocation(), RadiusCheck, NavLocationResult);
	if (!bFoundLocation)
		return EBTNodeResult::Failed;
	BlackboardComp->SetValueAsVector(GetSelectedBlackboardKey(), NavLocationResult.Location);
	return EBTNodeResult::Succeeded;
}
