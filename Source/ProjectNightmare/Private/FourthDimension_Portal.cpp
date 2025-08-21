// Fill out your copyright notice in the Description page of Project Settings.


#include "FourthDimension_Portal.h"
#include "Kismet/GameplayStatics.h"
#include "ThirdPersonPlayerCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/ArrowComponent.h"
#include "Camera/CameraComponent.h"
#include "NiagaraComponent.h"

// Sets default values
AFourthDimension_Portal::AFourthDimension_Portal()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootSceneComp = CreateDefaultSubobject<USceneComponent>(FName("Root Comp"));
	SetRootComponent(RootSceneComp);
	DoorMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(FName("Door Mesh"));
	DoorMeshComp->SetupAttachment(GetRootComponent());
	OtherSideViewComp = CreateDefaultSubobject<UStaticMeshComponent>(FName("Other Side View"));
	OtherSideViewComp->SetupAttachment(GetRootComponent());
	PlaneIntersectionComp = CreateDefaultSubobject<UStaticMeshComponent>(FName("Plane Intersection Test"));
	PlaneIntersectionComp->SetupAttachment(GetRootComponent());
	PortalCameraComp = CreateDefaultSubobject<USceneCaptureComponent2D>(FName("Portal Camera"));
	PortalCameraComp->SetupAttachment(GetRootComponent());
	OverlapBoxComp = CreateDefaultSubobject<UBoxComponent>(FName("Overlap Box Comp"));
	OverlapBoxComp->SetupAttachment(GetRootComponent());
	NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(FName("Niagara Comp"));
	NiagaraComp->SetupAttachment(GetRootComponent());
	PlayerNearbyBoundsBoxComp = CreateDefaultSubobject<UBoxComponent>(FName("Player Nearby Bounds"));
	PlayerNearbyBoundsBoxComp->SetupAttachment(GetRootComponent());
	PlayerCameraComp = CreateDefaultSubobject<UCameraComponent>(FName("Player Camera"));
	PlaneIntersectionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PlaneIntersectionComp->SetCollisionObjectType(ECC_WorldStatic);
	PlaneIntersectionComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Overlap);
	PlaneIntersectionComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Overlap);
	SetTickGroup(ETickingGroup::TG_PostUpdateWork);
}

// Called when the game starts or when spawned
void AFourthDimension_Portal::BeginPlay()
{
	Super::BeginPlay();
	ForwardDirectionArrowComp = FindComponentByClass<UArrowComponent>();
	SetClipPlanes();
	OtherSide_MID = OtherSideViewComp->CreateDynamicMaterialInstance(0, OtherSideViewComp->GetMaterial(0));
	OtherSideViewComp->SetMaterial(0, OtherSide_MID);
	World = GetWorld();
	const FVector2D ViewportSize2D = UWidgetLayoutLibrary::GetViewportSize(World);
	PortalRenderTarget2D = UKismetRenderingLibrary::CreateRenderTarget2D(World, FMath::TruncToInt32(ViewportSize2D.X * PortalQuality), FMath::TruncToInt32(ViewportSize2D.Y * PortalQuality));
	OtherSide_MID->SetTextureParameterValue(FName("Texture"), PortalRenderTarget2D);
	OverlapBoxComp->OnComponentBeginOverlap.AddDynamic(this, &AFourthDimension_Portal::DoorAreaBeginOverlap);
	OverlapBoxComp->OnComponentEndOverlap.AddDynamic(this, &AFourthDimension_Portal::DoorAreaEndOverlap);
	PlayerCameraManagerRef = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	// UGameplayStatics::GetActorOfClass(GetWorld(), )
}

void AFourthDimension_Portal::DoorAreaBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndexbool, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == this) return;
	ActorsPassingPortal.Add(new FPortalPassingObject(OtherActor, OtherActor->GetActorLocation(), false, Cast<AThirdPersonPlayerCharacter>(OtherActor)));
}

void AFourthDimension_Portal::DoorAreaEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndexbool)
{
	if (OtherActor == this) return;
	for (int i = 0; i < ActorsPassingPortal.Num(); i++)
	{
		FPortalPassingObject* ObjectPassing = ActorsPassingPortal[i];
		if (ObjectPassing->ActorPassing != OtherActor) continue;
		ActorsPassingPortal.RemoveAt(i);
		break;
	}
}

