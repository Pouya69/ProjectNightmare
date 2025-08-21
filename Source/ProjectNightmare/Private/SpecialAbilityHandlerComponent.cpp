// Fill out your copyright notice in the Description page of Project Settings.


#include "SpecialAbilityHandlerComponent.h"
#include "ThirdPersonPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
USpecialAbilityHandlerComponent::USpecialAbilityHandlerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void USpecialAbilityHandlerComponent::BeginPlay()
{
	Super::BeginPlay();
	PlayerCharacterRef = Cast<AThirdPersonPlayerCharacter>(GetOwner());
	CurrentAbility = ESpecialAbilityType::FREEZE_TIME;
	// ...
	
}


// Called every frame
void USpecialAbilityHandlerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
}

void USpecialAbilityHandlerComponent::SpecialActionStart()
{
	PlayerCharacterRef->ResetCameraPostEffects();
	UAnimMontage* AbilityToPlay = GetAnimMontageBasedOnCurrentAbility();
	if (AbilityToPlay) {
		PlayerCharacterRef->PlayAnimMontage(AbilityToPlay);
		CurrentAbilityCompletionAmount = 0.f;
		CurrentTargetCompletionSeconds = AbilityToPlay->GetPlayLength();
	}
	
}

void USpecialAbilityHandlerComponent::HandleCameraPostEffects(const float* DeltaTime, FPostProcessSettings* CameraPostProcess)
{
	if (bIsMarkedForReset) {
		HandleResettingCameraPostEffects(DeltaTime, CameraPostProcess);
	}
	else {
		if (CurrentTargetCompletionSeconds <= 0) return;
		CurrentAbilityCompletionAmount = FMath::FInterpConstantTo(CurrentAbilityCompletionAmount, 100.f, *DeltaTime, 100.f / CurrentTargetCompletionSeconds);

	}
	switch (CurrentAbility)
	{
		case ESpecialAbilityType::LAUNCH:
			break;
		case ESpecialAbilityType::FREEZE_TIME:
			HandleFreezeTimeCameraPostProcess(CameraPostProcess);
			break;
		default:
			break;
	}
	
	// CurrentAbilityCompletionAmount = MapRangeClamped(CurrentAbilityCompletionAmount, 0.f, 100.f, 0.f, CurrentTargetCompletionSeconds);
}

void USpecialAbilityHandlerComponent::HandleResettingCameraPostEffects(const float* DeltaTime, FPostProcessSettings* CameraPostProcess)
{
	CurrentAbilityCompletionAmount = FMath::FInterpConstantTo(CurrentAbilityCompletionAmount, 0.f, *DeltaTime, CameraPostEffectResetSpeed);
	if (CurrentAbilityCompletionAmount <= 0) {
		PlayerCharacterRef->ResetCameraPostEffects();
		bIsMarkedForReset = false;
	}
}

float USpecialAbilityHandlerComponent::MapRangeClamped(float Value, float InRangeA, float InRangeB, float OutRangeA, float OutRangeB) const
{
	return FMath::Lerp(OutRangeA, OutRangeB, (FMath::Clamp(Value, InRangeA, InRangeB) - InRangeA) / (InRangeB - InRangeA));
}

void USpecialAbilityHandlerComponent::SpecialActionHold()
{
}

void USpecialAbilityHandlerComponent::SpecialActionRelease()
{
	// CurrentAbilityCompletionAmount = 0.f;
	// CurrentTargetCompletionSeconds = 0.f;
}

void USpecialAbilityHandlerComponent::Launch()
{
	// TODO: Launch logic
	// PlayerCharacterRef->EnableInput(PlayerCharacterRef->MyPlayerController);
}

void USpecialAbilityHandlerComponent::HandleFreezeTimeCameraPostProcess(FPostProcessSettings* CameraPostProcess)
{
	CameraPostProcess->bOverride_VignetteIntensity = true;
	CameraPostProcess->VignetteIntensity = MapRangeClamped(CurrentAbilityCompletionAmount, 0.f, 100.f, 0.4f, FreezeTimeVignetteMax);
	LensSettings.ChromaticAberration = MapRangeClamped(CurrentAbilityCompletionAmount, 0.f, 100.f, 0.f, FreezeTimeChromaticAberrationMax);
	LensSettings.ExportToPostProcessSettings(CameraPostProcess);
	PlayerCharacterRef->PlayCameraShake(FreezeTimeProgressCameraShake);
}

void USpecialAbilityHandlerComponent::FreezeTime()
{
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.1f);
	PlayerCharacterRef->CustomTimeDilation = 1 / 0.1f;
	// FTimerDelegate MyDelegate;
	
	/*MyDelegate.BindLambda([&]() {
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.f);
		PlayerCharacterRef->CustomTimeDilation = 1.f;
		PlayerCharacterRef->PlayerCameraManager->StopAllCameraShakes();
	});
	PlayerCharacterRef->GetWorldTimerManager().SetTimer(FreezeTimeTimerHandle, MyDelegate, FreezeTimeAmount * 0.0001f, false);*/
	FTransform PortalTransform = PlayerCharacterRef->LastOverworldTransform;
	if (!PlayerCharacterRef->bIsInFourthDimension) {
		FHitResult HitResult;
		FVector SpawnLocationTypical = PlayerCharacterRef->GetActorLocation() + (PlayerCharacterRef->GetActorForwardVector() * PlayerCharacterRef->DroneSpawnDistanceFromPlayer);
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(PlayerCharacterRef);
		bool bCloseToWall = GetWorld()->SweepSingleByChannel(HitResult, PlayerCharacterRef->GetActorLocation(), SpawnLocationTypical, FQuat::Identity, ECC_WorldStatic, FCollisionShape::MakeSphere(80.f), QueryParams);
		if (bCloseToWall) {
			SpawnLocationTypical = PlayerCharacterRef->GetActorLocation() + (PlayerCharacterRef->GetActorUpVector() * PlayerCharacterRef->DroneSpawnDistanceFromPlayer);
			bCloseToWall = GetWorld()->SweepSingleByChannel(HitResult, PlayerCharacterRef->GetActorLocation(), SpawnLocationTypical, FQuat::Identity, ECC_WorldStatic, FCollisionShape::MakeSphere(80.f), QueryParams);
			if (bCloseToWall) {
				// TODO: notification cannot deploy drone
				return;
			}
		}

		FHitResult HitResultFinal;
		const bool bHit = GetWorld()->LineTraceSingleByChannel(HitResultFinal, SpawnLocationTypical, SpawnLocationTypical + (FVector::DownVector * 300.f), ECC_WorldStatic, QueryParams);
		if (!bHit) return;
		PortalTransform = FTransform((PlayerCharacterRef->GetActorForwardVector() * -1).Rotation(), HitResultFinal.ImpactPoint, FVector::OneVector);
	}
	PlayerCharacterRef->OpenFourthDimension_Implementation(PortalTransform);
	PlayerCharacterRef->OpenFourthDimension(PortalTransform);
}

UAnimMontage* USpecialAbilityHandlerComponent::GetAnimMontageBasedOnCurrentAbility()
{
	switch (CurrentAbility)
	{
	case ESpecialAbilityType::LAUNCH:
		return LaunchMontage;
		break;
	case ESpecialAbilityType::FREEZE_TIME:
		return FreezeTimeMontage;
		break;
	default:
		break;
	}
	return nullptr;
}

