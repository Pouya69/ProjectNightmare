// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractionWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTNIGHTMARE_API UInteractionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	float* Percent;
	UFUNCTION(BlueprintCallable)
		float GetPercent() const { return *Percent; }
	UFUNCTION(BlueprintImplementableEvent)
		void StartFocus();
	UFUNCTION(BlueprintImplementableEvent)
		void SetInRange(bool bInRange);

	virtual void NativePreConstruct() override;
};
