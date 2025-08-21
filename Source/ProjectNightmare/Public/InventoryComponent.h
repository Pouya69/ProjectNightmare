// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTNIGHTMARE_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintReadOnly)
		TArray<class AInteractableObject*> InventoryItems;
	UPROPERTY(BlueprintReadOnly)
		TArray<class AWeapon*> InventoryWeapons;

	void AddItemToInventory(class AWeapon* WeaponToAdd);
	void AddItemToInventory(class AInteractableObject* ItemToAdd);
	void DropItem(int index);
	class AInteractableObject* DropItem(class AInteractableObject* ItemToDrop);
	class AWeapon* DropItem(class AWeapon* WeaponToDrop);

	UFUNCTION(BlueprintCallable)
		class AWeapon* GetWeaponByID(int WeaponID) const;
	UFUNCTION(BlueprintCallable)
		int GetWeaponIndex(const class AWeapon* WeaponRef) const;

	class ACharacterBase* OwnerCharacter;

};
