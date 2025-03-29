// Fill out your copyright notice in the Description page of Project Settings.


#include "GravityPulser.h"
#include "Components/BoxComponent.h"
#include "Grenade.h"
#include "CharacterBase.h"

// Sets default values
AGravityPulser::AGravityPulser()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootSceneComp = CreateDefaultSubobject<USceneComponent>(FName("Root Scene Comp"));
	SetRootComponent(RootSceneComp);
	OverlapAreaComp = CreateDefaultSubobject<UBoxComponent>(FName("Overlap Comp"));
	OverlapAreaComp->SetupAttachment(RootSceneComp);
	OverlapAreaComp->SetGenerateOverlapEvents(true);
}

// Called when the game starts or when spawned
void AGravityPulser::BeginPlay()
{
	Super::BeginPlay();
	if (PulseTimerTimeInSeconds > 0)
		GetWorldTimerManager().SetTimer(PulseTimer, this, &AGravityPulser::GravityChange, PulseTimerTimeInSeconds, true);
	if (ChangeDirectionTimeInSeconds > 0)
		GetWorldTimerManager().SetTimer(DirectionChangeTimer, this, &AGravityPulser::ChangeDirectionRandom, ChangeDirectionTimeInSeconds, true);
}

// Called every frame
void AGravityPulser::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (PulseTimerTimeInSeconds > 0) return;
	GravityChange();
}

void AGravityPulser::GravityChange()
{
	TArray<UPrimitiveComponent*> OverlapComps;
	OverlapAreaComp->GetOverlappingComponents(OverlapComps);
	if (OverlapComps.IsEmpty()) return;
	for (UPrimitiveComponent* OverlapComp : OverlapComps) {
		AActor* OverlapOwner = OverlapComp->GetOwner();
		if (ACharacterBase* OverlapCharacter = Cast<ACharacterBase>(OverlapOwner)) {
			if (OverlapCharacter->IsAlive()) {
				if (OverlapCharacter->bShouldRagdollInGravityAreas) {
					// UE_LOG(LogTemp, Warning, TEXT("TRYINGAAAAA %s"), *OverlapCharacter->GetName());
					OverlapCharacter->StartRagdolling();
					OverlapCharacter->AddImpulseToCharacter(GravityDirection * GravityForce);
					// OverlapCharacter->GetMesh()->AddImpulse(OverlapCharacter->GetMesh()->GetMass() * GravityDirection * GravityForce);
					continue;
				}
				OverlapCharacter->LaunchCharacter(GravityDirection * GravityForce, true, true);
			}
			else
				OverlapComp->AddImpulse(OverlapComp->GetMass() * GravityDirection * GravityForce);
			continue;
		}
		else if (AGrenade* GrenadeOverlap = Cast<AGrenade>(OverlapOwner)) {
			GrenadeOverlap->AddImpulseToGrenade(GravityDirection * GravityForce);
			continue;
		}
		else if (!OverlapComp->IsSimulatingPhysics()) continue;
		OverlapComp->AddImpulse(OverlapComp->GetMass() * GravityDirection * GravityForce);
	}
}

void AGravityPulser::ChangeDirectionRandom()
{
	if (bIsDiagonalDirectionAllowed) {
		GravityDirection = FVector(FMath::RandRange(0, 1), FMath::RandRange(0, 1), FMath::RandRange(0, 1));
		return;
	}
	const int WhichDirection = FMath::RandRange(1, 3);
	const bool bOpposite = FMath::RandBool();
	switch (WhichDirection)
	{
		case 1:
			GravityDirection = FVector(bOpposite ? -1 : 1, 0, 0);
			break;
		case 2:
			GravityDirection = FVector(0, bOpposite ? -1 : 1, 0);
			break;
		case 3:
			GravityDirection = FVector(0, 0, bOpposite ? -1 : 1);
			break;
		default:
			break;
	}
}
