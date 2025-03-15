// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "LimbDismemberment.h"

// Sets default values
ACharacterBase::ACharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();
	Health = MaxHealth;
	OnTakePointDamage.AddDynamic(this, &ACharacterBase::TakePointDamage);
	OnTakeRadialDamage.AddDynamic(this, &ACharacterBase::TakeRadialDamage);
	DismembermentComp = FindComponentByClass<ULimbDismemberment>();
	//OnTakeAnyDamage.AddDynamic(this, &ACharacterBase::TakeAnyDamage);
	// OnTakeAnyDamage.AddDynamic(this, &ACharacterBase::TakeDamage);
}

// Called every frame
void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACharacterBase::TakePointDamage(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType, AActor* DamageCauser)
{
	if (!IsAlive()) return;
	ReduceHealth(Damage);
	UE_LOG(LogTemp, Warning, TEXT("New Health BULLET: %f"), Health);
	// TODO: Effects and animation
}

void ACharacterBase::TakeRadialDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, FVector Origin, const FHitResult& HitInfo, AController* InstigatedBy, AActor* DamageCauser)
{
	if (!IsAlive()) return;
	ReduceHealth(Damage);
	UE_LOG(LogTemp, Warning, TEXT("New Health: %f"), Health);
	// TODO: Effects and animation
}

float ACharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	//ReduceHealth(ActualDamage);
	//UE_LOG(LogTemp, Warning, TEXT("New Health TEST: %f"), Health);
	return ActualDamage;
}

void ACharacterBase::TakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	ReduceHealth(Damage);
	UE_LOG(LogTemp, Warning, TEXT("ANY New Health: %f"), Health);
}

void ACharacterBase::AddHealth(float Amount)
{
	Health = FMath::Clamp(Health+Amount, 0, MaxHealth);
}

float ACharacterBase::GetCharacterMass() const
{
	return GetCharacterMovement()->Mass;
}

void ACharacterBase::Die()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	// GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
	DisableInput(GetLocalViewingPlayerController());
	GetMesh()->SetSimulatePhysics(true);
	OnDeath.Broadcast();
	
}

void ACharacterBase::ReduceHealth(float Amount)
{
	Health -= Amount;
	if (Health <= 0) Die();
}

void ACharacterBase::HitByWeapon(FVector HitLocation, FVector HitNormal, float WeaponBaseDamage)
{
	// TODO: Calculate Damage Deault based on hitlocation
	// FHitResult HitResult;
	// const bool bIsHitHead = GetWorld()->SweepSingleByChannel(HitResult, HitLocation, HitLocation, FQuat::Identity, ECollisionChannel)
	ReduceHealth(WeaponBaseDamage);
}

float ACharacterBase::GetCapsuleRadius() const
{
	return GetCapsuleComponent()->GetScaledCapsuleRadius();
}

float ACharacterBase::GetCapsuleHalfHeight() const
{
	return GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}


void ACharacterBase::ApplyDismembermentToLimb(const FName& BoneName, FVector Impulse, FVector HitLocation)
{
	if (!DismembermentComp || BoneName.IsNone()) return;
	DismembermentComp->ApplyDismembermentToLimb(BoneName, Impulse, HitLocation);
	
	//if (GetMesh()->BoneIsChildOf(BoneName, RightHandArmBoneName)) {
	//	const FTransform BoneTransform = GetMesh()->GetBoneTransform(RightHandArmBoneName);
	//	GetMesh()->HideBoneByName(RightHandArmBoneName, EPhysBodyOp::PBO_Term);
	//	// TODO: Spawn the dismemberment
	//}


}
