// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "LimbDismemberment.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "kismet/KismetRenderingLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ACharacterBase::ACharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SceneCaptureComp = CreateDefaultSubobject<USceneCaptureComponent2D>(FName("Scene Capture 2D Comp"));
	SceneCaptureComp->ProjectionType = ECameraProjectionMode::Orthographic;
	SceneCaptureComp->OrthoWidth = 1024;
	SceneCaptureComp->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	SceneCaptureComp->bCaptureEveryFrame = false;
	SceneCaptureComp->bCaptureOnMovement = false;
	SceneCaptureComp->SetupAttachment(GetRootComponent());
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
}

// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();
	Health = MaxHealth;
	OnTakePointDamage.AddDynamic(this, &ACharacterBase::TakePointDamage);
	OnTakeRadialDamage.AddDynamic(this, &ACharacterBase::TakeRadialDamage);
	DismembermentComp = FindComponentByClass<ULimbDismemberment>();
	OnDestroyed.AddDynamic(this, &ACharacterBase::OnDestroyedDeath);

	Damage_RT = UKismetRenderingLibrary::CreateRenderTarget2D(GetWorld(), 1024, 1024);
	UMaterialInstanceDynamic* CreatedMDI = GetMesh()->CreateDynamicMaterialInstance(0, GetMesh()->GetMaterial(0));
	GetMesh()->SetMaterial(0, CreatedMDI);
	CreatedMDI->SetTextureParameterValue(FName("RT_Damage"), Damage_RT);

	SceneCaptureComp->ShowOnlyActors.Empty();
	SceneCaptureComp->ShowOnlyActors.Add(this);
	SceneCaptureComp->TextureTarget = Damage_RT;
	InitPhysicsSetup();
}

// Called every frame
void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bIsRagdolling && IsAlive())
		UpdateRagdollState();
	// SceneCaptureComp->SetWorldRotation(FRotator(-90, 0, 0));

}

void ACharacterBase::TakePointDamage(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType, AActor* DamageCauser)
{
	PaintBlood(HitLocation, BloodSplatterOnBulletHit);
	if (!IsAlive()) return;
	ReduceHealth(Damage);
	UE_LOG(LogTemp, Warning, TEXT("New Health BULLET: %f"), Health);
	// TODO: Effects and animation
}

void ACharacterBase::TakeRadialDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, FVector Origin, const FHitResult& HitInfo, AController* InstigatedBy, AActor* DamageCauser)
{
	PaintBlood(HitInfo.ImpactPoint, BloodSplatterOnBulletHit);
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
	//if (Controller)
		//Controller->Destroy();
	Health = 0;
	// SetActorTickEnabled(false);
	
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	// GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);


	// GetMesh()->SetAnimInstanceClass(nullptr);
	// GetMesh()->SetBodySimulatePhysics(FName("root"), true);
	// GetMesh()->SetSimulatePhysics(true);

	// Death Montage plays when shot.
	//if (DeathMontage)
		//PlayAnimMontage(DeathMontage);
	OnDeath.Broadcast();
	
}

void ACharacterBase::ReduceHealth(float Amount)
{
	Health -= Amount;
	if (Health <= 0) Die();
}

