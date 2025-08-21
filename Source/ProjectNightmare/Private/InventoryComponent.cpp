// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"
#include "Weapon.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UInventoryComponent::AddItemToInventory(AWeapon* WeaponToAdd)
{
	if (!InventoryWeapons.IsEmpty()) {
		if (InventoryWeapons.Contains(WeaponToAdd)) {
			// TODO: Ammo etc.
			return;
		}
	}
	InventoryWeapons.Add(WeaponToAdd);
}

void UInventoryComponent::AddItemToInventory(AInteractableObject* ItemToAdd)
{
	if (!InventoryItems.IsEmpty()) {
		if (InventoryItems.Contains(ItemToAdd)) {
			// TODO: Item existing
			return;
		}
	}
	InventoryItems.Add(ItemToAdd);
}

void UInventoryComponent::DropItem(int index)
{
}

AInteractableObject* UInventoryComponent::DropItem(AInteractableObject* ItemToDrop)
{
	return nullptr;
}

AWeapon* UInventoryComponent::DropItem(AWeapon* WeaponToDrop)
{
	return nullptr;
}

AWeapon* UInventoryComponent::GetWeaponByID(int WeaponID) const
{
	if (InventoryWeapons.IsEmpty()) return nullptr;
	for (AWeapon* WeaponItem : InventoryWeapons)
		if (WeaponItem->WeaponID == WeaponID) return WeaponItem;
	return nullptr;
}

int UInventoryComponent::GetWeaponIndex(const AWeapon* WeaponRef) const
{
	if (InventoryWeapons.IsEmpty()) return -1;
	for (int i = 0; i < InventoryWeapons.Num(); i++)
		if (InventoryWeapons[i] == WeaponRef) return i;
	return -1;
}
