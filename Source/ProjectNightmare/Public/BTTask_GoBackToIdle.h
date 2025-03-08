// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_GoBackToIdle.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTNIGHTMARE_API UBTTask_GoBackToIdle : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_GoBackToIdle();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
};
