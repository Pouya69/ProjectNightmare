// Fill out your copyright notice in the Description page of Project Settings.


#include "Grenade.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
// #include "GameFramework/DamageType.h"
#include "Engine/DamageEvents.h"
#include "CharacterBase.h"
#include "Kismet/GameplayStatics.h"

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
	ExplosionSphereComp = CreateDefaultSubobject<USphereComponent>(FName("Explosion Sphere Collision Comp"));
	ExplosionSphereComp->SetupAttachment(SphereComp);
	ExplosionSphereComp->SetGenerateOverlapEvents(true);
	ExplosionSphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	ExplosionSphereComp->SetCollisionResponseToChannel(ECC_Destructible, ECR_Overlap);
	ExplosionSphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	ExplosionSphereComp->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
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
	const FVector ExplosionLocation = SphereComp->GetComponentLocation();
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ExplosionNiagara, ExplosionLocation);
	TArray<AActor*> Overlaps;
	
	ExplosionSphereComp->GetOverlappingActors(Overlaps);
	if (Overlaps.IsEmpty()) return;
	FHitResult UnderHitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	const bool bHitUnder = GetWorld()->LineTraceSingleByChannel(UnderHitResult, ExplosionLocation, ExplosionLocation + (FVector::DownVector * 40), ECC_WorldStatic, QueryParams);
	TArray<AActor*> Ignores;
	if (bHitUnder)
		Ignores.Add(UnderHitResult.GetActor());
	Ignores.Add(this);
	FDamageEvent ExplosionDamageEvent = FDamageEvent(GrenadeExplosionDamageType);
	bool bDamaged = UGameplayStatics::ApplyRadialDamage(GetWorld(), GrenadeDamage, ExplosionLocation, GrenadeExplosionRadius, GrenadeExplosionDamageType, Ignores, nullptr, nullptr, false, ECC_Visibility);
	// UE_LOG(LogTemp, Warning, TEXT("Damaged : %d. Instigator: %s, Causer: %s"), bDamaged ? 1 : 0, *GetInstigatorController()->GetName(), *GetOwner()->GetName());
	for (AActor* Actor : Overlaps) {
		
		//FHitResult HitResult;
		//const bool bObjectInFront = GetWorld()->SweepSingleByChannel(HitResult, ExplosionLocation, Actor->GetActorLocation(), FQuat::Identity, ECC_WorldStatic, FCollisionShape::MakeSphere(5.f), QueryParams);
		//if (bObjectInFront) {
		//	UE_LOG(LogTemp, Warning, TEXT("%s Blocked By: %s"), *Actor->GetName(), *HitResult.GetActor()->GetName());
		//	continue;
		//}
		
		
		// Actor->TakeDamage(GrenadeDamage, ExplosionDamageEvent, GetInstigatorController(), GetOwner());
		if (ACharacterBase* CharacterDamaged = Cast<ACharacterBase>(Actor)) {
			if (!CharacterDamaged->IsAlive()) {
				// CharacterDamaged->GetMesh()->AddRadialImpulse(ExplosionLocation, GrenadeExplosionRadius, GrenadeExplosionStrength, ERadialImpulseFalloff::RIF_Linear);
			}
		}
		else if (UPrimitiveComponent* PrimitiveComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent())) {
			// PrimitiveComp->AddRadialImpulse(ExplosionLocation, GrenadeExplosionRadius, GrenadeExplosionStrength, ERadialImpulseFalloff::RIF_Linear);
		}
	}
	// TODO: Explosion logic
}
