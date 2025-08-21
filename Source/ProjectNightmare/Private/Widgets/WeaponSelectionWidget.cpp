// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/WeaponSelectionWidget.h"
#include "Weapon.h"
#include "ThirdPersonPlayerCharacter.h"

void UWeaponSelectionWidget::StartMakeWidget_Implementation(AThirdPersonPlayerCharacter* InPlayerCharacter)
{
	PlayerCharacterRef = InPlayerCharacter;
	NextItem_Implementation();
	NextItem();
}

void UWeaponSelectionWidget::NextItem_Implementation(bool bNext)
{
	
}

void UWeaponSelectionWidget::SelectWeapon(AWeapon* WeaponToEquip)
{
	if (!PlayerCharacterRef || !WeaponToEquip || WeaponToEquip == PlayerCharacterRef->CurrentWeapon) return;
	UE_LOG(LogTemp, Warning, TEXT("Changing to %s"), *WeaponToEquip->GetName());
	PlayerCharacterRef->UnequipWeapon();
	PlayerCharacterRef->CurrentWeapon = WeaponToEquip;
	PlayerCharacterRef->EquipWeapon();
}
