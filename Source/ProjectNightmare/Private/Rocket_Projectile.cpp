// Fill out your copyright notice in the Description page of Project Settings.


#include "Rocket_Projectile.h"
#include "Kismet/KismetMathLibrary.h"
#include "NiagaraSystem.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SplineComponent.h"

// Sets default values
ARocket_Projectile::ARocket_Projectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PathSpline = CreateDefaultSubobject<USplineComponent>(FName("PathSpline"));
	PathSpline->SetupAttachment(RootComponent);
}

void ARocket_Projectile::InitializeRocket(const FVector InRocketDirection, const FRotator InInitialRotation)
{
	InitialLocation = GetActorLocation();
	ShotDirection = InRocketDirection;
	FinalRotation = UKismetMathLibrary::MakeRotFromX(ShotDirection);
	InitialRotation = InInitialRotation;
	SetActorRotation(InitialRotation);
	GetWorldTimerManager().SetTimer(DestroyTimer, this, &ARocket_Projectile::Explode, TimeBeforeDestroy, false);
}

void ARocket_Projectile::Exploded(AActor* DestroyedActor)
{
	// UGameplayStatics::SpawnSoundAtLocation()
	TArray<AActor*> Ignores;
	Ignores.Add(this);
	UGameplayStatics::ApplyRadialDamage(GetWorld(),
		ExplosionDanage,
		ExplosionLocation,
		ExplosionRadius,
		RocketExplosionDamageType,
		Ignores,
		nullptr, nullptr, false, ECC_Visibility);
	if (bWasHit)
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), Explosion_NS, ExplosionLocation, ExplosionNormal.Rotation());
	else
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), Explosion_NS, GetActorLocation());
}

void ARocket_Projectile::RocketHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
{
	bWasHit = true;
	ExplosionLocation = Hit.ImpactPoint;
	Explode();
}

void ARocket_Projectile::Explode()
{
	Destroy();
}

// Called when the game starts or when spawned
void ARocket_Projectile::BeginPlay()
{
	Super::BeginPlay();
	OnDestroyed.AddDynamic(this, &ARocket_Projectile::Exploded);
	OnActorHit.AddDynamic(this, &ARocket_Projectile::RocketHit);
}

// Called every frame
void ARocket_Projectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
