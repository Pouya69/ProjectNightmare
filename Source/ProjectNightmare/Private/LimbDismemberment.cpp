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

bool ULimbDismemberment::ApplyDismembermentToLimb(const FName& BoneName, const FVector& Impulse, const FVector& HitLocation, bool bForced)
{
	if (BoneName.IsNone()) {
		UE_LOG(LogTemp, Error, TEXT("LIMB NAME INVALID FOR CHECK. BONE CHECKED: %s"), *BoneName.ToString());
		return false;
	}
	if (!bForced && FMath::RandRange(1, OneInNumChanceDismemberment) != 1)
		return false;


	if (OwmerCharacter && OwmerCharacter->bIsMarkedForDeath)
		OwmerCharacter->Die();

	// For when we are shooting the individual bone
	if (OwnerIndividualLimb && BoneName.IsEqual(OwnerIndividualLimb->LimbRoot)) {
		OwnerIndividualLimb->RootShotAmount += 1;
		if (OwnerIndividualLimb->RootShotAmount > OwnerIndividualLimb->RootLimbMaxShotThreshold) {
			OwnerIndividualLimb->HideBoneByName(BoneName);
			OwnerIndividualLimb->DisableBloodParticles();

		}
		return true;
	}

	const FTransform LimbTransform = Mesh->GetBoneTransform(BoneName);
	const FTransform RootTransform = Mesh->IsSimulatingPhysics() ? Mesh->GetBoneTransform(FName("pelvis")) : Mesh->GetBoneTransform(FName("root"));
	const bool bIsUpperBodyHit = BoneName.IsEqual(Pelvis) || BoneName.IsEqual(Spine1) || BoneName.IsEqual(Spine2) || BoneName.IsEqual(Spine3) || BoneName.IsEqual(Spine1);
	// Separating body parts
	if (BoneName.IsEqual(RightThigh) || BoneName.IsEqual(RightCalf) || BoneName.IsEqual(RightFoot))
		bIsLeftLegGone = true;
	else if (BoneName.IsEqual(LeftThigh) || BoneName.IsEqual(LeftCalf) || BoneName.IsEqual(LeftFoot))
		bIsRightLegGone = true;
	if (OwmerCharacter && OwmerCharacter->IsAlive() && !OwmerCharacter->bIsCrawling && (bIsRightLegGone || bIsLeftLegGone || bIsUpperBodyHit)) {
		OwmerCharacter->StopAnimMontage();
		// OwmerCharacter->StopMyMovement();
		// UE_LOG(LogTemp, Warning, TEXT("Separate"));
		// When shooting upperbody and we are separating the upperbody from lowerbody
			// Mesh->SetSkeletalMesh();
		DeleteAllBloodParticles();
		if (bIsLeftLegGone || bIsRightLegGone) {
			// OwmerCharacter->StopMyMovement();
			ADismemberedLimb* SpawnedLimb = SpawnLimbByBoneName(BoneName, RootTransform, LimbTransform, true);
			if (!SpawnedLimb) return false;
			Mesh->HideBoneByName(BoneName, EPhysBodyOp::PBO_Term);
			// TODO: Blood effects
			SpawnedLimb->EnablePhysics(true);
			SpawnedLimb->SkeletalMesh->AddImpulse(Impulse * DismembermentForce);
			UNiagaraComponent* SpawnedNiagara = UNiagaraFunctionLibrary::SpawnSystemAttached(BloodNiagara, Mesh, BoneName, LimbTransform.GetTranslation(), LimbTransform.GetRotation().Rotator(), EAttachLocation::KeepWorldPosition, true);

		}
		else if (!bIsLeftLegGone && !bIsRightLegGone) {
			const FTransform RightLegTransform = Mesh->GetBoneTransform(RightThigh);
			const FTransform LeftLegTransform = Mesh->GetBoneTransform(LeftThigh);
			ADismemberedLimb* RightLegSpawned = SpawnLimbByBoneName(RightThigh, RootTransform, RightLegTransform, false);
			RightLegSpawned->DismembermentComp->bIsHeadGone = bIsHeadGone;
			RightLegSpawned->DismembermentComp->bIsBodyDetachedFromPelvis = bIsBodyDetachedFromPelvis;
			RightLegSpawned->DismembermentComp->bIsRightArmGone = bIsRightArmGone;
			RightLegSpawned->DismembermentComp->bIsLeftArmGone = bIsLeftArmGone;
			RightLegSpawned->DismembermentComp->bIsRightLegGone = bIsRightLegGone;
			RightLegSpawned->DismembermentComp->bIsLeftLegGone = bIsLeftLegGone;
			RightLegSpawned->EnablePhysics(true);

			ADismemberedLimb* LeftLegSpawned = SpawnLimbByBoneName(LeftThigh, RootTransform, LeftLegTransform, false);
			
			LeftLegSpawned->DismembermentComp->bIsBodyDetachedFromPelvis = bIsBodyDetachedFromPelvis;
			LeftLegSpawned->DismembermentComp->bIsRightArmGone = bIsRightArmGone;
			LeftLegSpawned->DismembermentComp->bIsLeftArmGone = bIsLeftArmGone;
			LeftLegSpawned->DismembermentComp->bIsRightLegGone = bIsRightLegGone;
			LeftLegSpawned->DismembermentComp->bIsLeftLegGone = bIsLeftLegGone;
			LeftLegSpawned->EnablePhysics(true);

			LeftLegSpawned->SkeletalMesh->SetBodySimulatePhysics(FName("root"), true);
			RightLegSpawned->SkeletalMesh->SetBodySimulatePhysics(FName("root"), true);

			UNiagaraFunctionLibrary::SpawnSystemAttached(BloodNiagara, Mesh, RightThigh, RightLegTransform.GetTranslation(), RightLegTransform.GetRotation().Rotator(), EAttachLocation::KeepWorldPosition, true);
			UNiagaraFunctionLibrary::SpawnSystemAttached(BloodNiagara, Mesh, LeftThigh, LeftLegTransform.GetTranslation(), LeftLegTransform.GetRotation().Rotator(), EAttachLocation::KeepWorldPosition, true);
			
			Mesh->HideBoneByName(RightThigh, EPhysBodyOp::PBO_Term);
			Mesh->HideBoneByName(LeftThigh, EPhysBodyOp::PBO_Term);
			bIsRightLegGone = true;
			bIsLeftLegGone = true;
		}

		/*if (bIsRightLegGone) {
			
		}
			
		if (bIsLeftLegGone) {
			
		}*/
		
		

		// Mesh->SetSimulatePhysics(true);
		OwmerCharacter->StartCrawling();
		bShouldPlayDismembermentAnimations = false;

		// Mesh->AddImpulse(Impulse, Pelvis);
		return true;
	}
	if (OwmerCharacter && bIsUpperBodyHit && OwmerCharacter->bIsCrawling) {
		return true;
	}

	// Mesh->SetBodySimulatePhysics(*LimbName, true);
	
	UE_LOG(LogTemp, Warning, TEXT("%s"), *LimbTransform.ToString());
	
	ADismemberedLimb* SpawnedLimb = SpawnLimbByBoneName(BoneName, Mesh->IsSimulatingPhysics() ? Mesh->GetBoneTransform(FName("pelvis")) : Mesh->GetBoneTransform(FName("root")), Mesh->GetBoneTransform(BoneName), true);

	if (!SpawnedLimb) return false;
	Mesh->HideBoneByName(BoneName, EPhysBodyOp::PBO_Term);
	// TODO: Blood effects
	SpawnedLimb->SkeletalMesh->SetBodySimulatePhysics(FName("root"), true);
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

	if (!OwmerCharacter || !OwmerCharacter->IsAlive()) return true;
	// Play dismemberment animation

	UAnimMontage* AnimMontageToPlay = nullptr;

	if (Mesh->BoneIsChildOf(BoneName, LeftClavicle) || BoneName.IsEqual(LeftHand) || BoneName.IsEqual(LeftLowerArm)) {
		// LeftArm;
		bIsLeftArmGone = true;
		if (bIsRightArmGone) {
			// AnimMontageToPlay = LeftArmDismembermentAnim;
		}
		else
			AnimMontageToPlay = LeftArmDismembermentAnim;
		if (!OwmerCharacter->bIsCrawling)
			OwmerCharacter->StopMyMovement();
	}
	/*else if (BoneName.IsEqual(LeftThigh) || BoneName.IsEqual(LeftCalf) || BoneName.IsEqual(LeftFoot)) {
		// LeftLeg;
		bIsLeftLegGone = true;
		if (bIsRightLegGone) {
			DeleteAllBloodParticles();
			Mesh->SetSimulatePhysics(true);
		}
		else
			AnimMontageToPlay = LeftLegDismembermentAnim;
	}*/
	else if (Mesh->BoneIsChildOf(BoneName, RightClavicle) || BoneName.IsEqual(RightHand) || BoneName.IsEqual(RightLowerArm)) {
		// RightArm;
		bIsRightArmGone = true;
		if (bIsLeftArmGone) {
			// AnimMontageToPlay = LeftArmDismembermentAnim;
		}
		else
			AnimMontageToPlay = RightArmDismembermentAnim;
		if (!OwmerCharacter->bIsCrawling)
			OwmerCharacter->StopMyMovement();
	}
	/*else if (BoneName.IsEqual(RightThigh) || BoneName.IsEqual(RightCalf) || BoneName.IsEqual(RightFoot)) {
		// RightLeg;
		bIsRightLegGone = true;
		if (bIsLeftLegGone) {
			DeleteAllBloodParticles();
			Mesh->SetSimulatePhysics(true);
		}
		else
			AnimMontageToPlay = RightLegDismembermentAnim;
	}*/
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

	return true;

}

