// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyBaseCharacter.h"
#include "FlyingMutantCharacter.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTNIGHTMARE_API AFlyingMutantCharacter : public AEnemyBaseCharacter
{
	GENERATED_BODY()

public:
	AFlyingMutantCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
		float ObstacleCheckInFrontDistance = 100.f;

	float CapsuleHalfHeight;

	FVector CustomAccel;
	bool FlyToLocation(const FVector FinalDirection, bool bForce);
	class UCharacterMovementComponent* CharacterMovementComp;

public:
	// Grab Player
	UPROPERTY(EditAnywhere, Category = "Grab Player")
		float HeightForDroppingPlayer = 500.f;
	UPROPERTY(EditAnywhere, Category="Grab Player")
		FName PlayerGrabSocket;
	UFUNCTION(BlueprintCallable, Category = "Grab Player")
		void GrabPlayer(UObject* PlayerCharacterRef);
	UFUNCTION(BlueprintCallable, Category = "Grab Player")
		void ReleasePlayer(UObject* PlayerCharacterRef, bool bDidPlayerBreakTheAttack = false);

	class AThirdPersonPlayerCharacter* GrabbedPlayerCharacterRef;
		
};
