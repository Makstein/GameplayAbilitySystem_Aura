// Copyright Mercury Massif


#include "UI/WidgetController/SpellMenuWidgetController.h"

#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "Player/AuraPlayerState.h"

void USpellMenuWidgetController::BroadcastInitialParams()
{
	BroadcastAbilityInfo();
	SpellPointChanged.Broadcast(GetAuraPlayerState()->GetSpellPoints());
}

void USpellMenuWidgetController::BindCallbacksToDependencies()
{
	GetAuraAbilitySystemComponent()->AbilityStatusChangedDelegate.AddLambda([this](const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, const int32 Level)
	{
		if (SelectedAbility.AbilityTag.MatchesTagExact(AbilityTag))
		{
			SelectedAbility.StatusTag = StatusTag;
			bool bSpendPointsButtonEnabled = false;
			bool bEquipButtonEnabled = false;
			ShouldEnableButton(StatusTag, GetAuraPlayerState()->GetSpellPoints(), bSpendPointsButtonEnabled, bEquipButtonEnabled);
			FString Description;
			FString NextLevelDescription;
			GetAuraAbilitySystemComponent()->GetDescriptionsByAbilityTag(AbilityTag, Description, NextLevelDescription);
			SpellGlobeSelectedDelegate.Broadcast(bSpendPointsButtonEnabled, bEquipButtonEnabled, Description, NextLevelDescription);
		}
		
		if (AbilityInfo)
		{
			auto Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
			Info.StatusTag = StatusTag;
			AbilityInfoDelegate.Broadcast(Info);
		}
	});

	GetAuraPlayerState()->OnSpellPointsChangedDelegate.AddLambda([this](int32 SpellPoints)
	{
		SpellPointChanged.Broadcast(SpellPoints);
		bool bSpendPointsButtonEnabled = false;
		bool bEquipButtonEnabled = false;
		ShouldEnableButton(SelectedAbility.StatusTag, GetAuraPlayerState()->GetSpellPoints(), bSpendPointsButtonEnabled, bEquipButtonEnabled);
		FString Description;
		FString NextLevelDescription;
		GetAuraAbilitySystemComponent()->GetDescriptionsByAbilityTag(SelectedAbility.AbilityTag, Description, NextLevelDescription);
		SpellGlobeSelectedDelegate.Broadcast(bSpendPointsButtonEnabled, bEquipButtonEnabled, Description, NextLevelDescription);
	});
}

void USpellMenuWidgetController::SpendPointsButtonPressed()
{
	GetAuraAbilitySystemComponent()->ServerSpendSpellPoints(SelectedAbility.AbilityTag);
}

void USpellMenuWidgetController::SpellGlobeButtonPressed(const FGameplayTag& SelectedAbilityTag,
	const FGameplayTag& SelectedStatusTag)
{
	SelectedAbility.AbilityTag = SelectedAbilityTag;
	SelectedAbility.StatusTag = SelectedStatusTag;

	bool bSpendPointsButtonEnabled = false;
	bool bEquipButtonEnabled = false;
	ShouldEnableButton(SelectedAbility.StatusTag, GetAuraPlayerState()->GetSpellPoints(), bSpendPointsButtonEnabled, bEquipButtonEnabled);
	FString Description;
	FString NextLevelDescription;
	GetAuraAbilitySystemComponent()->GetDescriptionsByAbilityTag(SelectedAbility.AbilityTag, Description, NextLevelDescription);
	SpellGlobeSelectedDelegate.Broadcast(bSpendPointsButtonEnabled, bEquipButtonEnabled, Description, NextLevelDescription);
}

void USpellMenuWidgetController::ShouldEnableButton(const FGameplayTag& SelectedStatusTag, int32 CurrentSpellPoints,
                                                    bool& bSpendPointsButtonEnabled, bool& bEquipButtonEnabled)
{
	const auto GameplayTags = FAuraGameplayTags::Get();
	if (!SelectedStatusTag.MatchesTagExact(GameplayTags.Abilities_Status_Locked) && CurrentSpellPoints != 0)
	{
		bSpendPointsButtonEnabled = true;
	}
	if (SelectedStatusTag.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked)
		|| SelectedStatusTag.MatchesTagExact(GameplayTags.Abilities_Status_Equipped))
	{
		bEquipButtonEnabled = true;
	}
}
