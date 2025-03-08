// Fill out your copyright notice in the Description page of Project Settings.


#include "DronePawn.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ThirdPersonPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "InteractablePushableComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"

// Sets default values
ADronePawn::ADronePawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
}

// Called when the game starts or when spawned
void ADronePawn::BeginPlay()
{
	Super::BeginPlay();
	SkeletalMeshComp = FindComponentByClass<USkeletalMeshComponent>();
	Camera1stPersonComp = FindComponentByTag<UCameraComponent>("First Person");
	Camera3rdPersonComp = FindComponentByTag<UCameraComponent>("Third Person");
	if (!Camera1stPersonComp || !Camera3rdPersonComp)
		UE_LOG(LogTemp, Error, TEXT("Drone's Camera's are not set up correctly."));
	Health = MaxHealth;
	OnActorHit.AddDynamic(this, &ADronePawn::OnCollisionWithObject);
	PhysicsHandle = FindComponentByClass<UPhysicsHandleComponent>();
	// bUseControllerRotationYaw = bIsAiming;
}

// Called every frame
void ADronePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	const float MyMass = SkeletalMeshComp->GetMass();
	const float CargoMass = GetTotalMassOfCargo();
	UE_LOG(LogTemp, Warning, TEXT("Self: %f, CARGO: %f"), MyMass, CargoMass);
	if (CargoMass > MyMass)
		AddActorWorldOffset(FVector::DownVector * (CargoMass - MyMass) * 0.1, true);
	UPrimitiveComponent* GrabbedComp = PhysicsHandle->GetGrabbedComponent();
	if (GrabbedComp != nullptr) {
		FRotator NewRotation = GetActorRotation();
		NewRotation.Pitch = 0;
		PhysicsHandle->SetTargetLocationAndRotation(GetActorLocation() - (GetActorUpVector() * (GrabbedComp->GetMass() > SmallItemMass ? 1 : 0.25) * ItemGrabRange), NewRotation);
		return;
	}
	FHitResult HitResult;
	const FVector Start = GetActorLocation();
	const FVector End = Start - (GetActorUpVector() * ItemGrabRange);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	const bool bIsHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);
	if (!bIsHit) {
		if (CurrentComponentInFocusForGrab != nullptr)
			CurrentComponentInFocusForGrab->SetOverlayMaterial(nullptr);
		return;
	}
	if (!HitResult.GetComponent()->IsSimulatingPhysics()) return;
	if (UMeshComponent* MeshCompFocus = Cast<UMeshComponent>(HitResult.GetComponent())) {
		if (CurrentComponentInFocusForGrab != nullptr)
			CurrentComponentInFocusForGrab->SetOverlayMaterial(nullptr);
		CurrentComponentInFocusForGrab = MeshCompFocus;
		CurrentComponentInFocusForGrab->SetOverlayMaterial(ItemGrabFocusMaterial);
	}
	
}

// Called to bind functionality to input
void ADronePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	UEnhancedInputComponent* PlayerEnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!PlayerEnhancedInputComponent) return;
	PlayerEnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ADronePawn::Move);
	PlayerEnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ADronePawn::Look);
	PlayerEnhancedInputComponent->BindAction(MoveZAction, ETriggerEvent::Triggered, this, &ADronePawn::MoveZAxis);
	PlayerEnhancedInputComponent->BindAction(GrabAction, ETriggerEvent::Triggered, this, &ADronePawn::Grab);
	PlayerEnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &ADronePawn::Aim);
	PlayerEnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Triggered, this, &ADronePawn::Shoot);
	PlayerEnhancedInputComponent->BindAction(ReturnToPlayerAction, ETriggerEvent::Triggered, this, &ADronePawn::GoBackToPlayer);
}

