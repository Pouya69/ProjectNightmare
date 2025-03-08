// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CutsceneController.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCutsceneStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCutscenePaused);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCutsceneResumed, bool, bShouldClearWidget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCutsceneFinished, bool, bTeleport, bool, bFailedCutscene);

UCLASS()
class PROJECTNIGHTMARE_API ACutsceneController : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACutsceneController();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Reliants
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		class AThirdPersonPlayerCharacter* OtherCharacterInCutscene;  // This is the animated character

public:
	// Variables
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class AThirdPersonPlayerCharacter* PlayerCharacterRef;
	UPROPERTY(BlueprintReadOnly)
		class ALevelSequenceActor* LevelSequenceActorRef;
	bool bShouldDestroyAtEnd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TArray<UInputAction*> RequiredActions;
		

public:
	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Events")
		FOnCutsceneStarted OnCutsceneStarted;
	UPROPERTY(BlueprintAssignable, Category = "Events")
		FOnCutscenePaused OnCutscenePaused;
	UPROPERTY(BlueprintAssignable, Category = "Events")
		FOnCutsceneResumed OnCutsceneResumed;
	UPROPERTY(BlueprintAssignable, Category = "Events")
		FOnCutsceneFinished OnCutsceneFinished;

public:
	// Helpers
	UFUNCTION(BlueprintCallable)
		UInputAction* GetRequiredAction() const;
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
		void PlayerAction(UInputAction* Action);  // Should be defined in BP
	UFUNCTION(BlueprintCallable)
		void SectionDone();
	UFUNCTION(BlueprintCallable)
		class UUserWidget* GetCurrentWidget() const;
	UFUNCTION(BlueprintCallable)
		float GetPlaybackRateFromFrameSpeedInAction(int Current, int Target) const;
	UFUNCTION(BlueprintCallable)
		int GetTargetFrameProportional(float Value, int ActionStart, int ActionEnd) const;
	UFUNCTION(BlueprintCallable)
		bool HasCutsceneStarted() const;
	UFUNCTION(BlueprintCallable)
		void SetPlayerTransform(FTransform NewTransform);
	UFUNCTION(BlueprintCallable)
		void CreateWidgetAndShow(TSubclassOf<class UUserWidget> WidgetClass);
	UFUNCTION(BlueprintCallable)
		void ClearWidget();
	UFUNCTION(BlueprintCallable)
		bool IsCutscenePlaying() const;
	UFUNCTION(BlueprintCallable)
		void FinishedCutscene(FTransform NewPlayerTransform, bool bTeleport, bool bFailedCutscene);
	UFUNCTION(BlueprintCallable)
		void StartedCutscene();
	UFUNCTION(BlueprintCallable)
		void PauseCutscene();
	UFUNCTION(BlueprintCallable)
		void ResumeCutscene(bool bShouldClearWidget = false);
	UFUNCTION(BlueprintCallable)
		void ChangePlayRate(float PlayRate);


	float MapRangeClamped(float Value, float InRangeA, float InRangeB, float OutRangeA, float OutRangeB) const;
};
