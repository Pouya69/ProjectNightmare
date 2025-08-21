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

	float CurrentTargetCompletionSeconds = 0.f;
	float CurrentAbilityCompletionAmount = 0.f;

	void HandleCameraPostEffects(const float* DeltaTime, FPostProcessSettings* CameraPostProcess);
	void HandleResettingCameraPostEffects(const float* DeltaTime, FPostProcessSettings* CameraPostProcess);
	bool bIsMarkedForReset = false;

	FORCEINLINE float MapRangeClamped(float Value, float InRangeA, float InRangeB, float OutRangeA, float OutRangeB) const;


public:
	// Launch
	UFUNCTION(BlueprintCallable, Category="Abilities")
		void Launch();

public:
	// Freeze Time
	FTimerHandle FreezeTimeTimerHandle;
	void HandleFreezeTimeCameraPostProcess(FPostProcessSettings* CameraPostProcess);
	UPROPERTY(EditAnywhere, Category="Freeze Time")
		float FreezeTimeAmount = 5.f;
	UFUNCTION(BlueprintCallable, Category = "Abilities")
		void FreezeTime();

public:
	FLensSettings LensSettings;
	// Camera Post Process
	UPROPERTY(EditAnywhere, Category = "Camera Post Process")
		float CameraPostEffectResetSpeed = 0.f;
	UPROPERTY(EditAnywhere, Category = "Camera Post Process | Freeze Time")
		TSubclassOf<UCameraShakeBase> FreezeTimeProgressCameraShake;
	UPROPERTY(EditAnywhere, Category = "Camera Post Process | Freeze Time")
		float FreezeTimeChromaticAberrationMax = 50.f;
	UPROPERTY(EditAnywhere, Category = "Camera Post Process | Freeze Time")
		float FreezeTimeVignetteMax = 100.f;
		

public:
	// Animations
	UPROPERTY(EditAnywhere, Category = "Animation Montages")
		UAnimMontage* LaunchMontage;
	UPROPERTY(EditAnywhere, Category = "Animation Montages")
		UAnimMontage* FreezeTimeMontage;
};