float ADronePawn::GetTotalMassOfCargo() const
{
	const bool bHasItemGrabbed = PhysicsHandle->GetGrabbedComponent() != nullptr;
	TArray<FHitResult> HitResults;
	const FVector Start = GetActorLocation();
	const FVector End = Start + (FVector::UpVector * (5 + DroneRadius));
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	if (bHasItemGrabbed)
		QueryParams.AddIgnoredComponent(PhysicsHandle->GetGrabbedComponent());
	const bool bIsHit = GetWorld()->SweepMultiByChannel(HitResults, Start, End, FQuat::Identity, ECC_WorldStatic, FCollisionShape::MakeSphere(DroneRadius), QueryParams);
	if (!bIsHit)
		return (bHasItemGrabbed ? PhysicsHandle->GetGrabbedComponent()->GetMass() + GetTotalMassOnTopOfHandledCargo() : 0);
	float TotalMass = bHasItemGrabbed ? GetTotalMassOnTopOfHandledCargo() : 0.f;
	for (const FHitResult& HitResult : HitResults) {
		if (HitResult.GetComponent()->IsSimulatingPhysics())
			TotalMass += HitResult.GetComponent()->GetMass();
		else if (ACharacterBase* CharacterOnTop = Cast<ACharacterBase>(HitResult.GetActor()))
			TotalMass += CharacterOnTop->GetCharacterMass();
	}
		
	return TotalMass + (bHasItemGrabbed ? PhysicsHandle->GetGrabbedComponent()->GetMass() : 0);
}

float ADronePawn::GetTotalMassOnTopOfHandledCargo() const {
	UPrimitiveComponent* GrabbedComp = PhysicsHandle->GetGrabbedComponent();
	TArray<FHitResult> HitResults;
	const FBox box = GrabbedComp->GetLocalBounds().GetBox();
	const FVector Start = GrabbedComp->GetCenterOfMass();
	const FVector BoxSize = box.GetExtent();
	// const FRotator rotation = GrabbedComp->GetComponentRotation();
	const FVector End = Start + (FVector::UpVector * (5 + BoxSize.Z));
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredComponent(GrabbedComp);
	// DrawDebugBox(GetWorld(), End, FVector(BoxSize.X + BoxSize.X / 4, BoxSize.Y + BoxSize.Y / 4, 3), FColor::Red);
	const bool bIsHit = GetWorld()->SweepMultiByChannel(HitResults, Start, End, FQuat::Identity, ECC_WorldStatic, FCollisionShape::MakeBox(FVector(BoxSize.X+ BoxSize.X/4, BoxSize.Y+ BoxSize.Y / 4, 3)), QueryParams);
	if (!bIsHit)
		return 0.f;
	float TotalMass = 0.f;
	for (const FHitResult& HitResult : HitResults) {
		if (HitResult.GetComponent()->IsSimulatingPhysics())
			TotalMass += HitResult.GetComponent()->GetMass();
		else if (ACharacterBase* CharacterOnTop = Cast<ACharacterBase>(HitResult.GetActor()))
			TotalMass += CharacterOnTop->GetCharacterMass();
	}

	return TotalMass;
}

void ADronePawn::Look(const FInputActionInstance& ActionInstance)
{
	// if (!GetLocalViewingPlayerController()->InputEnabled()) return;
	FVector2D Axis = ActionInstance.GetValue().Get<FVector2D>() * PlayerCharacterRef->MouseSensivity;
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void ADronePawn::Grab(const FInputActionInstance& ActionInstance)
{
	// if (!ActionInstance.GetValue().Get<bool>()) return;
	if (HasItemGrabbed()) {
		ReleaseGrabbedItem();
		return;
	}
	if (CurrentComponentInFocusForGrab != nullptr) {
		FHitResult HitResult;
		const FVector Start = GetActorLocation();
		const FVector End = Start - (GetActorUpVector() * ItemGrabRange);
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		const bool bIsHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);
		if (!bIsHit) {
			CurrentComponentInFocusForGrab->SetOverlayMaterial(nullptr);
			CurrentComponentInFocusForGrab = nullptr;
			Grab(FInputActionInstance());
			return;
		}
		CurrentComponentInFocusForGrab->SetOverlayMaterial(nullptr);
		// CurrentComponentInFocusForGrab->SetCollisionResponseToAllChannels(ECR_Ignore);
		// PhysicsHandle->GrabComponentAtLocation(CurrentComponentInFocusForGrab, FName(""), HitResult.ImpactPoint);
		// UE_LOG(LogTemp, Warning, TEXT("Grabbed: %d"), HasItemGrabbed() ? 1 : 0);
		PhysicsHandle->GrabComponentAtLocationWithRotation(CurrentComponentInFocusForGrab, FName("BONE_NAME"), CurrentComponentInFocusForGrab->GetComponentLocation(), CurrentComponentInFocusForGrab->GetComponentRotation());
	
	}
}

