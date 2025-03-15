// Fill out your copyright notice in the Description page of Project Settings.


#include "LimbDismemberment.h"
#include "DismemberedLimb.h"
#include "CharacterBase.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "CharacterBase.h"

// Sets default values for this component's properties
ULimbDismemberment::ULimbDismemberment()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void ULimbDismemberment::BeginPlay()
{
	Super::BeginPlay();

	OwmerCharacter = Cast<ACharacterBase>(GetOwner());
	if (OwmerCharacter) {
		Mesh = OwmerCharacter->GetMesh();
	}
	else {
		OwnerIndividualLimb = Cast<ADismemberedLimb>(GetOwner());
		Mesh = OwnerIndividualLimb->SkeletalMesh;
	}
		

	
}

void ULimbDismemberment::ApplyDismembermentToLimb(const FName& BoneName, FVector Impulse, FVector HitLocation)
{
	if (BoneName.IsNone()) {
		UE_LOG(LogTemp, Error, TEXT("LIMB NAME INVALID FOR CHECK. BONE CHECKED: %s"), *BoneName.ToString());
		return;
	}

	// For when we are shooting the individual bone
	if (OwnerIndividualLimb && BoneName.IsEqual(OwnerIndividualLimb->LimbRoot)) {
		OwnerIndividualLimb->RootShotAmount += 1;
		if (OwnerIndividualLimb->RootShotAmount > OwnerIndividualLimb->RootLimbMaxShotThreshold) {
			OwnerIndividualLimb->HideBoneByName(BoneName);
			OwnerIndividualLimb->DisableBloodParticles();
		}
		return;
	}

	if (Mesh->BoneIsChildOf(BoneName, Pelvis) && !Mesh->BoneIsChildOf(BoneName, LeftClavicle) && !Mesh->BoneIsChildOf(BoneName, RightClavicle)) {
		// When shooting upperbody and we are separating the upperbody from lowerbody
		if (OwmerCharacter) {
			// Mesh->SetSkeletalMesh();
			DeleteAllBloodParticles();
		}
	}


	// Mesh->SetBodySimulatePhysics(*LimbName, true);
	FTransform LimbTransform = Mesh->GetBoneTransform(BoneName);
	FTransform RootTransform = Mesh->GetBoneTransform(FName("root"));
	UE_LOG(LogTemp, Warning, TEXT("%s"), *LimbTransform.ToString());
	
	ADismemberedLimb* SpawnedLimb = SpawnLimbByBoneName(BoneName, RootTransform, LimbTransform, true);

	if (!SpawnedLimb) return;
	Mesh->HideBoneByName(BoneName, EPhysBodyOp::PBO_None);
	// TODO: Blood effects
	SpawnedLimb->EnablePhysics(true);
	SpawnedLimb->SkeletalMesh->AddImpulse(Impulse * DismembermentForce);

	// UNiagaraFunctionLibrary::SpawnSystemAttached(BloodNiagara, Mesh, n, loc, rot, EAttachLocation::SnapToTarget, true);
	if (OwmerCharacter && !Mesh->IsSimulatingPhysics()) {
		const FName& ParentBone = Mesh->GetParentBone(BoneName);
		if (!ParentBone.IsNone()) {
			UNiagaraComponent* SpawnedNiagara = UNiagaraFunctionLibrary::SpawnSystemAttached(BloodNiagara, Mesh, ParentBone, LimbTransform.GetTranslation(), LimbTransform.GetRotation().Rotator(), EAttachLocation::KeepWorldPosition, true);
			DeleteAllBloodParticles();
			SpawnedParticles.Add(SpawnedNiagara);
		}
	}
	

	if (!OwmerCharacter || !OwmerCharacter->IsAlive()) return;
	// Play dismemberment animation

	UAnimMontage* AnimMontageToPlay = nullptr;

	if (Mesh->BoneIsChildOf(BoneName, LeftClavicle)) {
		// LeftArm;
		bIsLeftArmGone = true;
		if (bIsRightArmGone) {
			// AnimMontageToPlay = LeftArmDismembermentAnim;
		}
		else
			AnimMontageToPlay = LeftArmDismembermentAnim;
	}
	else if (Mesh->BoneIsChildOf(BoneName, LeftThigh) || BoneName.IsEqual(LeftThigh)) {
		// LeftLeg;
		bIsLeftLegGone = true;
		if (bIsRightLegGone) {
			DeleteAllBloodParticles();
			Mesh->SetSimulatePhysics(true);
		}
		else
			AnimMontageToPlay = LeftLegDismembermentAnim;
	}
	else if (Mesh->BoneIsChildOf(BoneName, RightClavicle)) {
		// RightArm;
		bIsRightArmGone = true;
		if (bIsLeftArmGone) {
			// AnimMontageToPlay = LeftArmDismembermentAnim;
		}
		else
			AnimMontageToPlay = RightArmDismembermentAnim;
	}
	else if (Mesh->BoneIsChildOf(BoneName, RightThigh) || BoneName.IsEqual(RightThigh)) {
		// RightLeg;
		bIsRightLegGone = true;
		if (bIsLeftLegGone) {
			// Mesh->SetSimulatePhysics(true);
		}
		else
			AnimMontageToPlay = RightLegDismembermentAnim;
	}
	else if (Mesh->BoneIsChildOf(BoneName, Neck) || BoneName.IsEqual(Head) || BoneName.IsEqual(Neck)) {
		// HEAD;
		bIsHeadGone = true;
		if (bIsRightArmGone && bIsLeftArmGone) {

		}
		else
			AnimMontageToPlay = HeadDismembermentAnim;
	}
	else {
		// Torso
		// TODO: Will not work for now because GetAnimInstance() does not have Animation Blueprint
		bIsBodyDetachedFromPelvis = true;
		if (bShouldPlayDismembermentAnimations)
			SpawnedLimb->SkeletalMesh->GetAnimInstance()->Montage_Play(TorsoDismembermentAnim);
	}

	SpawnedLimb->DismembermentComp->bIsHeadGone = bIsHeadGone;
	SpawnedLimb->DismembermentComp->bIsBodyDetachedFromPelvis = bIsBodyDetachedFromPelvis;
	SpawnedLimb->DismembermentComp->bIsRightArmGone = bIsRightArmGone;
	SpawnedLimb->DismembermentComp->bIsLeftArmGone = bIsLeftArmGone;
	SpawnedLimb->DismembermentComp->bIsRightLegGone = bIsRightLegGone;
	SpawnedLimb->DismembermentComp->bIsLeftLegGone = bIsLeftLegGone;

	if (bShouldPlayDismembermentAnimations && AnimMontageToPlay)
		Mesh->GetAnimInstance()->Montage_Play(AnimMontageToPlay);

}

