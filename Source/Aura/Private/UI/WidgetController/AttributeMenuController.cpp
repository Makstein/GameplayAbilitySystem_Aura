// Copyright Mercury Massif


#include "UI/WidgetController/AttributeMenuController.h"

#include <utility>

#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/AttributeInfo.h"
#include "Player/AuraPlayerState.h"

void UAttributeMenuController::BindCallbacksToDependencies()
{
	const UAuraAttributeSet* AS = CastChecked<UAuraAttributeSet>(AttributeSet);
	check(AttributeInfo);
	for (auto& Pair : AS->TagToAttributes)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Pair.Value()).AddLambda(
			[this, Pair](const FOnAttributeChangeData& Data)
			{
				BroadcastAttributeInfo(Pair.Key, Pair.Value());
			});
	}

	GetAuraPlayerState()->OnAttributePointsChangedDelegate.AddLambda(
		[this](int32 Points)
		{
			AttributePointsChangedDelegate.Broadcast(Points);
		});
}

void UAttributeMenuController::BroadcastInitialParams()
{
	const UAuraAttributeSet* AS = GetAuraAttributeSet();
	check(AttributeInfo);
	for (auto& Pair : AS->TagToAttributes)
	{
		BroadcastAttributeInfo(Pair.Key, Pair.Value());
	}

	AttributePointsChangedDelegate.Broadcast(GetAuraPlayerState()->GetAttributePoints());
}

void UAttributeMenuController::UpgradeAttributes(const FGameplayTag& AttributeTag)
{
	GetAuraAbilitySystemComponent()->UpgradeAttributes(AttributeTag);
}

void UAttributeMenuController::BroadcastAttributeInfo(const FGameplayTag& AttributeTag,
                                                      const FGameplayAttribute& Attribute)
{
	FAuraAttributeInfo Info = AttributeInfo->FindAttributeTag(AttributeTag);
	Info.AttributeValue = Attribute.GetNumericValue(AttributeSet);
	AttributeInfoSignature.Broadcast(Info);
}