// Called every frame
void AFourthDimension_Portal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateSceneCapture();
	CheckResoultion();
	if (ActorsPassingPortal.IsEmpty()) return;
	for (int i = 0; i < ActorsPassingPortal.Num(); i++)
	{
		FPortalPassingObject* ObjectPassing = ActorsPassingPortal[i];
		if (ObjectPassing->bIsMarkedForDeletion) continue;
		if (ShouldTeleport(ObjectPassing, i))
			TeleportObject(ObjectPassing, i);
	}
	
}

void AFourthDimension_Portal::DestroyPortal()
{
	// TODO: Effects and stuff and etc.

	Destroy();
}

void AFourthDimension_Portal::TeleportPlayer(AThirdPersonPlayerCharacter* PlayerCharacterRef)
{
	const FTransform SelfActorTransform = GetActorTransform();
	const FTransform LinkedPortalTransform = LinkedPortal->GetActorTransform();
	FVector BrokenX, BrokenY, BrokenZ;
	UKismetMathLibrary::BreakRotIntoAxes(PlayerCharacterRef->MyPlayerController->GetControlRotation(), BrokenX, BrokenY, BrokenZ);
	const FVector InverseDirectionX = UKismetMathLibrary::InverseTransformDirection(SelfActorTransform, BrokenX);
	const FVector InverseDirectionY = UKismetMathLibrary::InverseTransformDirection(SelfActorTransform, BrokenY);
	const FVector InverseDirectionZ = UKismetMathLibrary::InverseTransformDirection(SelfActorTransform, BrokenZ);

	const FVector TransformedVectorX = UKismetMathLibrary::TransformDirection(LinkedPortalTransform, UKismetMathLibrary::MirrorVectorByNormal(UKismetMathLibrary::MirrorVectorByNormal(InverseDirectionX, FVector(1, 0, 0)), FVector(0, 1, 0)));
	const FVector TransformedVectorY = UKismetMathLibrary::TransformDirection(LinkedPortalTransform, UKismetMathLibrary::MirrorVectorByNormal(UKismetMathLibrary::MirrorVectorByNormal(InverseDirectionY, FVector(1, 0, 0)), FVector(0, 1, 0)));
	const FVector TransformedVectorZ = UKismetMathLibrary::TransformDirection(LinkedPortalTransform, UKismetMathLibrary::MirrorVectorByNormal(UKismetMathLibrary::MirrorVectorByNormal(InverseDirectionZ, FVector(1, 0, 0)), FVector(0, 1, 0)));

	PlayerCharacterRef->DisableCameraLagUntilNextFrame();
	FRotator FinalRotation = UKismetMathLibrary::MakeRotationFromAxes(TransformedVectorX, TransformedVectorY, TransformedVectorZ);
	FinalRotation.Roll = 0;
	PlayerCharacterRef->MyPlayerController->SetControlRotation(FinalRotation);
	PlayerCameraManagerRef->SetGameCameraCutThisFrame();
	FTimerDelegate Delegate;
	Delegate.BindLambda([&]() {
		PlayerCameraManagerRef->SetGameCameraCutThisFrame();
	});
	GetWorldTimerManager().SetTimer(TimerHandleCutFrame, Delegate, CustomTimeDilation/20, false);

	// Update Velocity
	FVector OldVelocity = PlayerCharacterRef->GetCharacterVelocity();
	const FVector OldVelocityCONST = OldVelocity;
	UKismetMathLibrary::Vector_Normalize(OldVelocity);
	PlayerCharacterRef->SetCharacterVelocity(
		UKismetMathLibrary::TransformDirection(LinkedPortalTransform, UKismetMathLibrary::MirrorVectorByNormal(UKismetMathLibrary::MirrorVectorByNormal(UKismetMathLibrary::InverseTransformDirection(SelfActorTransform, OldVelocity), FVector(1, 0, 0)), FVector(0, 1, 0)) * OldVelocityCONST.Length())
	);
}

