// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_SetBlackboardValue.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_SetBlackboardValue::UBTTask_SetBlackboardValue()
{
	NodeName = TEXT("Set Blackboard Value");
}

EBTNodeResult::Type UBTTask_SetBlackboardValue::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

	switch (ValueType)
	{
	case EValueType::BOOL:
		BlackboardComp->SetValueAsBool(GetSelectedBlackboardKey(), ValueBool);
		break;
	case EValueType::FLOAT:
		BlackboardComp->SetValueAsFloat(GetSelectedBlackboardKey(), ValueFloat);
		break;
	case EValueType::INT:
		BlackboardComp->SetValueAsInt(GetSelectedBlackboardKey(), ValueInt);
		break;
	default:
		break;
	}
	return EBTNodeResult::Succeeded;
}