/*void ADronePawn::Roll(const FInputActionInstance& ActionInstance)
{
	AddActorLocalRotation(FRotator(0, 0, ActionInstance.GetValue().Get<float>() * DroneRollSpeed), true);
}*/

void ADronePawn::Move(const FInputActionInstance& ActionInstance)
{
	const FVector2D Direction2D = ActionInstance.GetValue().Get<FVector2D>() * DroneMovementSpeed;
	// UE_LOG(LogTemp, Warning, TEXT("X: %f, Y: %f"), Direction2D.X, Direction2D.Y);
	AddMovementInput(GetActorForwardVector(), Direction2D.Y, true);
	AddMovementInput(GetActorRightVector(), Direction2D.X, true);
	// PlayerCameraManager->StartCameraShake(WalkingCameraShake);
}

void ADronePawn::MoveZAxis(const FInputActionInstance& ActionInstance)
{
	AddMovementInput(GetActorUpVector(), ActionInstance.GetValue().Get<float>() * DroneMovementSpeed / 1.5);
}

void ADronePawn::GoBackToPlayer(const FInputActionInstance& ActionInstance)
{
	ReturnToPlayer();
}

void ADronePawn::ReturnToPlayer(bool bGotDestroyed)
{
	APlayerController* PlayerController = GetLocalViewingPlayerController();
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		Subsystem->RemoveMappingContext(DroneMappingContext);
		Subsystem->AddMappingContext(PlayerCharacterRef->ThirdPersonMappingContext, 0);
	}
	PlayerController->UnPossess();
	PlayerController->Possess(PlayerCharacterRef);
	if (bGotDestroyed) {
		PlayerCharacterRef->DroneInAir = nullptr;
		ReleaseGrabbedItem();
		Destroy();
		PlayerCharacterRef->DroneDestroyed();
		return;
	}
	if (FVector::Dist(GetActorLocation(), PlayerCharacterRef->GetActorLocation()) <= DronePlayerCollectDistance) {
		PlayerCharacterRef->DroneInAir = nullptr;
		Destroy();
	}
}

void ADronePawn::Shoot(const FInputActionInstance& ActionInstance)
{
	if (PlayerCharacterRef->DroneRocketsLeft == 0) return;
	// TODO: Shoot
	PlayerCharacterRef->DroneRocketsLeft--;
}

void ADronePawn::Aim(const FInputActionInstance& ActionInstance)
{
	bIsAiming = ActionInstance.GetValue().Get<bool>();
	Camera1stPersonComp->SetActive(bIsAiming);
	Camera3rdPersonComp->SetActive(!bIsAiming);
	// bUseControllerRotationYaw = bIsAiming;
}

void ADronePawn::OnCollisionWithObject(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
{
	// AddActorWorldOffset(DronePushbackForce * NormalImpulse, true);
	if (GetLocalViewingPlayerController() == nullptr) return;
	if (UInteractablePushableComponent* PushedComp = Cast<UInteractablePushableComponent>(Hit.GetComponent())) {
		PushedComp->InteractPush();
	}
	if (ACharacterBase* CharacterHit = Cast<ACharacterBase>(OtherActor)) {
		CharacterHit->ReduceHealth(DamageToCharactersHit);
	}
}

void ADronePawn::TakeControlOfDrone(AThirdPersonPlayerCharacter* InPlayerCharacterRef)
{
	PlayerCharacterRef = InPlayerCharacterRef;
	APlayerController* PlayerController = PlayerCharacterRef->MyPlayerController;
	PlayerController->UnPossess();
	PlayerController->Possess(this);
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		Subsystem->RemoveMappingContext(PlayerCharacterRef->ThirdPersonMappingContext);
		Subsystem->AddMappingContext(DroneMappingContext, 0);
	}
}

bool ADronePawn::HasItemGrabbed() const
{
	return PhysicsHandle->GrabbedComponent != nullptr;
}

void ADronePawn::ReleaseGrabbedItem()
{
	if (PhysicsHandle->GrabbedComponent != nullptr) {
		PhysicsHandle->ReleaseComponent();
	}
}
