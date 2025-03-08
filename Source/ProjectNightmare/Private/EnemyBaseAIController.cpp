// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBaseAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "ThirdPersonPlayerCharacter.h"

AEnemyBaseAIController::AEnemyBaseAIController()
{
}

void AEnemyBaseAIController::BeginPlay()
{
	Super::BeginPlay();
	if (EnemyBehaviour) {
		RunBehaviorTree(EnemyBehaviour);
	}
	AIPerceptionComp = FindComponentByClass<UAIPerceptionComponent>();
	if (AIPerceptionComp) {
		AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyBaseAIController::OnPerceptionInfoUpdated);
		AIPerceptionComp->OnTargetPerceptionForgotten.AddDynamic(this, &AEnemyBaseAIController::OnPerceptionInfoForgotten);
	}

}

void AEnemyBaseAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEnemyBaseAIController::OnPerceptionInfoUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	// UE_LOG(LogTemp, Warning, TEXT("%s"), *Stimulus.Type.Name.ToString());
	if (Stimulus.Type.Name.IsEqual(FName("Default__AISense_Sight"))) {
		HandleSense_Sight(Actor, Stimulus);
	}
}

void AEnemyBaseAIController::OnPerceptionInfoForgotten(AActor* Actor)
{
	UE_LOG(LogTemp, Warning, TEXT("FORGOTTEN PLAYERREF"));
	UObject* PlayerObject = Blackboard->GetValueAsObject(PlayerCharacterRefNameKEY);
	if (PlayerObject == nullptr || Actor != PlayerObject) return;
	Blackboard->ClearValue(PlayerCharacterRefNameKEY);
	Blackboard->SetValueAsInt(TimeSinceSeenPlayerKEY, 0);
}

void AEnemyBaseAIController::HandleSense_Sight(AActor* Actor, FAIStimulus& Stimulus)
{
	if (AThirdPersonPlayerCharacter* PlayerCharacter = Cast<AThirdPersonPlayerCharacter>(Actor)) {
		Blackboard->SetValueAsBool(FName("HasSeenPlayer"), true);
		if (Stimulus.WasSuccessfullySensed()) {
			Blackboard->SetValueAsObject(PlayerCharacterRefNameKEY, PlayerCharacter);
			
		}
		else {
			
			Blackboard->ClearValue(PlayerCharacterRefNameKEY);
		}
		Blackboard->SetValueAsVector(LastKnownPlayerLocationKEY, Stimulus.StimulusLocation);
		Stimulus.SetStimulusAge(1);
		Blackboard->SetValueAsInt(TimeSinceSeenPlayerKEY, (int) Stimulus.GetAge());
		UE_LOG(LogTemp, Warning, TEXT("AGE: %f"), Stimulus.GetAge());
	}
}
