// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Dash.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTNIGHTMARE_API UBTTask_Dash : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UAnimMontage* DashAnimMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bBackwards = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName OverrideMotionWarp;
	UBTTask_Dash();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