void AFourthDimension_Portal::UpdateSceneCapture()
{
	if (!LinkedPortal) return;
	const FTransform SelfActorTransform = GetActorTransform();
	const FVector SelfScale = SelfActorTransform.GetScale3D();
	const FTransform NewTransform = FTransform(SelfActorTransform.GetRotation(), SelfActorTransform.GetLocation(), FVector(SelfScale.X * -1, SelfScale.Y * -1, SelfScale.Z));
	const FVector InverseLoc = UKismetMathLibrary::InverseTransformLocation(NewTransform, PlayerCameraManagerRef->GetTransformComponent()->GetComponentLocation());
	const FTransform LinkedPortalTransform = LinkedPortal->GetActorTransform();

	FVector BrokenX, BrokenY, BrokenZ;
	UKismetMathLibrary::BreakRotIntoAxes(PlayerCameraManagerRef->GetTransformComponent()->GetComponentRotation(), BrokenX, BrokenY, BrokenZ);
	const FVector InverseDirectionX = UKismetMathLibrary::InverseTransformDirection(SelfActorTransform, BrokenX);
	const FVector InverseDirectionY = UKismetMathLibrary::InverseTransformDirection(SelfActorTransform, BrokenY);
	const FVector InverseDirectionZ = UKismetMathLibrary::InverseTransformDirection(SelfActorTransform, BrokenZ);
	
	const FVector TransformedVectorX = UKismetMathLibrary::TransformDirection(LinkedPortalTransform, UKismetMathLibrary::MirrorVectorByNormal(UKismetMathLibrary::MirrorVectorByNormal(InverseDirectionX, FVector(1, 0, 0)), FVector(0, 1, 0)));
	const FVector TransformedVectorY = UKismetMathLibrary::TransformDirection(LinkedPortalTransform, UKismetMathLibrary::MirrorVectorByNormal(UKismetMathLibrary::MirrorVectorByNormal(InverseDirectionY, FVector(1, 0, 0)), FVector(0, 1, 0)));
	const FVector TransformedVectorZ = UKismetMathLibrary::TransformDirection(LinkedPortalTransform, UKismetMathLibrary::MirrorVectorByNormal(UKismetMathLibrary::MirrorVectorByNormal(InverseDirectionZ, FVector(1, 0, 0)), FVector(0, 1, 0)));

	LinkedPortal->PortalCameraComp->SetWorldLocationAndRotation(UKismetMathLibrary::TransformLocation(LinkedPortalTransform, InverseLoc), UKismetMathLibrary::MakeRotationFromAxes(TransformedVectorX, TransformedVectorY, TransformedVectorZ));


}

void AFourthDimension_Portal::CheckResoultion()
{
	const FVector2D ViewportSize2D = UWidgetLayoutLibrary::GetViewportSize(World);
	const float ScaledViewportX = FMath::TruncToInt32(ViewportSize2D.X * PortalQuality);
	const float ScaledViewportY = FMath::TruncToInt32(ViewportSize2D.Y * PortalQuality);
	if (ScaledViewportX == PortalRenderTarget2D->SizeX && ScaledViewportY == PortalRenderTarget2D->SizeY) return;
	PortalRenderTarget2D->ResizeTarget(ScaledViewportX, ScaledViewportY);
}

void AFourthDimension_Portal::InitializePortal(AFourthDimension_Portal* OtherPortal, float PlayerTimeDilation)
{
	LinkedPortal = OtherPortal;
	OtherPortal->LinkedPortal = this;

	PortalCameraComp->TextureTarget = LinkedPortal->PortalRenderTarget2D;
	LinkedPortal->PortalCameraComp->TextureTarget = PortalRenderTarget2D;
	NiagaraComp->SetCustomTimeDilation(PlayerTimeDilation/5);
	CustomTimeDilation = PlayerTimeDilation;
	
	// PortalCameraComp->CaptureScene();
	// LinkedPortal->PortalCameraComp->CaptureScene();
}

void AFourthDimension_Portal::SetClipPlanes()
{
	PortalCameraComp->bEnableClipPlane = true;
	const FVector ForwardArrowDirection = ForwardDirectionArrowComp->GetForwardVector();
	PortalCameraComp->ClipPlaneBase = OtherSideViewComp->GetComponentLocation() + (ForwardArrowDirection * -3.f);
	PortalCameraComp->ClipPlaneNormal = ForwardArrowDirection;
}

bool AFourthDimension_Portal::ShouldTeleport(FPortalPassingObject* PortalPassingObject, int Index)
{
	//TArray<AActor*> NearbyBoundsOverlaps;
	//PlayerNearbyBoundsBoxComp->GetOverlappingActors(NearbyBoundsOverlaps);
	//if (NearbyBoundsOverlaps.IsEmpty()) return false;
	if (PortalPassingObject->bIsMarkedForDeletion) return false;
	const bool bResult = IsPointCrossingPortal(PortalPassingObject, Index);
	return bResult;
}