ADismemberedLimb* ULimbDismemberment::SpawnLimbByBoneName(const FName& LimbName, const FTransform& RootTransform, const FTransform& LimbTransform, bool bWasShot)
{
	if (!LimbsDataTable) return nullptr;
	FDismembermentLimb* LimbStruct = LimbsDataTable->FindRow<FDismembermentLimb>(LimbName, "");
	if (LimbStruct == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("LIMB NAME INVALID FOR LIMB SPAWN: %s"), *LimbName.ToString());
		return nullptr;
	}
	ADismemberedLimb* SpawnedLimb = GetWorld()->SpawnActorDeferred<ADismemberedLimb>(DismemberedLimbClass, RootTransform);
	if (!SpawnedLimb) return nullptr;
	SpawnedLimb->SetMesh(LimbStruct->LimbSkeletalMesh);
	if (bWasShot)
		SpawnedLimb->RootShotAmount = 1;
	SpawnedLimb->LimbRoot = LimbName;
	SpawnedLimb->HideBonesAlreadyHiddenInParent(Mesh, LimbName);
	// SpawnedLimb->SetBoneTransformOfMesh(LimbName, LimbTransform);
	// SpawnedLimb->AttachNiagaraComponentToBone(LimbName);
	SpawnedLimb->SetBloodNiagaraWorldTransform(LimbTransform);
	SpawnedLimb->FinishSpawning(RootTransform);
	return SpawnedLimb;
}

void ULimbDismemberment::DeleteAllBloodParticles()
{
	if (!SpawnedParticles.IsEmpty()) {
		for (int i = 0; i < SpawnedParticles.Num(); i++) {
			UNiagaraComponent* Particle = SpawnedParticles[i];
			SpawnedParticles.RemoveAt(i);
			Particle->DestroyComponent();
		}
	}
}
