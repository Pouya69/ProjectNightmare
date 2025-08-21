// Fill out your copyright notice in the Description page of Project Settings.


#include "FlyingMutantCharacter.h"
#include "ThirdPersonPlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

bool AFlyingMutantCharacter::FlyToLocation(const FVector FinalDirection, bool bForce)
{
	// UE_LOG(LogTemp, Warning, TEXT("AAAAAAAAAA"));
	FHitResult HitResult;
	const FVector Start = GetActorLocation();
	FVector End = Start + (FinalDirection * ObstacleCheckInFrontDistance);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	if (GrabbedPlayerCharacterRef)
		QueryParams.AddIgnoredActor(GrabbedPlayerCharacterRef);
	const bool bIsObstacleAhead = GetWorld()->SweepSingleByChannel(HitResult, Start, End, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(CapsuleHalfHeight), QueryParams);
	if (bIsObstacleAhead) {
		End = Start + (GetActorUpVector() * ObstacleCheckInFrontDistance);
		const bool bIsAboveBlocked = GetWorld()->SweepSingleByChannel(HitResult, Start, End, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(CapsuleHalfHeight), QueryParams);
		if (bIsAboveBlocked) {
			End = Start + (GetActorRightVector() * ObstacleCheckInFrontDistance);
			const bool bIsRightBlocked = GetWorld()->SweepSingleByChannel(HitResult, Start, End, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(CapsuleHalfHeight), QueryParams);
			if (bIsRightBlocked) {
				End = Start + (GetActorForwardVector() * ObstacleCheckInFrontDistance);
				const bool bIsFrontBlocked = GetWorld()->SweepSingleByChannel(HitResult, Start, End, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(CapsuleHalfHeight), QueryParams);
				if (bIsFrontBlocked) {
					End = Start + (GetActorForwardVector() * -1 * ObstacleCheckInFrontDistance);
					const bool bShouldTryAgain = GetWorld()->SweepSingleByChannel(HitResult, Start, End, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(CapsuleHalfHeight), QueryParams);
					if (bShouldTryAgain)
						return false;
					AddMovementInput(GetActorForwardVector() * -1, 1.f, false);
				}
				else
					AddMovementInput(GetActorForwardVector(), 1.f, false);
			}
			else
				AddMovementInput(GetActorRightVector(), 1.f, false);
			
		}
		else
			AddMovementInput(GetActorUpVector(), 1.f, false);
	}
	else
		AddMovementInput(FinalDirection, 1.f, false);
	return true;
	// CustomAccel += FinalDirection * GetMovementSpeed(MOVE_Flying);
}

void AFlyingMutantCharacter::GrabPlayer(UObject* PlayerCharacterRef)
{
	AThirdPersonPlayerCharacter* PlayerCharacter = Cast<AThirdPersonPlayerCharacter>(PlayerCharacterRef);
	if (!PlayerCharacter) return;
	GrabbedPlayerCharacterRef = PlayerCharacter;
	// TODO: Fix the SNAP and animation etc.
	// TODO: Timer for releasing player IN CASE OF ERROR
	PlayerCharacter->GrabbedByFlyingMutant(this);
	PlayerCharacter->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, PlayerGrabSocket);
}

void AFlyingMutantCharacter::ReleasePlayer(UObject* PlayerCharacterRef, bool bDidPlayerBreakTheAttack)
{
	AThirdPersonPlayerCharacter* PlayerCharacter = Cast<AThirdPersonPlayerCharacter>(PlayerCharacterRef);
	if (!PlayerCharacter) return;
	if (!bDidPlayerBreakTheAttack)
		PlayerCharacter->ReleasedFromGrab();
	GrabbedPlayerCharacterRef = nullptr;
}

AFlyingMutantCharacter::AFlyingMutantCharacter()
{
}

void AFlyingMutantCharacter::BeginPlay()
{
	Super::BeginPlay();
	CapsuleHalfHeight = GetCapsuleHalfHeight();
	CharacterMovementComp = GetCharacterMovement();
	CharacterMovementComp->SetMovementMode(EMovementMode::MOVE_Flying);
}

void AFlyingMutantCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// DrawDebugSphere(GetWorld(), GetActorLocation(), 20.f, 10, FColor::Blue, false, 5.f);
	// CustomAccel *= CustomDamp;
	// CharacterMovementComp->Velocity += CustomAccel;
}