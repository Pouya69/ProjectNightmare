// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ResetCombo.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTNIGHTMARE_API UBTTask_ResetCombo : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ResetCombo();
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bIsDash = false;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
