// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "SpecialAbilityHandlerComponent.generated.h"


UENUM(Blueprintable)
enum class ESpecialAbilityType : uint8 {
	LAUNCH,
	FREEZE_TIME,
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTNIGHTMARE_API USpecialAbilityHandlerComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USpecialAbilityHandlerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	void SpecialActionStart();
	void SpecialActionHold();
	void SpecialActionRelease();
	UAnimMontage* GetAnimMontageBasedOnCurrentAbility();
	ESpecialAbilityType CurrentAbility;
	
	class AThirdPersonPlayerCharacter* PlayerCharacterRef;

public:
	// Launch
	UFUNCTION(BlueprintCallable, Category="Abilities")
		void Launch();

public:
	// Freeze Time
	FTimerHandle FreezeTimeTimerHandle;
	UPROPERTY(EditAnywhere, Category="Freeze Time")
		float FreezeTimeAmount = 5.f;
	UFUNCTION(BlueprintCallable, Category = "Abilities")
		void FreezeTime();

public:
	// Animations
	UPROPERTY(EditAnywhere, Category = "Animation Montages")
		UAnimMontage* LaunchMontage;
	UPROPERTY(EditAnywhere, Category = "Animation Montages")
		UAnimMontage* FreezeTimeMontage;
};
