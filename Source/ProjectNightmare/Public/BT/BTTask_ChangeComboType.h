// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ChangeComboType.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTNIGHTMARE_API UBTTask_ChangeComboType : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_ChangeComboType();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		uint8 NewComboType_UINT8 = 0U;
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
