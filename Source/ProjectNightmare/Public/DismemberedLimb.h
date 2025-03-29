// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DismemberedLimb.generated.h"

UCLASS()
class PROJECTNIGHTMARE_API ADismemberedLimb : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADismemberedLimb();

	UPROPERTY(EditAnywhere)
		class ULimbDismemberment* DismembermentComp;

	// USceneComponent* RootSceneComp;

	UPROPERTY(EditAnywhere)
		USkeletalMeshComponent* SkeletalMesh;


	//UPROPERTY(EditAnywhere)
		//class UPoseableMeshComponent* PoseableMeshComp;

	UPROPERTY(EditAnywhere)
		class UNiagaraComponent* BloodNiagaraComp;

	// For checking for destroying
	UPROPERTY(EditAnywhere)
		FName LimbRoot;

	// Max shots taken before it explodes/disappears
	UPROPERTY(EditAnywhere)
		int RootLimbMaxShotThreshold = 2;
	int RootShotAmount = 0;

	UFUNCTION(BlueprintCallable)
		void ApplyDismembermentToLimb(const FName& BoneName, FVector Impulse, FVector HitLocation, bool bForced = false);

	void DisableBloodParticles();
	// void AttachNiagaraComponentToBone(const FName& BoneName);
	void SetBloodNiagaraWorldTransform(const FTransform Transform);

	void EnablePhysics(bool bEnable);
	void SetMesh(USkeletalMesh* Mesh);
	void SetBoneTransformOfMesh(const FName& BoneName, const FTransform& Transform);
	void HideBoneByName(const FName& BoneName);
	void HideBonesAlreadyHiddenInParent(const USkeletalMeshComponent* Mesh, const FName& BoneName);

	UFUNCTION()
		void OnLimbDestroyed(AActor* DestroyedActor);

	UFUNCTION()
		virtual void TakePointDamage(AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation, class UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser);
	UFUNCTION()
		virtual void TakeRadialDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, FVector Origin, const FHitResult& HitInfo, class AController* InstigatedBy, AActor* DamageCauser);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
