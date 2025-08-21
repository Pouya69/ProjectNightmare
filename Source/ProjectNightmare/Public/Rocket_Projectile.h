// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Rocket_Projectile.generated.h"

UCLASS()
class PROJECTNIGHTMARE_API ARocket_Projectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	FTimerHandle DestroyTimer;
	ARocket_Projectile();
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		class USplineComponent* PathSpline;
	UFUNCTION(BlueprintCallable)
		void InitializeRocket(const FVector InRocketDirection, const FRotator InitialRotation);
	UPROPERTY(BlueprintReadOnly)
		FVector ShotDirection;
	UPROPERTY(BlueprintReadOnly)
		FVector InitialLocation;
	UPROPERTY(BlueprintReadOnly)
		FRotator FinalRotation;
	UPROPERTY(BlueprintReadOnly)
		FRotator InitialRotation;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float TimeBeforeDestroy = 8.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float ExplosionDanage = 300.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float RocketSpeed = 50.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float ExplosionRadius = 300.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		class UNiagaraSystem* Explosion_NS;
	UFUNCTION()
		void Exploded(AActor* DestroyedActor);
	UFUNCTION()
		void RocketHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);
	UFUNCTION(BlueprintCallable)
		void Explode();
	UPROPERTY(BlueprintReadOnly)
		FVector ExplosionLocation;
	UPROPERTY(BlueprintReadOnly)
		FVector ExplosionNormal;
	UPROPERTY(BlueprintReadOnly)
		bool bWasHit = false;
	UPROPERTY(EditAnywhere)
		TSubclassOf<UDamageType> RocketExplosionDamageType;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
