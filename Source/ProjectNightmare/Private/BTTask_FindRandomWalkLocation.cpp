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
	const FVector SelfLocation = OwnerPawn->GetActorLocation();
	if (bIsFlying) {
		float FinalZValue = FMath::RandBool() ? 1 : -1 * FMath::FRandRange(0.2f, 3.f) * RadiusCheck;
		const FVector Start = OwnerPawn->GetActorLocation();
		const FVector End = Start + FVector(0,0, FinalZValue);
		FCollisionQueryParams QueryParams;
		FHitResult HitResult;
		QueryParams.AddIgnoredActor(OwnerPawn);
		bool bIsBlocked = GetWorld()->SweepSingleByChannel(HitResult, Start, End, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(50.f), QueryParams);
		if (bIsBlocked)
			FinalZValue = 0.f;
		const FVector FinalLocation = SelfLocation + FVector(FMath::RandBool() ? 1 : -1 * FMath::FRandRange(0.5f, 1.f) * RadiusCheck, FMath::RandBool() ? 1 : -1 * FMath::FRandRange(0.5f, 1.f) * RadiusCheck, FinalZValue);
		bIsBlocked = GetWorld()->SweepSingleByChannel(HitResult, SelfLocation, FinalLocation, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(40.f), QueryParams);
		if (bIsBlocked && FVector::Dist(SelfLocation, HitResult.ImpactPoint) <= 50.f)
			return EBTNodeResult::Failed;
		BlackboardComp->SetValueAsVector(GetSelectedBlackboardKey(), bIsBlocked ? HitResult.ImpactPoint : FinalLocation);
		return EBTNodeResult::Succeeded;
	}
	UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(this);
	FNavLocation NavLocationResult;
	const bool bFoundLocation = NavSystem->GetRandomReachablePointInRadius(SelfLocation, RadiusCheck, NavLocationResult);
	if (!bFoundLocation)
		return EBTNodeResult::Failed;
	BlackboardComp->SetValueAsVector(GetSelectedBlackboardKey(), NavLocationResult.Location);
	return EBTNodeResult::Succeeded;
}
