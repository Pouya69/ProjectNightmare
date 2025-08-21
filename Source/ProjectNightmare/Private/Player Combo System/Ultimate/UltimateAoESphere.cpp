// Fill out your copyright notice in the Description page of Project Settings.


#include "Player Combo System/Ultimate/UltimateAoESphere.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AUltimateAoESphere::AUltimateAoESphere()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	SphereOverlapComp = CreateDefaultSubobject<USphereComponent>(FName("Sphere Overlap Comp"));
	SetRootComponent(SphereOverlapComp);
	//SphereMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(FName("Sphere Mesh Comp"));
	//SphereMeshComp->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AUltimateAoESphere::BeginPlay()
{
	Super::BeginPlay();
	TargetLocation = GetActorLocation();
	const float Ratio = SphereStartRadius / GetSphereRadius();
	SphereOverlapComp->SetSphereRadius(SphereStartRadius);
	//SphereMeshComp->SetRelativeScale3D(SphereMeshComp->GetRelativeScale3D() * Ratio);
	OnDestroyed.AddDynamic(this, &AUltimateAoESphere::Explode);
}

// Called every frame
void AUltimateAoESphere::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AUltimateAoESphere::Explode(AActor* DestroyedActor)
{
	if (bShouldDestroyWithoutExplosion) return;
	// Ultimate Damage
	const float Radius = GetSphereRadius();
	const float FinalUltimateDamage = 30.f + ((UltimateMaxDamage - 30.f) / (SphereMaxRadius - SphereStartRadius)) * (Radius - SphereStartRadius);
	TArray<AActor*> Ignores;
	Ignores.Add(this);
	// Ignores.Add(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	UGameplayStatics::ApplyRadialDamage(GetWorld(), FinalUltimateDamage, GetActorLocation() + FVector(0,0,2), Radius, UltimateDamageType, Ignores, nullptr, nullptr, true);
	/*
	TArray<AActor*> OverlapActors;
	SphereOverlapComp->GetOverlappingActors(OverlapActors);
	for (const AActor* OverlapActor : OverlapActors) {
		OverlapActor->
	}
	*/
}

float AUltimateAoESphere::GetSphereRadius() const
{
	return SphereOverlapComp->GetScaledSphereRadius();
}

void AUltimateAoESphere::UpdateSphere(float& DeltaTime)
{
	const float PreviousRadius = GetSphereRadius();
	SphereOverlapComp->SetSphereRadius(FMath::FInterpConstantTo(PreviousRadius, SphereMaxRadius, DeltaTime, SphereSizeIncreaseRate));
	//SphereMeshComp->SetRelativeScale3D(SphereMeshComp->GetRelativeScale3D() * (GetSphereRadius() / PreviousRadius));
}

