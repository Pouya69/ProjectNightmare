// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ICP_Client.generated.h"

UCLASS()
class PROJECTNIGHTMARE_API AICP_Client : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AICP_Client();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	class SERVICE* IC_ClientService;

public:
	// Internet Computer 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Internet Computer")
		FString IC_SERVER_URL;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Internet Computer")
		FString IC_CANISTER_ID;
	UPROPERTY(EditAnywhere, Category = "Internet Computer")
		FString IC_DID_FILE_PATH_RELATIVE;

	UFUNCTION(BlueprintCallable, Category = "Internet Computer")
		void HelloWorldExample();
	UFUNCTION(BlueprintCallable, Category = "Internet Computer")
		bool InitializeIC_Client();

	UFUNCTION(BlueprintCallable, Category = "Internet Computer")
		bool IsIC_ClientValid() const;

};
