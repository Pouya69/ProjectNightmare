// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_SetBlackboardValue.generated.h"

/**
 * 
 */

UENUM(Blueprintable)
enum class EValueType : uint8 {
	BOOL,
	FLOAT,
	INT
};

UCLASS()
class PROJECTNIGHTMARE_API UBTTask_SetBlackboardValue : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_SetBlackboardValue();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EValueType ValueType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int ValueInt;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool ValueBool;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float ValueFloat;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
