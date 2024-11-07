// Copyright Mercury Massif


#include "UI/WidgetController/AttributeMenuController.h"

#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/AttributeInfo.h"

void UAttributeMenuController::BindCallbacksToDependencies()
{
	Super::BindCallbacksToDependencies();
}

void UAttributeMenuController::BroadcastInitialParams()
{
	const UAuraAttributeSet* AS = CastChecked<UAuraAttributeSet>(AttributeSet);
	
	check(AttributeInfo);

	FAuraAttributeInfo Info = AttributeInfo->FindAttributeTag(FAuraGameplayTags::Get().Attributes_Primary_Strength);
	Info.AttributeValue = AS->GetStrength();
	AttributeInfoSignature.Broadcast(Info);
}