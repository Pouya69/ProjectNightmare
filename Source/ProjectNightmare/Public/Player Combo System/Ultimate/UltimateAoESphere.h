// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UltimateAoESphere.generated.h"

UCLASS()
class PROJECTNIGHTMARE_API AUltimateAoESphere : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AUltimateAoESphere();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = "Sphere Properties")
		float SphereMaxRadius = 800.f;
	UPROPERTY(EditAnywhere, Category = "Sphere Properties")
		float SphereStartRadius = 200.f;
	UPROPERTY(EditAnywhere, Category = "Sphere Properties")
		float SphereSizeIncreaseRate = 100.f;
	UPROPERTY(EditAnywhere, Category = "Sphere Properties")
		float UltimateMaxDamage = 250.f;
	UPROPERTY(EditAnywhere, Category = "Sphere Properties")
		TSubclassOf<UDamageType> UltimateDamageType;
	UPROPERTY(EditAnywhere, Category="Comps")
		class USphereComponent* SphereOverlapComp;
	UFUNCTION()
		void Explode(AActor* DestroyedActor);
	UFUNCTION(BlueprintCallable, Category="Sphere Properties")
		float GetSphereRadius() const;
	bool bShouldDestroyWithoutExplosion = true;
	UFUNCTION(BlueprintCallable)
		void UpdateSphere(float& DeltaTime);
	UPROPERTY(EditAnywhere, Category = "Sphere Properties")
		float LocationTransitionSpeed = 50.f;
	FVector TargetLocation;

};
