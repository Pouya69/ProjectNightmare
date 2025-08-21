// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractableObject.h"
#include "ThirdPersonPlayerCharacter.h"
#include "InteractionWidget.h"
#include "Components/WidgetComponent.h"

// Sets default values
AInteractableObject::AInteractableObject()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	RootSceneComp = CreateDefaultSubobject<USceneComponent>(FName("Root"));
	SetRootComponent(RootSceneComp);
	InteractUserWidgetComp = CreateDefaultSubobject<UWidgetComponent>(FName("Interaction UI Widget Comp"));
	InteractUserWidgetComp->SetupAttachment(GetRootComponent());

}

// Called when the game starts or when spawned
void AInteractableObject::BeginPlay()
{
	Super::BeginPlay();
	InteractUserWidgetComp = FindComponentByClass<UWidgetComponent>();
	// if (InteractUserWidgetComp == nullptr) return;
	if (InteractUserWidgetComp) {
		InteractionWidget = Cast<UInteractionWidget>(InteractUserWidgetComp->GetWidget());
		if (InteractionWidget) {
			InteractionWidget->SetVisibility(ESlateVisibility::Hidden);
			InteractionWidget->Percent = &InteractionPercent;
		}
	}
	StaticMeshComp = FindComponentByClass<UStaticMeshComponent>();
}

void AInteractableObject::SetFocusMaterial(bool bIsFocused)
{
	if (StaticMeshComp)
		StaticMeshComp->SetOverlayMaterial(bIsFocused ? FocusedMaterial : nullptr);
}

void AInteractableObject::Interact(int Value)
{
}

void AInteractableObject::HoldInteract(AThirdPersonPlayerCharacter* PlayerCharacterRef, const float DeltaTime)
{
	InteractionPercent = FMath::FInterpConstantTo(InteractionPercent, bIsInteracting ? 100.f : 0.f, DeltaTime, bIsInteracting ? InteractionRate : (InteractionRate * 3));
	// InteractionWidget->SetInteractionPercent(InteractionPercent);
	if (InteractionPercent == 100.f) {
		InteractionComplete(PlayerCharacterRef);
		InteractionComplete_Finished_Event(PlayerCharacterRef);
		PlayerCharacterRef->StopInteract();
	}
}

void AInteractableObject::SetInRange(bool bInRange)
{
	if (!InteractionWidget) return;
	InteractionWidget->SetVisibility(bInRange ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	InteractionWidget->SetInRange(bInRange);
}

void AInteractableObject::SetIsFocused(bool bIsFocused)
{
	if (!FocusedMaterial) return;
	SetFocusMaterial(bIsFocused);
	if (bIsFocused) InteractionWidget->StartFocus();
}

void AInteractableObject::StartInteraction()
{
	bIsInteracting = true;
}

void AInteractableObject::StopInteraction()
{
	bIsInteracting = false;
}

void AInteractableObject::InteractionComplete(class AThirdPersonPlayerCharacter* PlayerCharacterRef)
{
	InteractionPercent = 0.f;
	StopInteraction();
}

void AInteractableObject::InteractionComplete()
{
	InteractionPercent = 0.f;
	StopInteraction();
}

void AInteractableObject::InteractionComplete_Finished_Event_Implementation(AThirdPersonPlayerCharacter* PlayerCharacterRef)
{
}
