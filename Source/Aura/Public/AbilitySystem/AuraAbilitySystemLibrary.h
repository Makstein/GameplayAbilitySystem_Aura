// Copyright Mercury Massif

#pragma once

#include "CoreMinimal.h"
#include "Data/CharacterClassInfo.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UI/HUD/AuraHUD.h"
#include "UI/WidgetController/AttributeMenuController.h"
#include "UI/WidgetController/OverlayWidgetController.h"
#include "UI/WidgetController/SpellMenuWidgetController.h"
#include "AuraAbilitySystemLibrary.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UAuraAbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, category = "AuraAbilitySystemLibrary|WidgetController", meta = (DefaultToSelf = "WorldContext"))
	static bool MakeWidgetControllerParams(const UObject* WorldContext, FWidgetControllerParams& OutWcParams, AAuraHUD*& AuraHUD);
	
	UFUNCTION(BlueprintPure, category = "AuraAbilitySystemLibrary|WidgetController", meta = (DefaultToSelf = "WorldContext"))
	static UOverlayWidgetController* GetOverlayWidgetController(const UObject* WorldContext);

	UFUNCTION(BlueprintPure, category = "AuraAbilitySystemLibrary|WidgetController", meta = (DefaultToSelf = "WorldContext"))
	static UAttributeMenuController* GetAttributeMenuController(const UObject* WorldContext);

	UFUNCTION(BlueprintPure, category = "AuraAbilitySystemLibrary|WidgetController", meta = (DefaultToSelf = "WorldContext"))
	static USpellMenuWidgetController* GetSpellMenuController(const UObject* WorldContext);

	UFUNCTION(BlueprintCallable, category = "AuraAbilitySystemLibrary|CharacterClassDefaults")
	static void InitializeDefaultAttributes(const UObject* WorldContext, ECharacterClass CharacterClass, float Level,
	                                        UAbilitySystemComponent* Asc);

	UFUNCTION(BlueprintCallable, category = "AuraAbilitySystemLibrary|CharacterClassDefaults")
	static void GiveStartupAbilities(const UObject* WorldContext, UAbilitySystemComponent* Asc,
	                                 ECharacterClass CharacterClass);

	UFUNCTION(BlueprintCallable, category = "AuraAbilitySystemLibrary|CharacterClassDefaults")
	static UCharacterClassInfo* GetCharacterClassInfo(const UObject* WorldContext);

	UFUNCTION(BlueprintCallable, category = "AuraAbilitySystemLibrary|CharacterClassDefaults")
	static UAbilityInfo* GetAbilityInfo(const UObject* WorldContext);

	UFUNCTION(BlueprintPure, category = "AuraAbilitySystemLibrary|GameplayEffects")
	static bool IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle);

	UFUNCTION(BlueprintPure, category = "AuraAbilitySystemLibrary|GameplayEffects")
	static bool IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle);

	UFUNCTION(BlueprintCallable, category = "AuraAbilitySystemLibrary|GameplayEffects")
	static void SetIsBlockedHit(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool IsBlocked);

	UFUNCTION(BlueprintCallable, category = "AuraAbilitySystemLibrary|GameplayEffects")
	static void SetIsCriticalHit(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool IsCritical);

	UFUNCTION(BlueprintCallable, category = "AuraAbilitySystemLibrary|GameplayMechanics")
	static void GetLivePlayerWithinRadius(const UObject* WorldContext, TArray<AActor*>& OutOverlappingActors,
	                                      const TArray<AActor*>& ActorsToIgnore, float Radius,
	                                      const FVector& SphereOrigin);

	UFUNCTION(BlueprintPure, category = "AuraAbilitySystemLibrary|GameplayMechanics")
	static bool IsFriend(const AActor* FirstActor, const AActor* SecondActor);

	static int32 GetXPRewardByClassAndLevel(const UObject* WorldContextObject, ECharacterClass CharacterClass, int32 Level);
};