bool AFourthDimension_Portal::IsPointCrossingPortal(FPortalPassingObject* PortalPassingObject, int Index)
{
	const FVector PortalNormal = ForwardDirectionArrowComp->GetForwardVector();
	const FVector PortalLocation = RootSceneComp->GetComponentLocation();
	const FVector Point = PortalPassingObject->ActorPassing->GetActorLocation();
	const float DotProduct = FVector::DotProduct(PortalNormal, (Point - PortalLocation));
	const bool bIsInFront = DotProduct >= 0;
	float OutT;
	FVector OutIntersection;
	const bool bIsIntersecting = UKismetMathLibrary::LinePlaneIntersection(PortalPassingObject->LastPosition, Point, UKismetMathLibrary::MakePlaneFromPointAndNormal(PortalLocation, PortalNormal), OutT, OutIntersection);
	// UE_LOG(LogTemp, Warning, TEXT("Last in front: %d, In front: %d, IsIntersecting: %d"), PortalPassingObject->bLastInFront ? 1 : 0, bIsInFront ? 1 : 0, bIsIntersecting ? 1 : 0);
	const bool bResult = PortalPassingObject->bLastInFront && !bIsInFront && bIsIntersecting;
	PortalPassingObject->LastPosition = Point;
	PortalPassingObject->bLastInFront = bIsInFront;
	
	return bResult;
}

bool AFourthDimension_Portal::TeleportObject(FPortalPassingObject* PortalPassingObject, int Index)
{
	if (!LinkedPortal || !PortalPassingObject || PortalPassingObject->bIsMarkedForDeletion) return false;
	if (PortalPassingObject->PlayerCharacterRef) {
		TeleportPlayer(PortalPassingObject->PlayerCharacterRef);
	}
	const FTransform SelfActorTransform = GetActorTransform();
	const FVector SelfScale = SelfActorTransform.GetScale3D();
	const FTransform NewTransform = FTransform(SelfActorTransform.GetRotation(), SelfActorTransform.GetLocation(), FVector(SelfScale.X * -1, SelfScale.Y * -1, SelfScale.Z));
	const FVector InverseLoc = UKismetMathLibrary::InverseTransformLocation(NewTransform, PortalPassingObject->ActorPassing->GetActorLocation());
	const FTransform LinkedPortalTransform = LinkedPortal->GetActorTransform();

	FVector BrokenX, BrokenY, BrokenZ;
	UKismetMathLibrary::BreakRotIntoAxes(PortalPassingObject->ActorPassing->GetActorRotation(), BrokenX, BrokenY, BrokenZ);
	const FVector InverseDirectionX = UKismetMathLibrary::InverseTransformDirection(SelfActorTransform, BrokenX);
	const FVector InverseDirectionY = UKismetMathLibrary::InverseTransformDirection(SelfActorTransform, BrokenY);
	const FVector InverseDirectionZ = UKismetMathLibrary::InverseTransformDirection(SelfActorTransform, BrokenZ);

	const FVector TransformedVectorX = UKismetMathLibrary::TransformDirection(LinkedPortalTransform, UKismetMathLibrary::MirrorVectorByNormal(UKismetMathLibrary::MirrorVectorByNormal(InverseDirectionX, FVector(1, 0, 0)), FVector(0, 1, 0)));
	const FVector TransformedVectorY = UKismetMathLibrary::TransformDirection(LinkedPortalTransform, UKismetMathLibrary::MirrorVectorByNormal(UKismetMathLibrary::MirrorVectorByNormal(InverseDirectionY, FVector(1, 0, 0)), FVector(0, 1, 0)));
	const FVector TransformedVectorZ = UKismetMathLibrary::TransformDirection(LinkedPortalTransform, UKismetMathLibrary::MirrorVectorByNormal(UKismetMathLibrary::MirrorVectorByNormal(InverseDirectionZ, FVector(1, 0, 0)), FVector(0, 1, 0)));
	PortalPassingObject->LastPosition = PortalPassingObject->ActorPassing->GetActorLocation();
	// PortalPassingObject->bLastInFront = true;
	
	PortalPassingObject->ActorPassing->SetActorLocationAndRotation(UKismetMathLibrary::TransformLocation(LinkedPortalTransform, InverseLoc), UKismetMathLibrary::MakeRotationFromAxes(TransformedVectorX, TransformedVectorY, TransformedVectorZ), false, nullptr);
	// LinkedPortal->LastPosition = ;
	// LinkedPortal->bLastInFront = false;
	PortalPassingObject->bIsMarkedForDeletion = true;
	return true;
}
