// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FourthDimension_Portal.generated.h"

struct FPortalPassingObject
{
	AActor* ActorPassing;
	FVector LastPosition;
	bool bLastInFront;
	class AThirdPersonPlayerCharacter* PlayerCharacterRef;
	bool bIsMarkedForDeletion = false;

	FPortalPassingObject(AActor* Actor, FVector LastPosition, bool bLastInFront, class AThirdPersonPlayerCharacter* PlayerCharacterRef = nullptr) {
		this->ActorPassing = Actor;
		this->LastPosition = LastPosition;
		this->bLastInFront = bLastInFront;
		this->PlayerCharacterRef = PlayerCharacterRef;
	}
};

UCLASS()
class PROJECTNIGHTMARE_API AFourthDimension_Portal : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFourthDimension_Portal();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	FTimerHandle TimerHandleCutFrame;
	// Comps
	UPROPERTY(EditAnywhere, Category="Comps")
		USceneComponent* RootSceneComp;
	UPROPERTY(EditAnywhere, Category = "Comps")
		UStaticMeshComponent* DoorMeshComp;
	UPROPERTY(EditAnywhere, Category = "Comps")
		UStaticMeshComponent* OtherSideViewComp;
	UPROPERTY(EditAnywhere, Category = "Comps")
		UStaticMeshComponent* PlaneIntersectionComp;
	UPROPERTY(EditAnywhere, Category = "Comps")
		class USceneCaptureComponent2D* PortalCameraComp;
	UPROPERTY(EditAnywhere, Category = "Comps")
		class UBoxComponent* PlayerNearbyBoundsBoxComp;
	UPROPERTY(EditAnywhere, Category = "Comps")
		class UCameraComponent* PlayerCameraComp;
	UPROPERTY(EditAnywhere, Category = "Comps")
		class UBoxComponent* OverlapBoxComp;
	UPROPERTY(EditAnywhere, Category = "Comps")
		class UNiagaraComponent* NiagaraComp;
	class UArrowComponent* ForwardDirectionArrowComp;

	class UMaterialInstanceDynamic* OtherSide_MID;
	class UTextureRenderTarget2D* PortalRenderTarget2D;

	class APlayerCameraManager* PlayerCameraManagerRef;

	UPROPERTY(EditAnywhere)
		float PortalQuality = 1.2f;


public:
	TArray<FPortalPassingObject*> ActorsPassingPortal;
	UFUNCTION()
		void DoorAreaBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndexbool, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
		void DoorAreaEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndexbool);

	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(BlueprintCallable)
		void DestroyPortal();

	UFUNCTION(BlueprintCallable)
		void TeleportPlayer(class AThirdPersonPlayerCharacter* PlayerCharacterRef);
	UPROPERTY(EditAnywhere)
		AFourthDimension_Portal* LinkedPortal;

	void UpdateSceneCapture();
	void CheckResoultion();
	void InitializePortal(AFourthDimension_Portal* OtherPortal, float PlayerTimeDilation);
	void SetClipPlanes();
	bool ShouldTeleport(FPortalPassingObject* PortalPassingObject, int Index);
	bool IsPointCrossingPortal(FPortalPassingObject* PortalPassingObject, int Index);
	bool TeleportObject(FPortalPassingObject* PortalPassingObject, int Index);
	UWorld* World;


};
