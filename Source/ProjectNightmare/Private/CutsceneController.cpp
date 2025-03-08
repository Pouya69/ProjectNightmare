// Fill out your copyright notice in the Description page of Project Settings.


#include "CutsceneController.h"
#include "Runtime/LevelSequence/Public/LevelSequenceActor.h"
#include "Runtime/LevelSequence/Public/LevelSequencePlayer.h"
#include "Runtime/LevelSequence/Public/LevelSequence.h"
#include "ThirdPersonPlayerCharacter.h"

// Sets default values
ACutsceneController::ACutsceneController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACutsceneController::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACutsceneController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

UInputAction* ACutsceneController::GetRequiredAction() const
{
	if (RequiredActions.IsEmpty()) return nullptr;
	return RequiredActions[0];
}

void ACutsceneController::SectionDone()
{
	if (RequiredActions.IsEmpty()) return;
	RequiredActions.RemoveAt(0);
	// UE_LOG(LogTemp, Warning, TEXT("RequiredActions Left: %d"), RequiredActions.Num());
}

UUserWidget* ACutsceneController::GetCurrentWidget() const
{
	return PlayerCharacterRef->GetCurrentWidgetOnScreen();
}

float ACutsceneController::GetPlaybackRateFromFrameSpeedInAction(int Current, int Target) const
{
	return Current < Target ? 1.f : (Current > Target ? -1.f : 0);
}

int ACutsceneController::GetTargetFrameProportional(float Value, int ActionStart, int ActionEnd) const
{
	return FMath::RoundToInt(MapRangeClamped(Value, 0.f, 100.f, ActionStart, ActionEnd));
}

bool ACutsceneController::HasCutsceneStarted() const
{
	return LevelSequenceActorRef != nullptr;
}

void ACutsceneController::SetPlayerTransform(FTransform NewTransform)
{
	PlayerCharacterRef->SetActorTransform(NewTransform);
}

void ACutsceneController::CreateWidgetAndShow(TSubclassOf<class UUserWidget> WidgetClass)
{
	PlayerCharacterRef->CreateWidgetAndShow(WidgetClass);
}

void ACutsceneController::ClearWidget()
{
	PlayerCharacterRef->ClearWidget();
}

bool ACutsceneController::IsCutscenePlaying() const
{
	return LevelSequenceActorRef->GetSequencePlayer()->IsPlaying();
}

void ACutsceneController::FinishedCutscene(FTransform NewPlayerTransform, bool bTeleport, bool bFailedCutscene)
{
	ClearWidget();
	LevelSequenceActorRef->Destroy();
	PlayerCharacterRef->FinishedCutscene(NewPlayerTransform, bTeleport, bFailedCutscene);
	PlayerCharacterRef = nullptr;

	if (bShouldDestroyAtEnd) {
		OtherCharacterInCutscene->Destroy();
		Destroy();
	}
	else {
		SetActorTickEnabled(false);
		OnCutsceneFinished.Broadcast(bTeleport, bFailedCutscene);
	}
		
}

void ACutsceneController::StartedCutscene()
{
	OnCutsceneStarted.Broadcast();
}

void ACutsceneController::PauseCutscene()
{
	if (LevelSequenceActorRef == nullptr) return;
	LevelSequenceActorRef->GetSequencePlayer()->Pause();
	OnCutscenePaused.Broadcast();
}

void ACutsceneController::ResumeCutscene(bool bShouldClearWidget)
{
	if (LevelSequenceActorRef == nullptr) return;
	if (bShouldClearWidget)
		ClearWidget();
	// UE_LOG(LogTemp, Warning, TEXT("Level Sequence Asset name from CutsceneController: %s"), *LevelSequenceActorRef->LevelSequenceAsset->GetFullName());
	LevelSequenceActorRef->GetSequencePlayer()->Play();
	OnCutsceneResumed.Broadcast(bShouldClearWidget);
}

void ACutsceneController::ChangePlayRate(float PlayRate)
{
	if (LevelSequenceActorRef == nullptr) return;
	LevelSequenceActorRef->GetSequencePlayer()->SetPlayRate(PlayRate);
}

float ACutsceneController::MapRangeClamped(float Value, float InRangeA, float InRangeB, float OutRangeA, float OutRangeB) const
{
    return FMath::Lerp(OutRangeA, OutRangeB, (FMath::Clamp(Value, InRangeA, InRangeB) - InRangeA) / (InRangeB - InRangeA));
}