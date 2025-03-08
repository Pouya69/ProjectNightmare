// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_FindRandomWalkLocation.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTNIGHTMARE_API UBTTask_FindRandomWalkLocation : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_FindRandomWalkLocation();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float RadiusCheck = 800.f;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
