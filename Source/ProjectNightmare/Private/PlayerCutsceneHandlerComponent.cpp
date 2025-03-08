// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCutsceneHandlerComponent.h"
#include "ThirdPersonPlayerCharacter.h"
#include "CutsceneController.h"
#include "CutsceneTrigger.h"
#include "EnhancedInputComponent.h"

// Sets default values for this component's properties
UPlayerCutsceneHandlerComponent::UPlayerCutsceneHandlerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UPlayerCutsceneHandlerComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerPlayerCharacter = Cast<AThirdPersonPlayerCharacter>(GetOwner());
	// ...
	
}

void UPlayerCutsceneHandlerComponent::FinishedCutscene(FTransform NewPlayerTransform, bool bTeleport, bool bFailedCutscene)
{
	CurrentCutsceneController->Destroy();
	CurrentCutsceneController = nullptr;
	if (bTeleport) {
		OwnerPlayerCharacter->SetActorTransform(NewPlayerTransform);
		OwnerPlayerCharacter->MyPlayerController->SetControlRotation(NewPlayerTransform.Rotator());
	}
	OwnerPlayerCharacter->ChangePlayerCollision(true);
	OwnerPlayerCharacter->ChangePlayerMovement(EMovementMode::MOVE_Walking);
}

bool UPlayerCutsceneHandlerComponent::IsPlayerActionAndActionRequiredEqual(const UInputAction* Action) const
{
	return Action == CurrentOverlapingCutsceneTrigger->ActionToStart;
}

void UPlayerCutsceneHandlerComponent::StartCutscene()
{
	if (!IsInCutsceneTrigger()) return;
	CurrentOverlapingCutsceneTrigger->StartCutscene(OwnerPlayerCharacter);
}
