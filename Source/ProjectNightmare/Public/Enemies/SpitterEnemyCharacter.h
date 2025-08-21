// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyBaseCharacter.h"
#include "SpitterEnemyCharacter.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTNIGHTMARE_API ASpitterEnemyCharacter : public AEnemyBaseCharacter
{
	GENERATED_BODY()
	
public:
	// virtual bool AttackPlayer() override;

	UPROPERTY(EditAnywhere, Category="Spit")
		TSubclassOf<class ASpitter_Projectile> SpitterProjectileClass;
	UFUNCTION(BlueprintCallable, Category = "Spit")
		void SpawnSpit(const FVector TargetHitLocation);
};
