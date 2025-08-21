// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WeaponSelectionWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTNIGHTMARE_API UWeaponSelectionWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly)
		class AThirdPersonPlayerCharacter* PlayerCharacterRef;
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void StartMakeWidget(class AThirdPersonPlayerCharacter* InPlayerCharacter);
	void StartMakeWidget_Implementation(class AThirdPersonPlayerCharacter* InPlayerCharacter);
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void NextItem(bool bNext = true);
	void NextItem_Implementation(bool bNext = true);

	UFUNCTION(BlueprintCallable)
		void SelectWeapon(class AWeapon* WeaponToEquip);
	UPROPERTY(BlueprintReadOnly)
		UDataTable* Weapons_DT;
};
