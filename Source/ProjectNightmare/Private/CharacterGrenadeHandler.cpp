// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterGrenadeHandler.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UCharacterGrenadeHandler::UCharacterGrenadeHandler()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UCharacterGrenadeHandler::BeginPlay()
{
	Super::BeginPlay();
	SplineComp = Cast<USplineComponent>(GetChildComponent(0));
	// ...
	
}


// Called every frame
void UCharacterGrenadeHandler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UCharacterGrenadeHandler::ThrowGrenade(bool bIsAiming)
{
	FActorSpawnParameters params;
	params.Owner = GetOwner();
	params.Instigator = Cast<APawn>(GetOwner());
	AGrenade* SpawnedGrenade;
	const FTransform SpawnTransform = FTransform(GetComponentRotation(), GetComponentLocation(), FVector::OneVector);
	if (bIsAiming) {
		SpawnedGrenade = GetWorld()->SpawnActorDeferred<AGrenade>(GrenadeClass, SpawnTransform, params.Owner, params.Instigator);
		if (SpawnedGrenade) {
			SpawnedGrenade->SetSpeed(GrenadeLaunchVelocity * (bIsAiming ? GrenadeLaunchVelocityAimMultiplier : 1));
			UGameplayStatics::FinishSpawningActor(SpawnedGrenade, SpawnTransform);
			// SpawnedGrenade->FinishSpawning(SpawnTransform);
		}
	}
	else
		SpawnedGrenade = GetWorld()->SpawnActor<AGrenade>(GrenadeClass, GetComponentLocation(), GetComponentRotation(), params);
	SplineComp->ClearSplinePoints();
	if (!CreatedSplines.IsEmpty()) {
		for (USceneComponent* Comp : CreatedSplines)
			Comp->DestroyComponent();
		CreatedSplines.Empty();
	}
}

void UCharacterGrenadeHandler::OngoingGrenadeAim(bool bIsAiming)
{
	// UE_LOG(LogTemp, Warning, TEXT("ONGOING"));
	SplineComp->ClearSplinePoints();
	if (!CreatedSplines.IsEmpty()) {
		for (USceneComponent* Comp : CreatedSplines)
			Comp->DestroyComponent();
		CreatedSplines.Empty();
	}
	FPredictProjectilePathParams params(GrenadeRadius,
		GetComponentLocation(),
		GetForwardVector() * GrenadeLaunchVelocity * (bIsAiming ? GrenadeLaunchVelocityAimMultiplier : 1),
		GrenadeSimTime,
		ECC_WorldStatic,
		GetOwner());
	params.SimFrequency = GrenadeSimFrequency;
	// params.DrawDebugType = EDrawDebugTrace::ForOneFrame;
	FPredictProjectilePathResult result;
	const bool bWillHit = UGameplayStatics::PredictProjectilePath(GetWorld(), params, result);
	const int ResultNum = result.PathData.Num();
	for (int i = 0; i < ResultNum; i++)
	{
		SplineComp->AddSplinePoint(result.PathData[i].Location, ESplineCoordinateSpace::World);
	}
	
	for (int i = 0; i < ResultNum-1; i++)
	{
		USplineMeshComponent* CreatedSplineMeshComp = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass());
		if (!CreatedSplineMeshComp) return;
		CreatedSplineMeshComp->RegisterComponent();
		// CreatedSplineMeshComp->CreationMethod = EComponentCreationMethod::UserConstructionScript;
		CreatedSplineMeshComp->SetMobility(EComponentMobility::Movable);
		CreatedSplineMeshComp->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetIncludingScale);
		GetOwner()->AddInstanceComponent(CreatedSplineMeshComp);
		CreatedSplineMeshComp->SetStaticMesh(SplineMesh);
		CreatedSplineMeshComp->SetForwardAxis(ESplineMeshAxis::X);
		CreatedSplineMeshComp->SetStartAndEnd(SplineComp->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Local), SplineComp->GetTangentAtSplinePoint(i, ESplineCoordinateSpace::Local), SplineComp->GetLocationAtSplinePoint(i + 1, ESplineCoordinateSpace::Local), SplineComp->GetTangentAtSplinePoint(i + 1, ESplineCoordinateSpace::Local));
		// CreatedSplineMeshComp->SetStartAndEnd(SplineComp->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Local), SplineComp->GetTangentAtDistanceAlongSpline(i * SplineSectionLength, ESplineCoordinateSpace::Local), SplineComp->GetLocationAtSplinePoint(i + 1, ESplineCoordinateSpace::Local), SplineComp->GetTangentAtDistanceAlongSpline((i+1) * SplineSectionLength, ESplineCoordinateSpace::Local));
		// CreatedSplineMeshComp->AttachToComponent(FirstCreatedSplineMeshComp, FAttachmentTransformRules::SnapToTargetIncludingScale);
		CreatedSplines.Add(CreatedSplineMeshComp);
	}
	
	// SplineComp->UpdateSpline();
}

