// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableObject.generated.h"

UCLASS()
class PROJECTNIGHTMARE_API AInteractableObject : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInteractableObject();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void SetFocusMaterial(bool bIsFocused);

public:
	UPROPERTY(EditAnywhere)
		int CurrentValue = 0;

	void Interact(int Value);
	void HoldInteract(class AThirdPersonPlayerCharacter* PlayerCharacterRef, const float DeltaTime);

	UPROPERTY(EditAnywhere)
		class UWidgetComponent* InteractUserWidgetComp;
	class UInteractionWidget* InteractionWidget;
	UPROPERTY(EditAnywhere)
		class USceneComponent* RootSceneComp;
	UStaticMeshComponent* StaticMeshComp;

public:
	UPROPERTY(EditAnywhere)
		class UMaterialInstance* FocusedMaterial;
	bool bIsInteractable = true;
	void SetInRange(bool bInRange);
	void SetIsFocused(bool bIsFocused);
	void StartInteraction();
	void StopInteraction();
	UFUNCTION(BlueprintCallable)
		virtual void InteractionComplete(class AThirdPersonPlayerCharacter* PlayerCharacterRef);
	virtual void InteractionComplete();
	UFUNCTION(BlueprintNativeEvent)
		void InteractionComplete_Finished_Event(class AThirdPersonPlayerCharacter* PlayerCharacterRef);
	void InteractionComplete_Finished_Event_Implementation(class AThirdPersonPlayerCharacter* PlayerCharacterRef);
	bool bIsInteracting;
	UPROPERTY(EditAnywhere)
		float InteractionRate = 50.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		float InteractionPercent;
};
