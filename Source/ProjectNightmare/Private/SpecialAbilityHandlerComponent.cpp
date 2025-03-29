// Fill out your copyright notice in the Description page of Project Settings.


#include "SpecialAbilityHandlerComponent.h"
#include "ThirdPersonPlayerCharacter.h"

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
	// ...
	
}


// Called every frame
void USpecialAbilityHandlerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void USpecialAbilityHandlerComponent::SpecialActionStart()
{
	UAnimMontage* AbilityToPlay = GetAnimMontageBasedOnCurrentAbility();
	if (AbilityToPlay) {
		PlayerCharacterRef->PlayAnimMontage(AbilityToPlay);
	}
}

void USpecialAbilityHandlerComponent::SpecialActionHold()
{
}

void USpecialAbilityHandlerComponent::SpecialActionRelease()
{
}

void USpecialAbilityHandlerComponent::Launch()
{
	// TODO: Launch logic
	// PlayerCharacterRef->EnableInput(PlayerCharacterRef->MyPlayerController);
}

UAnimMontage* USpecialAbilityHandlerComponent::GetAnimMontageBasedOnCurrentAbility()
{
	switch (CurrentAbility)
	{
	case ESpecialAbilityType::LAUNCH:
		return LaunchMontage;
		break;
	default:
		break;
	}
	return nullptr;
}

