// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CameraLockOn.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTNIGHTMARE_API UCameraLockOn : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCameraLockOn();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	UPROPERTY(EditAnywhere, Category = "Lock On System")
		float LockOnNextTargetCooldownInSeconds = 1.f;
	UPROPERTY(EditAnywhere, Category = "Lock On System")
		float StartLockOnScanRadius = 800.f;
	UPROPERTY(EditAnywhere, Category = "Lock On System")
		float StartLockOnScanDistance = 1200.f;
	UPROPERTY(EditAnywhere, Category = "Lock On System")
		float StopLockOnAfterNotInSightInSeconds = 2.5f;
	UPROPERTY(EditAnywhere, Category = "Lock On System")
		float LockOnCameraRotationSpeed = 100.f;
	UPROPERTY(EditAnywhere, Category = "Lock On System")
		float LockOnSwitchForceThreshold = 100.f;
	UPROPERTY(BlueprintReadOnly, Category = "Lock On System")
		class AEnemyBaseCharacter* EnemyCharacterLockedOn;
	UFUNCTION(BlueprintCallable, Category="Camera Lock On")
		void HandleCameraLockOn(const FVector2D& LookForce, const float& DeltaTime);
	UFUNCTION(BlueprintCallable, Category = "Camera Lock On")
		bool IsLockedOn() const { return EnemyCharacterLockedOn != nullptr; }
	UFUNCTION(BlueprintCallable, Category = "Camera Lock On")
		FVector GetDirectionToTarget() const;
	UFUNCTION(BlueprintCallable, Category = "Camera Lock On")
		bool IsTargetBlocked() const;
		bool IsTargetBlocked(class AEnemyBaseCharacter* TargetToScan) const;
	UFUNCTION(BlueprintCallable, Category = "Camera Lock On")
		bool StartLockOn();
	UFUNCTION(BlueprintCallable, Category = "Camera Lock On")
		void StopLockOn();
	void StopLockOn_FORCED();
	FTimerHandle StopLockOnTimerHandle;
	FTimerHandle LockOnNextTargetCooldownTimerHandle;
	class AThirdPersonPlayerCharacter* OwnerPlayerRef;
};
