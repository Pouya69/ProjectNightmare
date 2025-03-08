// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBaseCharacter.h"
#include "EnemyBaseAIController.h"

AEnemyBaseCharacter::AEnemyBaseCharacter()
{
	
}

void AEnemyBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	EnemyAIController = Cast<AEnemyBaseAIController>(GetController());
	
}

void AEnemyBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AEnemyBaseCharacter::AttackPlayer()
{

}
