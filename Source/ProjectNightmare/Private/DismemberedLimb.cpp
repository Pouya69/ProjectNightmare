// Fill out your copyright notice in the Description page of Project Settings.


#include "DismemberedLimb.h"
#include "LimbDismemberment.h"
#include "NiagaraComponent.h"

// Sets default values
ADismemberedLimb::ADismemberedLimb()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	// RootSceneComp = CreateDefaultSubobject<USceneComponent>(FName("Root Comp"));
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(FName("Mesh"));
	SetRootComponent(SkeletalMesh);

	
	// SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(FName("Mesh"));
	// SkeletalMesh->SetupAttachment(GetRootComponent());
	// SkeletalMesh->SetSimulatePhysics(true);
	// SkeletalMesh->SetSimulatePhysics(true);
	SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SkeletalMesh->SetCollisionObjectType(ECC_PhysicsBody);
	SkeletalMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	SkeletalMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	// SkeletalMesh->SetupAttachment(GetRootComponent());

	BloodNiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(FName("Blood Niagara Comp"));
	BloodNiagaraComp->SetupAttachment(SkeletalMesh);

	DismembermentComp = CreateDefaultSubobject<ULimbDismemberment>(FName("Dismemberment Comp"));

	InitialLifeSpan = 20.f;

}

void ADismemberedLimb::ApplyDismembermentToLimb(const FName& BoneName, FVector Impulse, FVector HitLocation, bool bForced)
{
	if (!DismembermentComp) return;
	DismembermentComp->ApplyDismembermentToLimb(BoneName, Impulse, HitLocation);
}

void ADismemberedLimb::DisableBloodParticles()
{
	BloodNiagaraComp->DeactivateImmediate();
}

//void ADismemberedLimb::AttachNiagaraComponentToBone(const FName& BoneName)
//{
//	BloodNiagaraComp->AttachToComponent(SkeletalMesh, FAttachmentTransformRules::KeepRelativeTransform, );
//}

void ADismemberedLimb::SetBloodNiagaraWorldTransform(const FTransform Transform)
{
	BloodNiagaraComp->SetWorldTransform(Transform);
	BloodNiagaraComp->AttachToComponent(SkeletalMesh, FAttachmentTransformRules::KeepWorldTransform);
}

void ADismemberedLimb::EnablePhysics(bool bEnable)
{
	SkeletalMesh->SetAllBodiesSimulatePhysics(true);
	SkeletalMesh->SetSimulatePhysics(bEnable);
}

void ADismemberedLimb::SetMesh(USkeletalMesh* Mesh)
{
	SkeletalMesh->SetSkeletalMesh(Mesh);
}

void ADismemberedLimb::SetBoneTransformOfMesh(const FName& BoneName, const FTransform& Transform)
{
	// SkeletalMesh->SetBoneTransformByName(BoneName, Transform, EBoneSpaces::WorldSpace);
}

void ADismemberedLimb::HideBoneByName(const FName& BoneName)
{
	SkeletalMesh->HideBoneByName(BoneName, EPhysBodyOp::PBO_Term);
}

void ADismemberedLimb::HideBonesAlreadyHiddenInParent(const USkeletalMeshComponent* Mesh, const FName& BoneName)
{
	const int NumBones = SkeletalMesh->GetNumBones();
	const int PartIndex = SkeletalMesh->GetBoneIndex(BoneName);
	for (int i = 0; i < NumBones; i++)
	{
		if (!Mesh->IsBoneHidden(i)) continue;

		SkeletalMesh->HideBone(i, EPhysBodyOp::PBO_Term);
	}
}

void ADismemberedLimb::OnLimbDestroyed(AActor* DestroyedActor)
{
	if (DismembermentComp)
		DismembermentComp->DeleteAllBloodParticles();

	// TODO: Explosion blood effect
}

void ADismemberedLimb::TakePointDamage(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType, AActor* DamageCauser)
{
	ApplyDismembermentToLimb(BoneName, FVector::ZeroVector, HitLocation);
}

void ADismemberedLimb::TakeRadialDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, FVector Origin, const FHitResult& HitInfo, AController* InstigatedBy, AActor* DamageCauser)
{
	// ApplyDismembermentToLimb(HitInfo.BoneName);
}

// Called when the game starts or when spawned
void ADismemberedLimb::BeginPlay()
{
	Super::BeginPlay();
	OnDestroyed.AddDynamic(this, &ADismemberedLimb::OnLimbDestroyed);
	OnTakePointDamage.AddDynamic(this, &ADismemberedLimb::TakePointDamage);
	OnTakeRadialDamage.AddDynamic(this, &ADismemberedLimb::TakeRadialDamage);
	SkeletalMesh->SetAllBodiesSimulatePhysics(true);
}
