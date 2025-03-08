// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyBaseAIController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTNIGHTMARE_API AEnemyBaseAIController : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyBaseAIController();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

public:
	// Perception and AI
	UPROPERTY(EditAnywhere, Category = "Perception and AI")
		class UBehaviorTree* EnemyBehaviour;
	UPROPERTY(EditAnywhere, Category = "Perception and AI | KEYS")
		FName PlayerCharacterRefNameKEY = FName("PlayerCharacterRef");
	UPROPERTY(EditAnywhere, Category = "Perception and AI | KEYS")
		FName LastKnownPlayerLocationKEY = FName("LastKnownPlayerLocation");
	UPROPERTY(EditAnywhere, Category = "Perception and AI | KEYS")
		FName TimeSinceSeenPlayerKEY = FName("TimeSinceSeenPlayer");
	UAIPerceptionComponent* AIPerceptionComp;
	UFUNCTION()
		void OnPerceptionInfoUpdated(AActor* Actor, struct FAIStimulus Stimulus);
	UFUNCTION()
		void OnPerceptionInfoForgotten(AActor* Actor);
	void HandleSense_Sight(AActor* Actor, struct FAIStimulus& Stimulus);
	
};
