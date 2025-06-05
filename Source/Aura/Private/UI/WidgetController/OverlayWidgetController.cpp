// Copyright Mercury Massif


#include "UI/WidgetController/OverlayWidgetController.h"

#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "AbilitySystem/Data/LevelUpInfo.h"
#include "Player/AuraPlayerState.h"

void UOverlayWidgetController::BroadcastInitialParams()
{
	const UAuraAttributeSet* AuraAttributes = Cast<UAuraAttributeSet>(AttributeSet);
	OnHealthChanged.Broadcast(AuraAttributes->GetHealth());
	OnMaxHealthChanged.Broadcast(AuraAttributes->GetMaxHealth());
	OnManaChanged.Broadcast(AuraAttributes->GetMana());
	OnMaxManaChanged.Broadcast(AuraAttributes->GetMaxMana());
}

void UOverlayWidgetController::BindCallbacksToDependencies()
{
	UAuraAttributeSet* AuraAttributes = Cast<UAuraAttributeSet>(AttributeSet);
	AAuraPlayerState* AuraPlayerState = CastChecked<AAuraPlayerState>(PlayerState);

	AuraPlayerState->OnXPChangedDelegate.AddUObject(this, &UOverlayWidgetController::OnXPChanged);
	AuraPlayerState->OnLevelChangedDelegate.AddLambda(
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

	if (auto AuraASC = Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent))
	{
		if (AuraASC->bStartupAbilitiesGiven)
		{
			OnInitializeStartupAbilities(AuraASC);
		}
		else
		{
			AuraASC->AbilityGivenDelegate.AddUObject(this, &UOverlayWidgetController::OnInitializeStartupAbilities);
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

void UOverlayWidgetController::OnXPChanged(const int32 NewXP) const
{
	const AAuraPlayerState* AuraPlayerState = CastChecked<AAuraPlayerState>(PlayerState);
	const ULevelUpInfo* LevelUpInfo = AuraPlayerState->LevelUpInfo;
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

void UOverlayWidgetController::OnInitializeStartupAbilities(UAuraAbilitySystemComponent* AuraAsc)
{
	if (!AuraAsc->bStartupAbilitiesGiven) return;

	FForeachAbility BroadcastDelegate;
	BroadcastDelegate.BindLambda([this, AuraAsc](const FGameplayAbilitySpec& Data)
	{
		auto Info = AbilityInfo->FindAbilityInfoForTag(AuraAsc->GetAbilityTagFromSpec(Data));
		Info.InputTag = AuraAsc->GetInputTagFromSpec(Data);
		AbilityInfoDelegate.Broadcast(Info);
	});
	AuraAsc->ForeachAbility(BroadcastDelegate);
}
