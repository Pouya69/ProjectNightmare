// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_GrabPlayer.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnemyBaseAIController.h"
#include "FlyingMutantCharacter.h"

UBTTask_GrabPlayer::UBTTask_GrabPlayer()
{
	NodeName = bIsRelease ? TEXT("Release Player") : TEXT("Grab Player");
}

EBTNodeResult::Type UBTTask_GrabPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	AFlyingMutantCharacter* EnemyBaseCharacter = Cast<AFlyingMutantCharacter>(OwnerComp.GetAIOwner()->GetCharacter());
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	UObject* PlayerCharacterRef = BlackboardComp->GetValueAsObject(FName("PlayerCharacterRef"));
	if (bIsRelease)
		EnemyBaseCharacter->ReleasePlayer(PlayerCharacterRef);
	else {
		BlackboardComp->SetValueAsVector(FName("TargetDestination"),
			EnemyBaseCharacter->GetActorLocation() + FVector(FMath::RandBool() ? 1 : -1 * FMath::FRandRange(0.5f, 1.f) * EnemyBaseCharacter->HeightForDroppingPlayer,
				FMath::RandBool() ? 1 : -1 * FMath::FRandRange(0.5f, 1.f) * EnemyBaseCharacter->HeightForDroppingPlayer,
				EnemyBaseCharacter->HeightForDroppingPlayer));
				
		EnemyBaseCharacter->GrabPlayer(PlayerCharacterRef);
	}
	BlackboardComp->SetValueAsBool(FName("IsMovingToLocation"), !bIsRelease);
	BlackboardComp->SetValueAsBool(FName("GrabbedPlayer"), !bIsRelease);
	return EBTNodeResult::Succeeded;
}