void ACharacterBase::HitByWeapon(FVector HitLocation, FVector HitNormal, float WeaponBaseDamage)
{
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

FVector ACharacterBase::GetCharacterVelocity() const
{
	return GetCharacterMovement()->Velocity;
}

void ACharacterBase::SetCharacterVelocity(const FVector NewVelocity) const
{
	GetCharacterMovement()->Velocity = NewVelocity;
}

void ACharacterBase::UpdateRagdollState()
{
	bShouldGetUpFromFront_ANIMATION_ONLY = ShouldGetUpFromFront();
	const FVector PelvisLocation = GetMesh()->GetSocketLocation(FName("pelvis"));
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	const bool bResult = GetWorld()->LineTraceSingleByChannel(HitResult, PelvisLocation, PelvisLocation + FVector(0, 0, -100), ECollisionChannel::ECC_Visibility, Params);
	const FVector TargetGroundLocation = bResult ? HitResult.ImpactPoint : PelvisLocation;
	GetCapsuleComponent()->SetWorldLocation(TargetGroundLocation - PelvisOffset);

	if (bIsRagdolling && bResult && PelvisLocation.Z - TargetGroundLocation.Z < 20.f && !GetCharacterMovement()->IsFalling() && GetVelocity().Length() <= StopRagdollingAfterVelocity) {
		FTimerDelegate MyDelegate;
		MyDelegate.BindLambda([&]() {
			if (bIsMarkedForGettingUp) {
				const FVector MyPelvisLocation = GetMesh()->GetSocketLocation(FName("pelvis"));
				FHitResult MyHitResult;
				FCollisionQueryParams MyParams;
				MyParams.AddIgnoredActor(this);
				const bool MybResult = GetWorld()->LineTraceSingleByChannel(MyHitResult, MyPelvisLocation, MyPelvisLocation + FVector(0, 0, -100), ECollisionChannel::ECC_Visibility, MyParams);
				const FVector MyTargetGroundLocation = MybResult ? MyHitResult.ImpactPoint : MyPelvisLocation;
				if (MybResult && MyPelvisLocation.Z - MyTargetGroundLocation.Z < 20.f && GetVelocity().Length() <= StopRagdollingAfterVelocity) {
					bIsMarkedForGettingUp = false;
					// bIsRagdolling = false;
					StopRagdollingBackToAnimation_FROM_TIMER();
				}
				else {
					bIsMarkedForGettingUp = false;
				}
			}
		});
		if (!bIsMarkedForGettingUp) {
			bIsMarkedForGettingUp = true;
			GetWorldTimerManager().SetTimer(RagdollStopTimer, MyDelegate, StopRagdollingAfterSeconds, false);
		}
	}
	else
		bIsMarkedForGettingUp = false;
}

void ACharacterBase::InitPhysicsSetup()
{
	PelvisOffset = GetMesh()->GetRelativeLocation();
}

void ACharacterBase::StartRagdolling()
{
	
	
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	// GetMesh()->SetAllBodiesSimulatePhysics(true);
	// GetMesh()->SetBodySimulatePhysics(FName("root"), false);
	
	GetMesh()->SetAllBodiesBelowSimulatePhysics(FName("pelvis"), true);
	GetMesh()->SetSimulatePhysics(true);
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(IsAlive() ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	bIsRagdolling = true;
	if (IsAlive())
		UpdateRagdollState();
}

FVector ACharacterBase::GetPelvisLocation() const
{
	return GetMesh()->GetSocketLocation(FName("pelvis"));
}

void ACharacterBase::StopRagdollingBackToAnimation()
{
	if (!bIsRagdolling || bIsMarkedForGettingUp) return;
	UE_LOG(LogTemp, Warning, TEXT("GETTING UPPPPP"));
	// GetWorldTimerManager().SetTimerForNextTick(this, &ACharacterBase::TakeSnapshotOfBodyPosition);
	
	const FVector PelvisLocation = GetMesh()->GetSocketLocation(FName("pelvis"));
	// UpdateRagdollState();
	const FRotator NewRotation = UKismetMathLibrary::MakeRotFromZX(FVector::UpVector, (GetMesh()->GetSocketLocation(FName("head")) - PelvisLocation)
		* (bShouldGetUpFromFront_ANIMATION_ONLY ? -1 : 1));
	GetCapsuleComponent()->SetWorldRotation(NewRotation);

	
	
	GetWorldTimerManager().SetTimerForNextTick(this, &ACharacterBase::TakeSnapShot);
	GetWorldTimerManager().SetTimerForNextTick(this, &ACharacterBase::AfterSnapShot);
}

void ACharacterBase::StopRagdollingBackToAnimation_FROM_TIMER()
{
	if (!bIsRagdolling || bIsMarkedForGettingUp) return;
	const FVector PelvisLocation = GetMesh()->GetSocketLocation(FName("pelvis"));
	// UpdateRagdollState();
	const FRotator NewRotation = UKismetMathLibrary::MakeRotFromZX(FVector::UpVector, (GetMesh()->GetSocketLocation(FName("head")) - PelvisLocation)
		* (bShouldGetUpFromFront_ANIMATION_ONLY ? -1 : 1));
	Controller->SetControlRotation(NewRotation);
	GetCapsuleComponent()->SetWorldRotation(NewRotation);

	GetWorldTimerManager().SetTimerForNextTick(this, &ACharacterBase::TakeSnapShot);
	GetWorldTimerManager().SetTimerForNextTick(this, &ACharacterBase::AfterSnapShot);
}

bool ACharacterBase::ShouldGetUpFromFront() const
{
	return UKismetMathLibrary::GetRightVector(GetMesh()->GetSocketRotation(FName("pelvis"))).Z > 0;
}

void ACharacterBase::AfterSnapShot()
{
	bIsRagdolling = false;
	GetMesh()->SetAllBodiesSimulatePhysics(false);
	GetMesh()->SetSimulatePhysics(false);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void ACharacterBase::TakeSnapShot()
{
	GetMesh()->GetAnimInstance()->SavePoseSnapshot(FName("RagdollFinalSnapShot"));
}

void ACharacterBase::AddImpulseToCharacter(const FVector& Impulse)
{
	GetMesh()->AddImpulse(GetMesh()->GetMass() * Impulse / 3);
}


void ACharacterBase::PlayHitReactionMontage(const float SignedAngle)
{
	if (ReactHitMontageAdditive_Back && SignedAngle < -90 && SignedAngle > 90) // From Back
		PlayAnimMontage(ReactHitMontageAdditive_Back);
	//else if (SignedAngle < 0 && SignedAngle > -90) // Right
	else if (ReactHitMontageAdditive_Front && SignedAngle < 90 && SignedAngle > -90)  // Front
		PlayAnimMontage(ReactHitMontageAdditive_Front);
	else if (ReactHitMontageAdditive_Right && SignedAngle < 90 && SignedAngle > 0)  // Right
		PlayAnimMontage(ReactHitMontageAdditive_Right);
	else if (ReactHitMontageAdditive_Left && SignedAngle > -90 && SignedAngle < 0)  // Left
		PlayAnimMontage(ReactHitMontageAdditive_Left);
	else
		PlayAnimMontage(ReactHitMontageAdditive_Front);
}

void ACharacterBase::ApplyDismembermentToLimb(const FName& BoneName, FVector Impulse, FVector HitLocation, bool bForced)
{
	if (!DismembermentComp || BoneName.IsNone()) return;
	const bool bIsHead = BoneName.IsEqual(DismembermentComp->Head) || BoneName.IsEqual(DismembermentComp->Neck);
	const bool bResult = DismembermentComp->ApplyDismembermentToLimb(BoneName, Impulse, HitLocation, bForced);
	if (IsAlive() && bResult) {
		StopMyMovement();
		if (bIsHead) {
			bIsMarkedForDeath = true;
			if (bIsCrawling)
				Die();
		}
		else
			ApplyEpicEffect(0.2, HitLocation, 0.5f, true, true, false);
	}
	
		
	
	//if (GetMesh()->BoneIsChildOf(BoneName, RightHandArmBoneName)) {
	//	const FTransform BoneTransform = GetMesh()->GetBoneTransform(RightHandArmBoneName);
	//	GetMesh()->HideBoneByName(RightHandArmBoneName, EPhysBodyOp::PBO_Term);
	//	// TODO: Spawn the dismemberment
	//}


}

void ACharacterBase::OnDestroyedDeath(AActor* DestroyedActor)
{
	if (DismembermentComp)
		DismembermentComp->DeleteAllBloodParticles();
}

void ACharacterBase::StartCrawling()
{
	bIsCrawling = true;
	Crouch();
	GetCharacterMovement()->MaxWalkSpeedCrouched = CrawlingMovementSpeed;
	GetCharacterMovement()->MaxWalkSpeed = CrawlingMovementSpeed;
	/*
	FTimerDelegate MyDelegate;
	MyDelegate.BindLambda([&]() {
		GetMesh()->SetSimulatePhysics(false);
		bIsCrawling = true;
	});
	GetWorldTimerManager().SetTimer(CrawlingTimer, MyDelegate, StartCrawlAfterRagdollInSeconds, false);
	*/
}

void ACharacterBase::PaintBlood(const FVector& ImpactPoint, const float Radius)
{
	// DrawDebugSphere(GetWorld(), ImpactPoint, 10, 30, FColor::Blue, true);
	OldMaterial = GetMesh()->GetMaterial(0);
	GetMesh()->SetMaterial(0, UnwrapMaterial);
	GetMesh()->SetVectorParameterValueOnMaterials(FName("CaptureLocation"), RootComponent->GetComponentLocation());
	GetMesh()->SetVectorParameterValueOnMaterials(FName("HitLocation"), ImpactPoint);
	GetMesh()->SetScalarParameterValueOnMaterials(FName("ImpactRadius"), Radius);
	SceneCaptureComp->CaptureScene();
	GetMesh()->SetMaterial(0, OldMaterial);
	// UKismetRenderingLibrary::DrawMaterialToRenderTarget(GetWorld(), HitRT, OriginalMID);
	
}

void ACharacterBase::StandingEventDone()
{
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
}

void ACharacterBase::ApplyEpicEffect(float TimeDilationAmount, FVector Location, float Duration, bool bIsAttached, bool bPlayNiagara, bool bSlowDownPlayer, float PlayerSlowdownCustomRate)
{
	if (bPlayNiagara) {
		if (bIsAttached)
			UNiagaraFunctionLibrary::SpawnSystemAttached(SlowMotionNiagaraEffect, GetMesh(), FName(), GetActorLocation(), FRotator::ZeroRotator, EAttachLocation::KeepWorldPosition, true);
		else
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), SlowMotionNiagaraEffect, Location);

	}
		
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), TimeDilationAmount);
	FTimerDelegate MyDelegate;
	if (!bSlowDownPlayer) {
		UGameplayStatics::GetPlayerPawn(GetWorld(), 0)->CustomTimeDilation = 1 / TimeDilationAmount;
		MyDelegate.BindLambda([&]() {
			UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.f);
			UGameplayStatics::GetPlayerPawn(GetWorld(), 0)->CustomTimeDilation = 1.f;
		});
	}
	else {
		MyDelegate.BindLambda([&]() {
			UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.f);
		});
	}
	GetWorldTimerManager().SetTimer(EpicEffectTimerHandle, MyDelegate, Duration, false);
}

void ACharacterBase::StopMyMovement()
{
}



float ACharacterBase::GetMovementSpeed(EMovementMode InMovementMode) const
{
	switch (InMovementMode)
	{
	case MOVE_None:
		break;
	case MOVE_Walking:
		return GetCharacterMovement()->MaxWalkSpeed;
		break;
	case MOVE_Swimming:
		return GetCharacterMovement()->MaxSwimSpeed;
		break;
	case MOVE_Flying:
		return GetCharacterMovement()->MaxFlySpeed;
		break;
	default:
		break;
	}
	return 0.0f;
}