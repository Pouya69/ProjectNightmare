// Fill out your copyright notice in the Description page of Project Settings.


#include "NightmareGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "ThirdPersonPlayerCharacter.h"

ANightmareGameMode::ANightmareGameMode()
{
}

void ANightmareGameMode::BeginPlay()
{
	Super::BeginPlay();
	PlayerCharacter = Cast<AThirdPersonPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
}

void ANightmareGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
