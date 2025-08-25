// Copyright Mercury Massif


#include "UI/WidgetController/OverlayWidgetController.h"

#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "AbilitySystem/Data/LevelUpInfo.h"
#include "Player/AuraPlayerState.h"

void UOverlayWidgetController::BroadcastInitialParams()
{
	const UAuraAttributeSet* AuraAttributes = GetAuraAttributeSet();
	OnHealthChanged.Broadcast(AuraAttributes->GetHealth());
	OnMaxHealthChanged.Broadcast(AuraAttributes->GetMaxHealth());
	OnManaChanged.Broadcast(AuraAttributes->GetMana());
	OnMaxManaChanged.Broadcast(AuraAttributes->GetMaxMana());
}

void UOverlayWidgetController::BindCallbacksToDependencies()
{
	UAuraAttributeSet* AuraAttributes = GetAuraAttributeSet();

	GetAuraPlayerState()->OnXPChangedDelegate.AddUObject(this, &UOverlayWidgetController::OnXPChanged);
	GetAuraPlayerState()->OnLevelChangedDelegate.AddLambda(
		[this](int32 NewLevel)
		{
			OnPlayerLevelChanged.Broadcast(NewLevel);
		});
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributes->GetHealthAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnHealthChanged.Broadcast(Data.NewValue);
		});
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributes->GetMaxHealthAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnMaxHealthChanged.Broadcast(Data.NewValue);
		});
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributes->GetManaAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnManaChanged.Broadcast(Data.NewValue);
		});
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributes->GetMaxManaAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnMaxManaChanged.Broadcast(Data.NewValue);
		});

	if (auto AuraASC = GetAuraAbilitySystemComponent())
	{
		AuraASC->AbilityEquippedDelegate.AddUObject(this, &UOverlayWidgetController::OnAbilityEquipped);
		if (AuraASC->bStartupAbilitiesGiven)
		{
			BroadcastAbilityInfo();
		}
		else
		{
			AuraASC->AbilityGivenDelegate.AddUObject(this, &UOverlayWidgetController::BroadcastAbilityInfo);
		}
		
		AuraASC->EffectAssetTags.AddLambda(
        		[this](const FGameplayTagContainer& AssetTags)
        		{
        			for (const FGameplayTag& Tag : AssetTags)
        			{
        				if (FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("Message"));
        					Tag.MatchesTag(MessageTag))
        				{
        					const FUIWidgetRow* Row = GetDataTableRowByTag<FUIWidgetRow>(MessageWidgetDataTable, Tag);
        					MessageWidgetRowDelegate.Broadcast(*Row);
        				}
        			}
        		});
	}
}

void UOverlayWidgetController::OnXPChanged(const int32 NewXP)
{
	const ULevelUpInfo* LevelUpInfo = GetAuraPlayerState()->LevelUpInfo;
	checkf(LevelUpInfo, TEXT("Unable to find LevelUpInfo, fill out AuraPlayerState blueprint"));

	const auto Level = LevelUpInfo->FindLevelForXP(NewXP);
	if (const auto MaxLevel = LevelUpInfo->LevelUpInformation.Num(); Level <= MaxLevel && Level >= 0)
	{
		const auto LevelUpRequirement = LevelUpInfo->LevelUpInformation[Level].LevelUpRequirement;
		const auto PreviousLevelUpRequirement = LevelUpInfo->LevelUpInformation[Level - 1].LevelUpRequirement;
		const auto XPForThisLevel = NewXP - PreviousLevelUpRequirement;
		const auto DeltaLevelRequirement = LevelUpRequirement - PreviousLevelUpRequirement;
		const auto XPBarPrecent = static_cast<float>(XPForThisLevel) / DeltaLevelRequirement;
		OnXPPercentChanged.Broadcast(XPBarPrecent);
	}
}

void UOverlayWidgetController::OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& SlotTag,
	const FGameplayTag& StatusTag, const FGameplayTag& PrevSlotTag) const
{
	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();

	FAuraAbilityInfo LastSlotInfo;
	LastSlotInfo.StatusTag = GameplayTags.Abilities_Status_Unlocked;
	LastSlotInfo.InputTag = PrevSlotTag;
	LastSlotInfo.AbilityTag = GameplayTags.Abilities_None;

	AbilityInfoDelegate.Broadcast(LastSlotInfo);

	FAuraAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	Info.StatusTag = StatusTag;
	Info.InputTag = SlotTag;
	AbilityInfoDelegate.Broadcast(Info);
}
