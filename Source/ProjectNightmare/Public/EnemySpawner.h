// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableObject.h"
#include "EnemySpawner.generated.h"

UENUM(Blueprintable)
enum class EEnemyType : uint8
{
	NORMAL,
	FAST,
	SPITTER,
	NONE,
};

UCLASS()
class PROJECTNIGHTMARE_API AEnemySpawner : public AInteractableObject
{
	GENERATED_BODY()
	


public:	
	// Sets default values for this actor's properties
	AEnemySpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UPROPERTY(EditAnywhere, Category = "Spawner")
		float SpawnEnemyEverySeconds = 15.f;
	FTimerHandle SpawnTimerHandle;
		
	UPROPERTY(EditAnywhere)
		USceneComponent* SpawnLocationComp;

	UPROPERTY(EditAnywhere, Category = "Spawner")
		float MaxHealth = 200.f;
	float Health;
	UPROPERTY(EditAnywhere, Category="Spawner")
		EEnemyType SpecificEnemyType = EEnemyType::NONE;
	UPROPERTY(EditAnywhere, Category = "Spawner")
		TSubclassOf<class AEnemyBaseCharacter> NormalEnemyClass;
	UPROPERTY(EditAnywhere, Category = "Spawner")
		TSubclassOf<class AEnemyBaseCharacter> FastEnemyClass;
	UPROPERTY(EditAnywhere, Category = "Spawner")
		TSubclassOf<class AEnemyBaseCharacter> SpitterEnemyClass;

	UFUNCTION(BlueprintCallable, Category = "Spawner")
		bool SpawnEnemy();
	UFUNCTION(BlueprintCallable, Category = "Spawner")
		void SpawnEnemyNOCHECK();
	UFUNCTION(BlueprintCallable, Category = "Spawner")
		bool SpawnEnemyWithType(EEnemyType EnemyType);

	// For when enemy is spawned and want to do something with animations on the spawned, effects and etc.
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Spawner")
		void EnemySpawnedSuccessfully();

	virtual void InteractionComplete() override;
	UPROPERTY(EditAnywhere, Category = "Spawner")
		class UNiagaraSystem* DestructionNiagara;
	UPROPERTY(EditAnywhere, Category = "Spawner")
		class UNiagaraSystem* HitNiagara;
	UFUNCTION()
		void SpawnerDestroyed(AActor* ActorDestroyed);
	UFUNCTION()
		void TakePointDamage(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType, AActor* DamageCauser);
	UFUNCTION()
		void TakeRadialDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, FVector Origin, const FHitResult& HitInfo, AController* InstigatedBy, AActor* DamageCauser);
	
	// For making visuals and etc.
	UFUNCTION(BlueprintCallable)
		void DestroySpawner();
};
