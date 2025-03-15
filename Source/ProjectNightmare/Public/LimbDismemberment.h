// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LimbDismemberment.generated.h"


USTRUCT(BlueprintType)
struct FDismembermentLimb : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USkeletalMesh* LimbSkeletalMesh;
};

UENUM(BlueprintType)
enum class EDismemberType : uint8 {
	HEAD,
	Torso,
	RightArm,
	LeftArm,
	RightLeg,
	LeftLeg,
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTNIGHTMARE_API ULimbDismemberment : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULimbDismemberment();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	UFUNCTION(BlueprintCallable, Category = "Dismemberment")
		void ApplyDismembermentToLimb(const FName& BoneName, FVector Impulse, FVector HitLocation);

	UPROPERTY(EditAnywhere, Category = "Dismemberment")
		UDataTable* LimbsDataTable;

	UPROPERTY(EditAnywhere, Category = "Dismemberment")
		float DismembermentForce = 300.f;
		
	UPROPERTY(EditAnywhere, Category = "Dismemberment")
		class UNiagaraSystem* BloodNiagara;
	UPROPERTY(EditAnywhere, Category = "Dismemberment")
		TSubclassOf<class ADismemberedLimb> DismemberedLimbClass;

	TArray<class UNiagaraComponent*> SpawnedParticles;



	UFUNCTION(BlueprintCallable, Category = "Dismemberment")
		ADismemberedLimb* SpawnLimbByBoneName(const FName& LimbName, const FTransform& RootTransform, const FTransform& LimbTransform, bool bWasShot = true);
	class ACharacterBase* OwmerCharacter;
	class ADismemberedLimb* OwnerIndividualLimb;

	// For individual limbs when they are not attached to the body so we can destroy them individually
	USkeletalMeshComponent* Mesh;

	void DeleteAllBloodParticles();


	bool bIsLeftLegGone = false;
	bool bIsRightLegGone = false;
	bool bIsRightArmGone = false;
	bool bIsLeftArmGone = false;
	bool bIsBodyDetachedFromPelvis = false;
	bool bIsHeadGone = false;

	UPROPERTY(EditAnywhere, Category = "Animations")
		bool bShouldPlayDismembermentAnimations = true;
	UPROPERTY(EditAnywhere, Category = "Animations")
		UAnimMontage* LeftArmDismembermentAnim;
	UPROPERTY(EditAnywhere, Category = "Animations")
		UAnimMontage* RightArmDismembermentAnim;
	UPROPERTY(EditAnywhere, Category = "Animations")
		UAnimMontage* HeadDismembermentAnim;
	UPROPERTY(EditAnywhere, Category = "Animations")
		UAnimMontage* TorsoDismembermentAnim;
	UPROPERTY(EditAnywhere, Category = "Animations")
		UAnimMontage* RightLegDismembermentAnim;
	UPROPERTY(EditAnywhere, Category = "Animations")
		UAnimMontage* LeftLegDismembermentAnim;


	UPROPERTY(EditAnywhere, Category="Limb Names | RightArm")
		FName RightHand = FName("hand_r");
	UPROPERTY(EditAnywhere, Category = "Limb Names | RightArm")
		FName RightLowerArm = FName("lowerarm_r");
	UPROPERTY(EditAnywhere, Category = "Limb Names | RightArm")
		FName RightUpperArm = FName("upperarm_r");
	UPROPERTY(EditAnywhere, Category = "Limb Names | RightArm")
		FName RightClavicle = FName("clavicle_r");

	UPROPERTY(EditAnywhere, Category = "Limb Names | LeftArm")
		FName LeftHand = FName("hand_l");
	UPROPERTY(EditAnywhere, Category = "Limb Names | LeftArm")
		FName LeftLowerArm = FName("lowerarm_l");
	UPROPERTY(EditAnywhere, Category = "Limb Names | LeftArm")
		FName LeftUpperArm = FName("upperarm_l");
	UPROPERTY(EditAnywhere, Category = "Limb Names | LeftArm")
		FName LeftClavicle = FName("clavicle_l");
	
	UPROPERTY(EditAnywhere, Category = "Limb Names | Head")
		FName Neck = FName("neck_01");
	UPROPERTY(EditAnywhere, Category = "Limb Names | Head")
		FName Head = FName("head");

	UPROPERTY(EditAnywhere, Category = "Limb Names | RightLeg")
		FName RightFoot = FName("foot_r");
	UPROPERTY(EditAnywhere, Category = "Limb Names | RightLeg")
		FName RightCalf = FName("calf_r");
	UPROPERTY(EditAnywhere, Category = "Limb Names | RightLeg")
		FName RightThigh = FName("thigh_r");

	UPROPERTY(EditAnywhere, Category = "Limb Names | LeftLeg")
		FName LeftFoot = FName("foot_l");
	UPROPERTY(EditAnywhere, Category = "Limb Names | LeftLeg")
		FName LeftCalf = FName("calf_l");
	UPROPERTY(EditAnywhere, Category = "Limb Names | LeftLeg")
		FName LeftThigh = FName("thigh_l");

	UPROPERTY(EditAnywhere, Category = "Limb Names | Torso")
		FName Pelvis = FName("pelvis");
	UPROPERTY(EditAnywhere, Category = "Limb Names | Torso")
		FName Spine1 = FName("spine_01");
	UPROPERTY(EditAnywhere, Category = "Limb Names | Torso")
		FName Spine2 = FName("spine_02");
	UPROPERTY(EditAnywhere, Category = "Limb Names | Torso")
		FName Spine3 = FName("spine_03");
		
};
