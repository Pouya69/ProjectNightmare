// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyBaseAIController.h"
#include "FemaleMutantAIController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTNIGHTMARE_API AFemaleMutantAIController : public AEnemyBaseAIController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
};
