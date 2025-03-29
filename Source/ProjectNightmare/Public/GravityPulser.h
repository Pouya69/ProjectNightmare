// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GravityPulser.generated.h"

UCLASS()
class PROJECTNIGHTMARE_API AGravityPulser : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGravityPulser();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
		FVector GravityDirection = FVector(1, 0, 0);
	UPROPERTY(EditAnywhere)
		float GravityForce = 1000.f;
	// If less than 0, No direction change
	UPROPERTY(EditAnywhere, Category="Direction Change")
		float ChangeDirectionTimeInSeconds = -1.f;
	// If true, e.g. FVector(1, 1, 0) is allowed
	UPROPERTY(EditAnywhere, Category = "Direction Change")
		bool bIsDiagonalDirectionAllowed = true;
	// If less than 0, it will be consistant
	UPROPERTY(EditAnywhere)
		float PulseTimerTimeInSeconds = -1.f;
	UPROPERTY(EditAnywhere)
		class UBoxComponent* OverlapAreaComp;
	UPROPERTY(EditAnywhere)
		USceneComponent* RootSceneComp;
	UFUNCTION()
		void GravityChange();
	UFUNCTION()
		void ChangeDirectionRandom();
	FTimerHandle PulseTimer;
	FTimerHandle DirectionChangeTimer;
};
