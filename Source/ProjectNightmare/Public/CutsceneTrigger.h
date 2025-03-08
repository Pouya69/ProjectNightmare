// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CutsceneTrigger.generated.h"

UCLASS()
class PROJECTNIGHTMARE_API ACutsceneTrigger : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACutsceneTrigger();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// Components
	UPROPERTY(EditAnywhere)
		class UBoxComponent* BoxComp;
	UPROPERTY(EditAnywhere)
		class ACutsceneController* CutsceneController;

public:
	// Functions and Variables
	UPROPERTY(EditAnywhere)
		class UInputAction* ActionToStart;
	UPROPERTY(EditAnywhere)
		TSubclassOf<class UUserWidget> StartCutsceneWidget;
	UPROPERTY(EditAnywhere)
		FString PlayerCharacterName;

	UFUNCTION(BlueprintCallable)
		bool ShouldStartCutsceneOnOverlap() const { return ActionToStart == nullptr; }
	UFUNCTION(BlueprintCallable)
		void StartCutscene(class AThirdPersonPlayerCharacter* PlayerCharacterRef, bool bShouldDestroy = true);
	UFUNCTION()
		void OnTriggerBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
		void OnTriggerBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
public:
	UPROPERTY(EditAnywhere)
		class ULevelSequence* LevelSequenceNeeded;
};
