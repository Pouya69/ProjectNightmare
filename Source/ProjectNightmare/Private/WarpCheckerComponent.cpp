// Fill out your copyright notice in the Description page of Project Settings.


#include "WarpCheckerComponent.h"
#include "ThirdPersonPlayerCharacter.h"
#include "LocationSwapper.h"

// Sets default values for this component's properties
UWarpCheckerComponent::UWarpCheckerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UWarpCheckerComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<AThirdPersonPlayerCharacter>(GetOwner());
	
}


// Called every frame
void UWarpCheckerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!OwnerCharacter || OwnerCharacter->GetController() == nullptr) {
		if (CurrentWarpInFocus) {
			CurrentWarpInFocus->SetInFocus(false);
			CurrentWarpInFocus = nullptr;
		}
		return;
	}
	FHitResult HitResult;
	const FVector Start = OwnerCharacter->GetActorLocation();
	const bool bIsHit = GetWorld()->SweepSingleByChannel(HitResult, Start, Start + (OwnerCharacter->GetCameraLookingDirection() * DistanceCheck), FQuat::Identity, ECC_GameTraceChannel2, FCollisionShape::MakeSphere(RadiusCheck));
	if (!bIsHit) {
		if (CurrentWarpInFocus) {
			CurrentWarpInFocus->SetInFocus(false);
			CurrentWarpInFocus = nullptr;
		}
		return;
	}
	CurrentWarpInFocus = Cast<ALocationSwapper>(HitResult.GetActor());
	CurrentWarpInFocus->SetInFocus(true);
}

bool UWarpCheckerComponent::SwapLocation()
{
	if (CurrentWarpInFocus == nullptr) return false;
	CurrentWarpInFocus->SwapLocation(OwnerCharacter);
	return true;
}

