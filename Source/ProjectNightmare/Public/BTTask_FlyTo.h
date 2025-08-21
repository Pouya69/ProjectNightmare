// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
// #include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_FlyTo.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTNIGHTMARE_API UBTTask_FlyTo : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_FlyTo();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float AcceptableRadius = 50.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bIsTargetCharacter = true;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
