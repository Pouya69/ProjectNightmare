// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NightmareGameMode.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTNIGHTMARE_API ANightmareGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANightmareGameMode();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		AActor* FourthDimensionLocationActor;
	UPROPERTY(EditAnywhere)
		TSubclassOf<class AFourthDimension_Portal> PortalToOverworldClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class AFourthDimension_Portal* PortalToOverworld;
	UPROPERTY(BlueprintReadOnly)
		class AThirdPersonPlayerCharacter* PlayerCharacter;

};
