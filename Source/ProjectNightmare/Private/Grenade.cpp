// Fill out your copyright notice in the Description page of Project Settings.


#include "Grenade.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraFunctionLibrary.h"

// Sets default values
AGrenade::AGrenade()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	if (!RootComponent) {
		RootComp = CreateDefaultSubobject<USceneComponent>(FName("Root Comp"));
		SetRootComponent(RootComp);
	}
	SphereComp = CreateDefaultSubobject<USphereComponent>(FName("Sphere Collision Comp"));
	SphereComp->SetupAttachment(GetRootComponent());
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(FName("Projectile Movement Comp"));
	ProjectileMovementComponent->InitialSpeed = 0;
	ProjectileMovementComponent->MaxSpeed = 0;
	ProjectileMovementComponent->ProjectileGravityScale = 0;
	ProjectileMovementComponent->bShouldBounce = true;
	ProjectileMovementComponent->SetUpdatedComponent(SphereComp);
}

// Called when the game starts or when spawned
void AGrenade::BeginPlay()
{
	Super::BeginPlay();
	OnDestroyed.AddDynamic(this, &AGrenade::Explode);
}

// Called every frame
void AGrenade::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGrenade::SetSpeed(float Speed)
{
	ProjectileMovementComponent->InitialSpeed = Speed;
	ProjectileMovementComponent->MaxSpeed = Speed;
	ProjectileMovementComponent->ProjectileGravityScale = 1;
}

void AGrenade::Explode(AActor* DestroyedActor)
{
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ExplosionNiagara, SphereComp->GetComponentLocation());
	// TODO: Explosion logic
}
