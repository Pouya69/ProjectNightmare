// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemySpawner.h"
#include "EnemyBaseCharacter.h"
#include "NiagaraFunctionLibrary.h"

// Sets default values
AEnemySpawner::AEnemySpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SpawnLocationComp = CreateDefaultSubobject<USceneComponent>(FName("Spawn Location Comp"));
	SpawnLocationComp->SetupAttachment(GetRootComponent());
	SpawnLocationComp->SetComponentTickEnabled(false);
	OnTakePointDamage.AddDynamic(this, &AEnemySpawner::TakePointDamage);
	OnTakeRadialDamage.AddDynamic(this, &AEnemySpawner::TakeRadialDamage);
}

// Called when the game starts or when spawned
void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();
	Health = MaxHealth;
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnEnemyNOCHECK, SpawnEnemyEverySeconds, true);
	OnDestroyed.AddDynamic(this, &AEnemySpawner::SpawnerDestroyed);
}

// Called every frame
void AEnemySpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

bool AEnemySpawner::SpawnEnemy()
{
	return SpawnEnemyWithType(SpecificEnemyType == EEnemyType::NONE ? StaticCast<EEnemyType>(FMath::RandRange(0, 2)) : SpecificEnemyType);
}

void AEnemySpawner::SpawnEnemyNOCHECK()
{
	SpawnEnemy();
}

bool AEnemySpawner::SpawnEnemyWithType(EEnemyType EnemyType)
{
	AEnemyBaseCharacter* SpawnedEnemy = nullptr;
	switch (EnemyType)
	{
		case EEnemyType::NONE:
			return false;	
			break;
		case EEnemyType::NORMAL:
			SpawnedEnemy = GetWorld()->SpawnActor<AEnemyBaseCharacter>(NormalEnemyClass, SpawnLocationComp->GetComponentTransform());
			break;
		case EEnemyType::FAST:
			SpawnedEnemy = GetWorld()->SpawnActor<AEnemyBaseCharacter>(FastEnemyClass, SpawnLocationComp->GetComponentTransform());
			break;
		case EEnemyType::SPITTER:
			SpawnedEnemy = GetWorld()->SpawnActor<AEnemyBaseCharacter>(SpitterEnemyClass, SpawnLocationComp->GetComponentTransform());
			break;
		default:
			break;
	}
	if (!SpawnedEnemy) return false;
	SpawnedEnemy->EnemySpawned();
	// SpawnedEnemy->FinishSpawning(SpawnLocationComp->GetComponentTransform());
	EnemySpawnedSuccessfully();
	return true;
}

void AEnemySpawner::InteractionComplete()
{
	DestroySpawner();
}

void AEnemySpawner::SpawnerDestroyed(AActor* ActorDestroyed)
{
	const FVector ExplosionLocation = ActorDestroyed->GetActorLocation();
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), DestructionNiagara, ExplosionLocation);
}

void AEnemySpawner::TakePointDamage(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType, AActor* DamageCauser)
{
	if (InstigatedBy && !InstigatedBy->IsA(APlayerController::StaticClass())) return;
	Health -= Damage;
	if (Health <= 0) {
		DestroySpawner();
		return;
	}
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitNiagara, GetActorLocation());
}

void AEnemySpawner::TakeRadialDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, FVector Origin, const FHitResult& HitInfo, AController* InstigatedBy, AActor* DamageCauser)
{
	if (InstigatedBy && !InstigatedBy->IsA(APlayerController::StaticClass())) return;
	Health -= Damage;
	if (Health <= 0) {
		DestroySpawner();
		return;
	}
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitNiagara, GetActorLocation());
}

void AEnemySpawner::DestroySpawner()
{
	// TODO: Goo and flesh effects etc.
	Destroy();
}
