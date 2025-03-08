// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Grenade.h"
#include "Components/SceneComponent.h"
#include "CharacterGrenadeHandler.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTNIGHTMARE_API UCharacterGrenadeHandler : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCharacterGrenadeHandler();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	class USplineComponent* SplineComp;
	UPROPERTY(EditAnywhere, Category = "Spline")
		class UStaticMesh* SplineMesh;
	UPROPERTY(EditAnywhere, Category = "Spline")
		float SplineSectionLength = 266.f;

	UPROPERTY(EditAnywhere, Category = "Grenade Initial Setup")
		TSubclassOf<AGrenade> GrenadeClass;
	UPROPERTY(EditAnywhere, Category="Grenade Initial Setup")
		float GrenadeLaunchVelocity;
	UPROPERTY(EditAnywhere, Category = "Grenade Initial Setup")
		float GrenadeLaunchVelocityAimMultiplier = 2.f;
	UPROPERTY(EditAnywhere, Category = "Grenade Initial Setup")
		float GrenadeRadius;
	UPROPERTY(EditAnywhere, Category = "Grenade Initial Setup")
		float GrenadeSimTime = 1.f;
	UPROPERTY(EditAnywhere, Category = "Grenade Initial Setup")
		float GrenadeSimFrequency = 20.f;
	TArray<USceneComponent*> CreatedSplines;
	void ThrowGrenade(bool bIsAiming);
	UFUNCTION(BlueprintCallable)
		void OngoingGrenadeAim(bool bIsAiming);
};
