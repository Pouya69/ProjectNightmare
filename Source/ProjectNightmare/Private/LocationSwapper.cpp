// Fill out your copyright notice in the Description page of Project Settings.


#include "LocationSwapper.h"

// Sets default values
ALocationSwapper::ALocationSwapper()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(FName("StaticMeshComp"));
	SetRootComponent(StaticMeshComp);
	StaticMeshComp->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Block);
	StaticMeshComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
	StaticMeshComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
}

// Called when the game starts or when spawned
void ALocationSwapper::BeginPlay()
{
	Super::BeginPlay();
	SetActorTickEnabled(HasPatrolPath());
	// SetInFocus(false);
}

// Called every frame
void ALocationSwapper::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!HasPatrolPath()) return;
	const FVector MyLocation = GetActorLocation();
	const FVector TargetLocation = PatrolPath[CurrentPatrolPathIndex]->GetActorLocation();
	AddActorWorldOffset((TargetLocation - MyLocation).GetUnsafeNormal() * PatrolSpeed);
	if (FVector::Dist(MyLocation, TargetLocation) <= 10.f) {
		CurrentPatrolPathIndex = (CurrentPatrolPathIndex + 1) > (PatrolPath.Num() - 1) ? 0 : CurrentPatrolPathIndex + 1;
	}

}

void ALocationSwapper::SwapLocation(AActor* OtherCharacter)
{
	FVector ActorLocation = OtherCharacter->GetActorLocation();
	FVector MyLocation = GetActorLocation();
	OtherCharacter->SetActorLocation(MyLocation);

	if (bDestroysOnUse)
		Destroy();
	else
		SetActorLocation(ActorLocation);
	
	

	// SetInFocus(false);

	// FRotator Rot = (ActorLocation - MyLocation).Rotation();

	// OtherCharacter->Controller->SetControlRotation(Rot);
	// SetActorRotation(Rot.GetInverse());
}

void ALocationSwapper::SetInFocus(bool bFocused)
{
	StaticMeshComp->SetOverlayMaterial(bFocused ? FocusedMaterial : nullptr);
}

bool ALocationSwapper::HasPatrolPath() const
{
	return !PatrolPath.IsEmpty();
}


