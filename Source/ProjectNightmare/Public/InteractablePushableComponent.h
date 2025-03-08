// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "InteractablePushableComponent.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTNIGHTMARE_API UInteractablePushableComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere)
		int Value;

	void InteractPush();

	class AInteractableObject* InteractableOwner;
};
