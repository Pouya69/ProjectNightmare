// Fill out your copyright notice in the Description page of Project Settings.


#include "EQS_QueryContext_Player.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "FemaleMutantAIController.h"
#include "FemaleMutantCharacter.h"

void UEQS_QueryContext_Player::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	Super::ProvideContext(QueryInstance, ContextData);
	if (AFemaleMutantCharacter* FemaleCharacterOwner = Cast<AFemaleMutantCharacter>(QueryInstance.Owner))
		UEnvQueryItemType_Actor::SetContextHelper(ContextData, FemaleCharacterOwner->EnemyAIController->SoftPlayerRef_READONLY);
}
