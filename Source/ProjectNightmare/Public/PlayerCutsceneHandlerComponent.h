// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerCutsceneHandlerComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class PROJECTNIGHTMARE_API UPlayerCutsceneHandlerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPlayerCutsceneHandlerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	UPROPERTY(BlueprintReadWrite)
		class ACutsceneController* CurrentCutsceneController;
	class AThirdPersonPlayerCharacter* OwnerPlayerCharacter;
	
	UFUNCTION(BlueprintCallable)
		bool IsInCutscene() const { return CurrentCutsceneController != nullptr; }
	UFUNCTION(BlueprintCallable)
		void FinishedCutscene(FTransform NewPlayerTransform, bool bTeleport, bool bFailedCutscene);
	UFUNCTION(BlueprintCallable)
		void SetInCutsceneTrigger(class ACutsceneTrigger* NewCutsceneTrigger) { CurrentOverlapingCutsceneTrigger = NewCutsceneTrigger; }
	UFUNCTION(BlueprintCallable)
		bool IsInCutsceneTrigger() const { return CurrentOverlapingCutsceneTrigger != nullptr; }
	UFUNCTION(BlueprintCallable)
		bool IsPlayerActionAndActionRequiredEqual(const class UInputAction* Action) const;
	UFUNCTION(BlueprintCallable)
		void StartCutscene();

private:
	class ACutsceneTrigger* CurrentOverlapingCutsceneTrigger;
		
};
