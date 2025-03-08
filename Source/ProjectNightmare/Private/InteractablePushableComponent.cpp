// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractablePushableComponent.h"
#include "InteractableObject.h"

void UInteractablePushableComponent::BeginPlay()
{
	InteractableOwner = Cast<AInteractableObject>(GetOwner());
}

void UInteractablePushableComponent::InteractPush()
{
	if (InteractableOwner->CurrentValue != Value)
		InteractableOwner->Interact(Value);
}
