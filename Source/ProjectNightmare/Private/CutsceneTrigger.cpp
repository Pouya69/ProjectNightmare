// Fill out your copyright notice in the Description page of Project Settings.


#include "CutsceneTrigger.h"
#include "Components/BoxComponent.h"
#include "ThirdPersonPlayerCharacter.h"
#include "CutsceneController.h"
#include "Runtime/LevelSequence/Public/LevelSequenceActor.h"
#include "Runtime/LevelSequence/Public/LevelSequencePlayer.h"
#include "Runtime/LevelSequence/Public/LevelSequence.h"

// Sets default values
ACutsceneTrigger::ACutsceneTrigger()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	BoxComp = CreateDefaultSubobject<UBoxComponent>(FName("Root Box Comp"));
	BoxComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BoxComp->SetGenerateOverlapEvents(true);
	SetRootComponent(BoxComp);

}

// Called when the game starts or when spawned
void ACutsceneTrigger::BeginPlay()
{
	Super::BeginPlay();
	BoxComp->OnComponentBeginOverlap.AddDynamic(this, &ACutsceneTrigger::OnTriggerBoxOverlap);
	if (!ShouldStartCutsceneOnOverlap())
		BoxComp->OnComponentEndOverlap.AddDynamic(this, &ACutsceneTrigger::OnTriggerBoxEndOverlap);
}

// Called every frame
void ACutsceneTrigger::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACutsceneTrigger::StartCutscene(AThirdPersonPlayerCharacter* PlayerCharacterRef, bool bShouldDestroy)
{

	PlayerCharacterRef->MyPlayerController->SetControlRotation(CutsceneController->OtherCharacterInCutscene->GetActorRotation());
	PlayerCharacterRef->SetActorLocation(CutsceneController->OtherCharacterInCutscene->GetActorLocation());
	PlayerCharacterRef->ChangePlayerCollision(false);
	PlayerCharacterRef->ChangePlayerMovement(EMovementMode::MOVE_None);
	PlayerCharacterRef->SetCutsceneController(CutsceneController);
	ALevelSequenceActor* SequenceActor;
	// if (!SequenceActor) return;
	FMovieSceneSequencePlaybackSettings MovieSettings = FMovieSceneSequencePlaybackSettings();
	ULevelSequencePlayer* SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), LevelSequenceNeeded, MovieSettings, SequenceActor);
	if (!SequencePlayer || !SequenceActor) return;
	CutsceneController->LevelSequenceActorRef = SequenceActor;
	CutsceneController->PlayerCharacterRef = PlayerCharacterRef;
	CutsceneController->bShouldDestroyAtEnd = bShouldDestroy;
	CutsceneController->SetActorTickEnabled(true);
	FMovieSceneObjectBindingID Binding = FMovieSceneObjectBindingID();

	for (const FMovieSceneBinding& BindingA : SequenceActor->LevelSequenceAsset->GetMovieScene()->GetBindings())
	{
		// UE_LOG(LogTemp, Warning, TEXT("%s"), *BindingA.GetName());
		if (BindingA.GetName() == PlayerCharacterName)
		{
			Binding = BindingA.GetObjectGuid();
			break;
		}
	}
	if (!Binding.IsValid()) return;
	SequenceActor->AddBinding(Binding, PlayerCharacterRef, false);
	CutsceneController->ResumeCutscene(true);
	if (bShouldDestroy)
		Destroy();

} 

void ACutsceneTrigger::OnTriggerBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AThirdPersonPlayerCharacter* PlayerCharacter = Cast<AThirdPersonPlayerCharacter>(OtherActor)) {
		if (!PlayerCharacter->IsPlayerInputEnabled()) return;
		if (ShouldStartCutsceneOnOverlap())
			StartCutscene(PlayerCharacter);
		else {
			PlayerCharacter->SetInCutsceneTrigger(this, StartCutsceneWidget);
		}
	}
}

void ACutsceneTrigger::OnTriggerBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AThirdPersonPlayerCharacter* PlayerCharacter = Cast<AThirdPersonPlayerCharacter>(OtherActor)) {
		if (!PlayerCharacter->IsPlayerInputEnabled()) return;
		PlayerCharacter->SetInCutsceneTrigger(nullptr);
	}
}
