// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerComboComponent.generated.h"

UENUM(Blueprintable)
enum class ECombatComboType : uint8
{
	LIGHT = 0,
	HEAVY,
	KNOCK,
	RIFLE,
	ROCKET,
	NOT_COMBO_TYPE
};

UENUM(Blueprintable)
enum class EComboResultType : uint8
{
	HeavyShotShotgun,
	BlastShot
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable )
class PROJECTNIGHTMARE_API UPlayerComboComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPlayerComboComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Anim Montages
	UPROPERTY(EditAnywhere, Category = "Combo Anims")
		UAnimMontage* HeavyShotAnimMontage;
	UPROPERTY(EditAnywhere, Category = "Combo Anims")
		UAnimMontage* HeavyShotMovingAnimMontage;
	UPROPERTY(EditAnywhere, Category = "Combo Anims")
		UAnimMontage* BlastShotAnimMontage;
	UPROPERTY(EditAnywhere, Category = "Combo Anims")
		UAnimMontage* BlastShotMovingAnimMontage;

public:
	FTimerHandle ResetCombatComboTimerHandle;
	// Used to reset the combo for the timer.
	UPROPERTY(EditAnywhere, Category = "Shoot Combo")
		float ResetCombatComboAfterSeconds = 1.f;
	UPROPERTY(EditAnywhere, Category = "Shoot Combo | Heavy Shot")
		TArray<ECombatComboType> CombatComboShotgunHeavyShot;
	bool bIsHeavyShotOkay = true;
	bool bIsBlastShotOkay = true;
	UPROPERTY(EditAnywhere, Category = "Shoot Combo | Heavy Shot")
		bool bDoesComboResetAfterHeavyShot = false;
	UPROPERTY(EditAnywhere, Category = "Shoot Combo | Blast Shot")
		TArray<ECombatComboType> CombatComboBlastShot;
	UPROPERTY(EditAnywhere, Category = "Shoot Combo | Blast Shot")
		bool bDoesComboResetAfterBlastShot = false;
	UPROPERTY(BlueprintReadOnly, Category = "Shoot Combo")
		TArray<ECombatComboType> CurrentShootCombo;
	UPROPERTY(EditAnywhere, Category = "Shoot Combo")
		int CombatComboLimit = 6;
	UFUNCTION(BlueprintCallable, Category = "Shoot Combo")
		UAnimMontage* CurrentShootComboAdd(ECombatComboType CombatType, float VelocityLengthXY = 0);
	UFUNCTION(BlueprintCallable, Category = "Shoot Combo")
		void ResetCombatCombo();
	UFUNCTION(BlueprintCallable, Category = "Shoot Combo")
		void SetResetTimer();
	UPROPERTY(EditAnywhere, Category = "Ultimate")
		float UltimateRaycastRange = 10000.f;
	UPROPERTY(EditAnywhere, Category="Ultimate")
		TSubclassOf<class AUltimateAoESphere> UltimateSphereClass;
	// Used for AoE and preview.
	UPROPERTY(BlueprintReadOnly, Category="Ultimate")
		class AUltimateAoESphere* UltimateSphere;
	UPROPERTY(BlueprintReadOnly, Category = "Ultimate")
		bool bIsOkayToShoot = false;
	void Ultimate_Start(const AActor* PlayerRef, const FVector& StartPoint, const FVector& Direction);
	void Ultimate_Shoot();
	void Ultimate_Ongoing(float& DeltaTime, const AActor* PlayerRef, const FVector& StartPoint, const FVector& Direction);

		
public:	
	UPROPERTY(BlueprintReadOnly, Category="Reload Combo")
		int TimesReloadedBeforeReset = 0;

	// Called every frame
	// virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
