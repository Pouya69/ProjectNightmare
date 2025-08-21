// Fill out your copyright notice in the Description page of Project Settings.


#include "NightmareGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "ThirdPersonPlayerCharacter.h"
#include "FourthDimension_Portal.h"

ANightmareGameMode::ANightmareGameMode()
{
}

void ANightmareGameMode::BeginPlay()
{
	Super::BeginPlay();
	PlayerCharacter = Cast<AThirdPersonPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	PortalToOverworld = Cast<AFourthDimension_Portal>(UGameplayStatics::GetActorOfClass(GetWorld(), PortalToOverworldClass));
}

void ANightmareGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
