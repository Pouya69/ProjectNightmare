// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBaseCharacter.h"
#include "EnemyBaseAIController.h"
#include "BehaviorTree/BlackboardComponent.h"

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
	if (AttackMontage)
		PlayAnimMontage(AttackMontage);
}

void AEnemyBaseCharacter::Die()
{
	// Controller->UnPossess();
	Super::Die();
	SetLifeSpan(DisappearAfterDeathInSeconds);
}

void AEnemyBaseCharacter::TakePointDamage(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType, AActor* DamageCauser)
{
	
	Super::TakePointDamage(DamagedActor, Damage, InstigatedBy, HitLocation, FHitComponent, BoneName, ShotFromDirection, DamageType, DamageCauser);
	ApplyDismembermentToLimb(BoneName, ShotFromDirection, HitLocation);
	
}

void AEnemyBaseCharacter::TakeRadialDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, FVector Origin, const FHitResult& HitInfo, AController* InstigatedBy, AActor* DamageCauser)
{
	Super::TakeRadialDamage(DamagedActor, Damage, DamageType, Origin, HitInfo, InstigatedBy, DamageCauser);
	ApplyDismembermentToLimb(HitInfo.BoneName, HitInfo.TraceEnd - HitInfo.TraceStart, HitInfo.ImpactPoint, false);

}

void AEnemyBaseCharacter::StopMyMovement()
{
	if (!EnemyAIController) return;
	EnemyAIController->StopMovement();
	EnemyAIController->MoveToLocation(GetActorLocation());
	EnemyAIController->GetBlackboardComponent()->SetValueAsBool(FName("ShouldRunAI"), false);
	//FTimerDelegate TimerDelegate;
	//TimerDelegate.BindLambda([&]() {
		//EnemyAIController->GetBlackboardComponent()->SetValueAsBool(FName("ShouldRunAI"), true);
	//});
	//GetWorldTimerManager().SetTimer(ShouldRunAITimer, TimerDelegate, 1.5f, false);
	
}

void AEnemyBaseCharacter::AllowAIMovement()
{
	EnemyAIController->GetBlackboardComponent()->SetValueAsBool(FName("ShouldRunAI"), true);
}
