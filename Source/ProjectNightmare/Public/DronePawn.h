// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "DronePawn.generated.h"

UCLASS()
class PROJECTNIGHTMARE_API ADronePawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ADronePawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	UPROPERTY(EditAnywhere)
		float MaxHealth = 100.f;
	UPROPERTY(EditAnywhere)
		float Health;
	UPROPERTY(EditAnywhere)
		float DroneRadius;

	float GetTotalMassOfCargo() const;
	float GetTotalMassOnTopOfHandledCargo() const;
	UFUNCTION()
		void Look(const FInputActionInstance& ActionInstance);
	UFUNCTION()
		void Grab(const FInputActionInstance& ActionInstance);
	UFUNCTION()
		void Move(const FInputActionInstance& ActionInstance);
	UFUNCTION()
		void MoveZAxis(const FInputActionInstance& ActionInstance);
	UFUNCTION()
		void GoBackToPlayer(const FInputActionInstance& ActionInstance);
	UFUNCTION()
		void ReturnToPlayer(bool bGotDestroyed = false);
	UFUNCTION()
		void Shoot(const FInputActionInstance& ActionInstance);
	UFUNCTION()
		void Aim(const FInputActionInstance& ActionInstance);
	UFUNCTION()
		void OnCollisionWithObject(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(EditAnywhere)
		float DronePlayerCollectDistance = 60.f;

	UFUNCTION(BlueprintCallable)
		void TakeControlOfDrone(class AThirdPersonPlayerCharacter* InPlayerCharacterRef);

	int RocketsLeft;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		bool bIsAiming;

	class AThirdPersonPlayerCharacter* PlayerCharacterRef;

	UPROPERTY(EditAnywhere)
		class UCameraComponent* Camera1stPersonComp;
	UPROPERTY(EditAnywhere)
		class UCameraComponent* Camera3rdPersonComp;
	UPROPERTY(EditAnywhere)
		USkeletalMeshComponent* SkeletalMeshComp;

	UPROPERTY(EditAnywhere)
		float DroneMovementSpeed = 300.f;
	//UPROPERTY(EditAnywhere)
	//	float DroneRollSpeed = 20.f;
	UPROPERTY(EditAnywhere)
		float DamageToCharactersHit = 2.f;
	UPROPERTY(EditAnywhere)
		float DronePushbackForce = 50.f;

	class UPhysicsHandleComponent* PhysicsHandle;
	UFUNCTION(BlueprintCallable)
		bool HasItemGrabbed() const;
	UFUNCTION(BlueprintCallable)
		void ReleaseGrabbedItem();
	UPROPERTY(EditAnywhere)
		float ItemGrabRange = 20.f;
	UPROPERTY(EditAnywhere)
		float SmallItemMass = 20.f;
	UMeshComponent* CurrentComponentInFocusForGrab;
	UPROPERTY(EditAnywhere)
		UMaterialInterface* ItemGrabFocusMaterial;

public:
	// Enhanced Input
	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
		class UInputMappingContext* DroneMappingContext;
	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
		class UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
		class UInputAction* MoveZAction;
	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
		class UInputAction* GrabAction;
	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
		class UInputAction* LookAction;
	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
		class UInputAction* AimAction;
	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
		class UInputAction* ShootAction;
	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
		class UInputAction* ReturnToPlayerAction;
};
