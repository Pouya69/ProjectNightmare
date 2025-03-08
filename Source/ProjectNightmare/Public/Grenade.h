// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Grenade.generated.h"

UCLASS()
class PROJECTNIGHTMARE_API AGrenade : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGrenade();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = "Movement")
		class UProjectileMovementComponent* ProjectileMovementComponent;
	UPROPERTY(EditAnywhere, Category = "Movement")
		class USphereComponent* SphereComp;
	UPROPERTY(EditAnywhere, Category = "Movement")
		USceneComponent* RootComp;

	UPROPERTY(EditAnywhere)
		class UNiagaraSystem* ExplosionNiagara;
	void SetSpeed(float Speed);
	UFUNCTION()
		void Explode(AActor* DestroyedActor);


};
