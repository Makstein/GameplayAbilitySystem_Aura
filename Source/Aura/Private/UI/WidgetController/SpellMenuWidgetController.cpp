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

	GetAuraAbilitySystemComponent()->AbilityEquippedDelegate.AddUObject(this, &USpellMenuWidgetController::OnAbilityEquipped);

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
	if (bWaitingForEquipSelection)
	{
		const auto SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbilityTag).AbilityType;
		StopWaitForEquipSelectionDelegate.Broadcast(SelectedAbilityType);
		bWaitingForEquipSelection = false;
	}
	
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

void USpellMenuWidgetController::GlobeDeselect()
{
	if (bWaitingForEquipSelection)
	{
		const auto SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.AbilityTag).AbilityType;
		StopWaitForEquipSelectionDelegate.Broadcast(SelectedAbilityType);
		bWaitingForEquipSelection = false;
	}
	
	SelectedAbility.StatusTag = FAuraGameplayTags::Get().Abilities_Status_Locked;
	SpellGlobeSelectedDelegate.Broadcast(false, false, FString(), FString());
}

void USpellMenuWidgetController::EquipButtonPressed()
{
	const auto AbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.AbilityTag).AbilityType;
	WaitForEquipSelectionDelegate.Broadcast(AbilityType);
	bWaitingForEquipSelection = true;

	const FGameplayTag SelectedStatus = GetAuraAbilitySystemComponent()->GetStatusFromAbilityTag(SelectedAbility.AbilityTag);
	if (SelectedStatus.MatchesTagExact(FAuraGameplayTags::Get().Abilities_Status_Equipped))
	{
		SelectedSlot = GetAuraAbilitySystemComponent()->GetInputTagAbilityTag(SelectedAbility.AbilityTag);
	}
}

void USpellMenuWidgetController::EquipRowGlobePressed(const FGameplayTag& SlotTag, const FGameplayTag& AbilityType)
{
	if (!bWaitingForEquipSelection) return;
	const FGameplayTag SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.AbilityTag).AbilityType;
	if (!SelectedAbilityType.MatchesTagExact(AbilityType)) return;

	GetAuraAbilitySystemComponent()->ServerEquipAbility(SelectedAbility.AbilityTag, SlotTag);
}

void USpellMenuWidgetController::OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& SlotTag,
	const FGameplayTag& StatusTag, const FGameplayTag& PrevSlotTag)
{
	bWaitingForEquipSelection = false;

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

	StopWaitForEquipSelectionDelegate.Broadcast(AbilityInfo->FindAbilityInfoForTag(AbilityTag).AbilityType);
	SpellGlobeReassignedDelegate.Broadcast(AbilityTag);
	GlobeDeselect();
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
