// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_GrabPlayer.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTNIGHTMARE_API UBTTask_GrabPlayer : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_GrabPlayer();
	
	// To do ReleasePlayer() instead of GrabPlayer()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bIsRelease = false;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
