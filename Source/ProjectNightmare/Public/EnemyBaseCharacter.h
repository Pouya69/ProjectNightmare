// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "EnemyBaseCharacter.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTNIGHTMARE_API AEnemyBaseCharacter : public ACharacterBase
{
	GENERATED_BODY()
	
public:
	AEnemyBaseCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadOnly)
		class AEnemyBaseAIController* EnemyAIController;

	UFUNCTION(BlueprintCallable)
		void AttackPlayer();
};
