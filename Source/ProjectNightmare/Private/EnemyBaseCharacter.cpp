// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBaseCharacter.h"
#include "EnemyBaseAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"


AEnemyBaseCharacter::AEnemyBaseCharacter()
{
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AEnemyBaseCharacter::BeginPlay()
{

	Super::BeginPlay();
	EnemyAIController = Cast<AEnemyBaseAIController>(GetController());
	if (EnemyAIController)
		EnemyAIController->EnemyOwner = this;
}

void AEnemyBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AEnemyBaseCharacter::EnemySpawned()
{
	// EnemyAIController->GetBrainComponent()->StopLogic(FString());
	// StopMyMovement();
	bSpawned = true;
	if (JustSpawnedAnim)
		PlayAnimMontage(JustSpawnedAnim);
}

void AEnemyBaseCharacter::SpawnFinished()
{
	EnemyAIController->bSpawned = true;
	EnemyAIController->EnemyOwner = this;
	EnemyAIController->RunBehaviorTree(EnemyAIController->EnemyBehaviour);
	GetWorldTimerManager().SetTimerForNextTick(this, &AEnemyBaseCharacter::InitPhysicsSetup);
	GetWorldTimerManager().SetTimerForNextTick(this, &AEnemyBaseCharacter::UpdateRagdollState);
	//InitPhysicsSetup();
	//UpdateRagdollState();
}

bool AEnemyBaseCharacter::AttackPlayer()
{
	if (AttackMontage)
		PlayAnimMontage(AttackMontage);
	return true;
}

void AEnemyBaseCharacter::StunStart(const float StunTimer, const bool bShouldStopAnims)
{
	if (GetWorldTimerManager().IsTimerActive(StunTimerHandle)) return;  // Already stunned.
	UBlackboardComponent* BlackboardComp = EnemyAIController->GetBlackboardComponent();
	if (!BlackboardComp) return;
	if (bShouldStopAnims) {
		StopAnimMontage();
		if (ReactHitMontage_Front) PlayAnimMontage(ReactHitMontage_Front);
	}
	BlackboardComp->SetValueAsBool(FName("IsStunned"), true);
	GetWorldTimerManager().SetTimer(StunTimerHandle, this, &AEnemyBaseCharacter::StopStun, StunTimer, false);
}

void AEnemyBaseCharacter::StopStun()
{
	UBlackboardComponent* BlackboardComp = EnemyAIController->GetBlackboardComponent();
	if (!BlackboardComp) return;
	BlackboardComp->SetValueAsBool(FName("IsStunned"), false);
}

void AEnemyBaseCharacter::PlayerParryAllow(const bool bIsAllowed)
{
	bIsTryingToDamagePlayer = bIsAllowed;
}

bool AEnemyBaseCharacter::CanPlayerParryMyAttack() const
{
	return bIsTryingToDamagePlayer;
}

void AEnemyBaseCharacter::PlayerStartMeleeMe()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemyBaseCharacter::PlayerStartParryMe()
{
}

void AEnemyBaseCharacter::PlayerMeleeEvent(const float Damage)
{
	ReduceHealth(Damage);
}

void AEnemyBaseCharacter::PlayerParryEvent(const float Damage)
{
	StunStart(ParryStunTimerInSeconds, true);
	ReduceHealth(Damage);
}

void AEnemyBaseCharacter::PlayerMeleeFinished()
{
	if (!IsAlive()) return;
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void AEnemyBaseCharacter::PlayerParryFinished()
{
	if (!IsAlive()) return;
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void AEnemyBaseCharacter::Die()
{
	// Controller->UnPossess();
	Super::Die();
	StartRagdolling();
	GetMesh()->AddImpulse(GetActorForwardVector() * -4.f + FVector(0,0,4), FName("pelvis"));
	EnemyAIController->EnemyDied();
	SetLifeSpan(DisappearAfterDeathInSeconds);
}

void AEnemyBaseCharacter::TakePointDamage(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType, AActor* DamageCauser)
{
	Super::TakePointDamage(DamagedActor, Damage, InstigatedBy, HitLocation, FHitComponent, BoneName, ShotFromDirection, DamageType, DamageCauser);
	if (IsAlive() && !bIsRagdolling) {
		FVector2D A(-GetActorForwardVector().X, -GetActorForwardVector().Y);
		FVector2D B(ShotFromDirection.X, ShotFromDirection.Y);

		A.Normalize();
		B.Normalize();

		const float SignedAngle = FMath::RadiansToDegrees(FMath::Atan2(A.X * B.Y - A.Y * B.X
			, FVector2D::DotProduct(A, B))); // Returns signed angle in radians
		if (GetWorldTimerManager().IsTimerActive(ReactHitDelayTimerHandle)) {
			// Additive hit
			PlayHitReactionMontage(SignedAngle);
		}
		else {
			// Hit ragdoll or animation. TODO: Not for now.
			if (false && Damage >= DamageThresholdRagdoll) {
				// We will ragdoll if the damage is bigger than the threshold.
				StartRagdolling();
				GetMesh()->AddImpulse(ShotFromDirection * 8.f + FVector(0, 0, 20.f), BoneName);
			}
			else {
				const bool bIsFront = SignedAngle < 90 || SignedAngle > -90;
				if (!bIsFront && bShouldOnlyPlayFrontHit) {
					StopMyMovement();
					PlayHitReactionMontage(SignedAngle);
				}
				else {
					// StopMyMovement();  // We enable it back from the anim events.
					// We will play the hit react animation if the damage is less than the threshold.
					UE_LOG(LogTemp, Warning, TEXT("Angle from me to shot direction: %f"), SignedAngle);
					if (ReactHitMontage_Back && SignedAngle < -90 && SignedAngle > 90) // From Back
						PlayAnimMontage(ReactHitMontage_Back);
					//else if (SignedAngle < 0 && SignedAngle > -90) // Right
					else if (ReactHitMontage_Front && SignedAngle < 90 && SignedAngle > -90)  // Front
						PlayAnimMontage(ReactHitMontage_Front);
					else if (ReactHitMontage_Right && SignedAngle < 90 && SignedAngle > 0)  // Right
						PlayAnimMontage(ReactHitMontage_Right);
					else if (ReactHitMontage_Left && SignedAngle > -90 && SignedAngle < 0)  // Left
						PlayAnimMontage(ReactHitMontage_Left);
					else
						PlayHitReactionMontage(SignedAngle);
				}
			}
		}
		GetWorldTimerManager().SetTimer(ReactHitDelayTimerHandle, ReactHitDelayInSeconds, false);
	}
	ApplyDismembermentToLimb(BoneName, ShotFromDirection, HitLocation);

}

void AEnemyBaseCharacter::TakeRadialDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, FVector Origin, const FHitResult& HitInfo, AController* InstigatedBy, AActor* DamageCauser)
{
	Super::TakeRadialDamage(DamagedActor, Damage, DamageType, Origin, HitInfo, InstigatedBy, DamageCauser);

	ApplyDismembermentToLimb(HitInfo.BoneName, HitInfo.TraceEnd - HitInfo.TraceStart, HitInfo.ImpactPoint, false);
	StartRagdolling();
	//if (Damage >= DamageThresholdRagdoll)
		//StartRagdolling();
}

void AEnemyBaseCharacter::StopMyMovement()
{
	if (!EnemyAIController || !bSpawned) return;
	UBlackboardComponent* BlackboardComp = EnemyAIController->GetBlackboardComponent();
	if (!BlackboardComp) return;
	EnemyAIController->StopMovement();
	// EnemyAIController->MoveToLocation(GetActorLocation());
	BlackboardComp->SetValueAsBool(FName("ShouldRunAI"), false);
	//FTimerDelegate TimerDelegate;
	//TimerDelegate.BindLambda([&]() {
		//EnemyAIController->GetBlackboardComponent()->SetValueAsBool(FName("ShouldRunAI"), true);
	//});
	//GetWorldTimerManager().SetTimer(ShouldRunAITimer, TimerDelegate, 1.5f, false);
	
}

void AEnemyBaseCharacter::AllowAIMovement()
{
	if (!EnemyAIController || !bSpawned || !EnemyAIController->GetBlackboardComponent()) return;
	EnemyAIController->GetBlackboardComponent()->SetValueAsBool(FName("ShouldRunAI"), true);
}

void AEnemyBaseCharacter::AfterSnapShot()
{
	if (!IsAlive()) return;
	bIsRagdolling = false;
	GetMesh()->SetAllBodiesSimulatePhysics(false);
	GetMesh()->SetSimulatePhysics(false);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);  // This is only for enemies.
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void AEnemyBaseCharacter::StopRagdollingBackToAnimation_FROM_TIMER()
{
	if (!bIsRagdolling || bIsMarkedForGettingUp || !IsAlive()) return;
	const FVector PelvisLocation = GetMesh()->GetSocketLocation(FName("pelvis"));
	// UpdateRagdollState();
	//const FRotator NewRotation = UKismetMathLibrary::MakeRotFromZX(FVector::UpVector, (GetMesh()->GetSocketLocation(FName("head")) - PelvisLocation)
		//* (bShouldGetUpFromFront_ANIMATION_ONLY ? -1 : 1));
	// Controller->SetControlRotation(NewRotation);
	//GetCapsuleComponent()->SetWorldRotation(NewRotation);
	// TakeSnapShot();
	GetWorldTimerManager().SetTimerForNextTick(this, &AEnemyBaseCharacter::TakeSnapShot);
	GetWorldTimerManager().SetTimerForNextTick(this, &AEnemyBaseCharacter::AfterSnapShot);
}

void AEnemyBaseCharacter::UpdateRagdollState()
{
	bShouldGetUpFromFront_ANIMATION_ONLY = ShouldGetUpFromFront();
	const FVector PelvisLocation = GetMesh()->GetSocketLocation(FName("pelvis"));
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	const bool bResult = GetWorld()->LineTraceSingleByChannel(HitResult, PelvisLocation, PelvisLocation + FVector(0, 0, -100), ECollisionChannel::ECC_Visibility, Params);
	const FVector TargetGroundLocation = bResult ? HitResult.ImpactPoint : PelvisLocation;
	const FRotator NewRotation = UKismetMathLibrary::MakeRotFromZX(FVector::UpVector, (GetMesh()->GetSocketLocation(FName("head")) - PelvisLocation)
		* (bShouldGetUpFromFront_ANIMATION_ONLY ? -1 : 1));
	GetCapsuleComponent()->SetWorldLocationAndRotation(TargetGroundLocation - PelvisOffset, NewRotation);

	if (IsAlive() && bIsRagdolling && bResult && PelvisLocation.Z - TargetGroundLocation.Z < 20.f && !GetCharacterMovement()->IsFalling() && GetVelocity().Length() <= StopRagdollingAfterVelocity) {
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

void AEnemyBaseCharacter::StartRagdolling()
{
	StopMyMovement();
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	// GetMesh()->SetAllBodiesSimulatePhysics(true);
	// GetMesh()->SetBodySimulatePhysics(FName("root"), false);

	GetMesh()->SetAllBodiesBelowSimulatePhysics(FName("pelvis"), true);
	GetMesh()->SetSimulatePhysics(true);
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(IsAlive() ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	bIsRagdolling = true;
	//if (IsAlive())
		//UpdateRagdollState();
}

void AEnemyBaseCharacter::StandingEventDone()
{
	Super::StandingEventDone();
	AllowAIMovement();
}

void AEnemyBaseCharacter::ChangeAttackComboTypeTo(EEnemyComboType NewComboType)
{
	ResetCombo();
	CurrentComboType = NewComboType;
}

void AEnemyBaseCharacter::ResetCombo(bool bIsDash)
{
	if (bIsDash)
		CurrentDashIndex = 0;
	else
		CurrentComboIndex = 0;
}

void AEnemyBaseCharacter::Dash(UAnimMontage* DashAnimMontage, FVector TargetLocation, FName OverrideMotionWarp, bool bBackwards)
{
}
