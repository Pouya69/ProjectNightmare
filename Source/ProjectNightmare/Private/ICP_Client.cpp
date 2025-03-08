// Fill out your copyright notice in the Description page of Project Settings.


#include "ICP_Client.h"
#include "agent.h"
#include "helper.h"
#include "idl_value.h"
#include "icp-client-cpp/examples-cpp/src/declarations/CUSTOM/CUSTOM.hpp"
// using namespace zondax;

// Sets default values
AICP_Client::AICP_Client()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AICP_Client::BeginPlay()
{
	Super::BeginPlay();
    const bool bServiceInitializeResult = InitializeIC_Client();
}

// Called every frame
void AICP_Client::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AICP_Client::HelloWorldExample()
{
    if (!IsIC_ClientValid()) return;
    std::string arg = "Pouya";
    auto out = IC_ClientService->greet(arg, 80085);

    if (out.index() == 0) {
        // Print message from the canister
        FString Respone = FString(std::get<0>(out).c_str());
        UE_LOG(LogTemp, Warning, TEXT("RESPONSE FROM SERVER: %s"), *Respone);
    }
    else {
        // Get the second string value from the variant
        FString str2 = FString(std::get<1>(out).c_str());
        UE_LOG(LogTemp, Error, TEXT("ERROR RESPONSE: %s"), *str2);
    }
}

bool AICP_Client::InitializeIC_Client()
{

    // Canister info from hello world deploy example

  // canister id, this might need to be change as canister when deployed
  // might change its id
    std::string id_text = std::string(TCHAR_TO_UTF8(*IC_CANISTER_ID));
    // FString FullPath = FPaths::Combine(FPaths::ProjectDir(), TEXT("Source\\icp-client-cpp\\examples-cpp\\hello\\declarations\\rust_hello\\rust_hello_backend.did"));

    FString FullPath = FPaths::Combine(FPaths::ProjectDir(), IC_DID_FILE_PATH_RELATIVE);
    std::string did_file = TCHAR_TO_UTF8(*FullPath);
    if (!std::filesystem::exists(did_file)) {
        UE_LOG(LogTemp, Error, TEXT("ERROR: .did file path is invalid: %s\nPlease check the path inside your IC_Client."), *FullPath);
        return false;
    }
    std::string url = std::string(TCHAR_TO_UTF8(*IC_SERVER_URL));

    std::vector<char> buffer;
    auto bytes_read = did_file_content(did_file, buffer);

    // Get principal form text
    auto principal = zondax::Principal::FromText(id_text);

    if (std::holds_alternative<std::string>(principal)) {
        FString Response = FString(std::get<std::string>(principal).c_str());
        UE_LOG(LogTemp, Error, TEXT("Error DID: %s"), *Response);
        // std::cerr << "Error: " << std::get<std::string>(principal) << std::endl;
        return false;
    }
    
    // Construct anonymoous id
    zondax::Identity anonymousIdentity;

    // Create agent with agent constructor
    auto agent = zondax::Agent::create_agent(url, std::move(anonymousIdentity),
        std::get<zondax::Principal>(principal), buffer);

    if (std::holds_alternative<std::string>(agent)) {
        FString Response = FString(std::get<std::string>(agent).c_str());
        UE_LOG(LogTemp, Error, TEXT("Error AGENT: %s"), *Response);
        // std::cerr << "Error: " << std::get<std::string>(agent) << std::endl;
        return false;
    }

    IC_ClientService = new SERVICE(std::move(std::get<zondax::Agent>(agent)));
    return true;
}

bool AICP_Client::IsIC_ClientValid() const
{
    return IC_ClientService != nullptr;
}
