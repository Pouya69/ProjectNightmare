// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_FlyTo.h"
#include "FlyingMutantCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnemyBaseAIController.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"


UBTTask_FlyTo::UBTTask_FlyTo()
{
	NodeName = TEXT("Fly To");
}

EBTNodeResult::Type UBTTask_FlyTo::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	AFlyingMutantCharacter* EnemyBaseCharacter = Cast<AFlyingMutantCharacter>(OwnerComp.GetAIOwner()->GetCharacter());
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	ACharacterBase* TargetCharacter = nullptr;
	const FVector SelfLocation = EnemyBaseCharacter->GetActorLocation();
	FVector TargetLocation;
	if (bIsTargetCharacter) {
		TargetCharacter = Cast<ACharacterBase>(BlackboardComp->GetValueAsObject(GetSelectedBlackboardKey()));
		if (!TargetCharacter->IsAlive()) {
			BlackboardComp->SetValueAsBool(FName("IsMovingToLocation"), false);
			return EBTNodeResult::Failed;
		}
		TargetLocation = TargetCharacter->GetActorLocation();
	}
	else
		TargetLocation = BlackboardComp->GetValueAsVector(GetSelectedBlackboardKey());
	if (BlackboardComp->GetValueAsBool(FName("IsMovingToLocation"))) {
		if (FVector::Dist(SelfLocation, TargetLocation) <= AcceptableRadius) {
			BlackboardComp->SetValueAsBool(FName("IsMovingToLocation"), false);
			return EBTNodeResult::Failed;
		}
		// Move to location
		// const float FinalSpeed = EnemyBaseCharacter->GetMovementSpeed(EMovementMode::MOVE_Flying);
		const bool bShouldTryAgain = EnemyBaseCharacter->FlyToLocation((TargetLocation - SelfLocation).GetSafeNormal(), true);
		//if (bShouldTryAgain && !bIsTargetCharacter)
			//return EBTNodeResult::Failed;
		return EBTNodeResult::Succeeded;
	}
	// Set new location
	BlackboardComp->SetValueAsVector(FName("TargetDestination"), TargetLocation);
	BlackboardComp->SetValueAsBool(FName("IsMovingToLocation"), true);
	return EBTNodeResult::Succeeded;
	
}
