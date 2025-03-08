// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LocationSwapper.generated.h"

UCLASS()
class PROJECTNIGHTMARE_API ALocationSwapper : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALocationSwapper();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
		UMaterialInterface* FocusedMaterial;

	UFUNCTION(BlueprintCallable)
		void SwapLocation(AActor* Actor);

	UFUNCTION(BlueprintCallable)
		void SetInFocus(bool bFocused);

	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* StaticMeshComp;

	UPROPERTY(EditAnywhere)
		bool bDestroysOnUse = false;

	UFUNCTION(BlueprintCallable)
		bool HasPatrolPath() const;

	UPROPERTY(EditAnywhere)
		TArray<AActor*> PatrolPath;

	int CurrentPatrolPathIndex = 0;
	UPROPERTY(EditAnywhere)
		float PatrolSpeed = 300.f;

};