ADismemberedLimb* ULimbDismemberment::SpawnLimbByBoneName(const FName& LimbName, const FTransform& RootTransform, const FTransform& LimbTransform, bool bWasShot)
{
	if (!LimbsDataTable) return nullptr;
	FDismembermentLimb* LimbStruct = LimbsDataTable->FindRow<FDismembermentLimb>(LimbName, "");
	if (LimbStruct == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("LIMB NAME INVALID FOR LIMB SPAWN: %s"), *LimbName.ToString());
		return nullptr;
	}
	ADismemberedLimb* SpawnedLimb = GetWorld()->SpawnActorDeferred<ADismemberedLimb>(DismemberedLimbClass, OwmerCharacter != nullptr ? RootTransform : LimbTransform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!SpawnedLimb) return nullptr;
	SpawnedLimb->SetMesh(LimbStruct->LimbSkeletalMesh);
	if (bWasShot)
		SpawnedLimb->RootShotAmount = 1;
	SpawnedLimb->LimbRoot = LimbName;
	SpawnedLimb->HideBonesAlreadyHiddenInParent(Mesh, LimbName);
	// SpawnedLimb->SetBoneTransformOfMesh(LimbName, LimbTransform);
	// SpawnedLimb->AttachNiagaraComponentToBone(LimbName);
	SpawnedLimb->SkeletalMesh->SetMaterial(0, Mesh->GetMaterial(0));
	SpawnedLimb->SetBloodNiagaraWorldTransform(LimbTransform);
	SpawnedLimb->FinishSpawning(OwmerCharacter != nullptr ? RootTransform : LimbTransform);
	return SpawnedLimb;
}

void ULimbDismemberment::DeleteAllBloodParticles()
{
	if (!SpawnedParticles.IsEmpty()) {
		for (int i = 0; i < SpawnedParticles.Num(); i++) {
			UNiagaraComponent* Particle = SpawnedParticles[i];
			SpawnedParticles.RemoveAt(i);
			Particle->DeactivateImmediate();
			Particle->DestroyComponent();
		}
	}
	if (OwnerIndividualLimb)
		OwnerIndividualLimb->DisableBloodParticles();
}
