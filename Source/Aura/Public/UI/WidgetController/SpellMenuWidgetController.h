// Copyright Mercury Massif

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "SpellMenuWidgetController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FSpellGlobeSelectedSignature, bool, bSpendPointsButtonEnabled,
	bool, bEquipButtonEnabled,
	FString, DescriptionString,
	FString, NextLevelDescriptionString);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWaitForEquipSelectionSignature, const FGameplayTag&, SelectedAbilityTypeTag);

struct FSelectedAbility
{
	FGameplayTag AbilityTag = FGameplayTag();
	FGameplayTag StatusTag = FGameplayTag();
};

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class AURA_API USpellMenuWidgetController : public UAuraWidgetController
{
	GENERATED_BODY()

public:
	virtual void BroadcastInitialParams() override;
	virtual void BindCallbacksToDependencies() override;

	UPROPERTY(BlueprintAssignable)
	FSpellGlobeSelectedSignature SpellGlobeSelectedDelegate;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerStateChangedSignature SpellPointChanged;

	UPROPERTY(BlueprintAssignable)
	FWaitForEquipSelectionSignature WaitForEquipSelectionDelegate;

	UPROPERTY(BlueprintAssignable)
	FWaitForEquipSelectionSignature StopWaitForEquipSelectionDelegate;

	FSelectedAbility SelectedAbility;

	UFUNCTION(BlueprintCallable)
	void SpendPointsButtonPressed();

	UFUNCTION(BlueprintCallable)
	void SpellGlobeButtonPressed(const FGameplayTag& SelectedAbilityTag, const FGameplayTag& SelectedStatusTag);

	UFUNCTION(BlueprintCallable)
	void GlobeDeselect();

	UFUNCTION(BlueprintCallable)
	void EquipButtonPressed();

private:
	static void ShouldEnableButton(const FGameplayTag& SelectedStatusTag, int32 CurrentSpellPoints, bool& bSpendPointsButtonEnabled, bool& bEquipButtonEnabled);
	bool bWaitingForEquipSelection = false;
	FGameplayTag SelectedSlot;
};
