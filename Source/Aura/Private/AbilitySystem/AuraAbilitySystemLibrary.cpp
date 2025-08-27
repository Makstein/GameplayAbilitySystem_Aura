// Copyright Mercury Massif


#include "AbilitySystem/AuraAbilitySystemLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AuraAbilityTypes.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Engine/OverlapResult.h"
#include "Game/AuraGameModeBase.h"
#include "Interaction/CombatInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Player/AuraPlayerState.h"
#include "UI/HUD/AuraHUD.h"

bool UAuraAbilitySystemLibrary::MakeWidgetControllerParams(const UObject* WorldContext, FWidgetControllerParams& OutWcParams, AAuraHUD*& AuraHUD)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContext, 0))
	{
		AuraHUD = Cast<AAuraHUD>(PC->GetHUD());
		if (AuraHUD)
		{
			AAuraPlayerState* PS = PC->GetPlayerState<AAuraPlayerState>();
			UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			UAttributeSet* AS = PS->GetAttributeSet();
			OutWcParams.AttributeSet = AS;
			OutWcParams.AbilitySystemComponent = ASC;
			OutWcParams.PlayerController = PC;
			OutWcParams.PlayerState = PS;
			return true;
		}
	}

	return false;
}

UOverlayWidgetController* UAuraAbilitySystemLibrary::GetOverlayWidgetController(const UObject* WorldContext)
{
	FWidgetControllerParams Params;
	AAuraHUD* AuraHUD;
	if (MakeWidgetControllerParams(WorldContext, Params, AuraHUD))
	{
		return AuraHUD->GetOverlayWidgetController(Params);
	}

	return nullptr;
}

UAttributeMenuController* UAuraAbilitySystemLibrary::GetAttributeMenuController(const UObject* WorldContext)
{
	FWidgetControllerParams Params;
	AAuraHUD* AuraHUD;
	if (MakeWidgetControllerParams(WorldContext, Params, AuraHUD))
	{
		return AuraHUD->GetAttributeMenuController(Params);
	}

	return nullptr;
}

USpellMenuWidgetController* UAuraAbilitySystemLibrary::GetSpellMenuController(const UObject* WorldContext)
{
	FWidgetControllerParams Params;
	AAuraHUD* AuraHUD;
	if (MakeWidgetControllerParams(WorldContext, Params, AuraHUD))
	{
		return AuraHUD->GetSpellMenuWidgetController(Params);
	}

	return nullptr;
}

void UAuraAbilitySystemLibrary::InitializeDefaultAttributes(const UObject* WorldContext, ECharacterClass CharacterClass,
                                                            float Level, UAbilitySystemComponent* Asc)
{
	const auto CharacterClassInfo = GetCharacterClassInfo(WorldContext);
	const auto ClassDefaultInfo = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);

	const auto AvatarActor = Asc->GetAvatarActor();

	auto PrimaryAttributeContextHandle = Asc->MakeEffectContext();
	PrimaryAttributeContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle PrimaryAttributesSpecHandle = Asc->MakeOutgoingSpec(
		ClassDefaultInfo.PrimaryAttributes, Level, PrimaryAttributeContextHandle);
	Asc->ApplyGameplayEffectSpecToSelf(*PrimaryAttributesSpecHandle.Data.Get());

	auto SecondaryAttributeContextHandle = Asc->MakeEffectContext();
	SecondaryAttributeContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle SecondaryAttributesSpecHandle = Asc->MakeOutgoingSpec(
		CharacterClassInfo->SecondaryAttributes, Level, SecondaryAttributeContextHandle);
	Asc->ApplyGameplayEffectSpecToSelf(*SecondaryAttributesSpecHandle.Data.Get());

	auto VitalAttributeContextHandle = Asc->MakeEffectContext();
	VitalAttributeContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle VitalAttributesSpecHandle = Asc->MakeOutgoingSpec(
		CharacterClassInfo->VitalAttributes, Level, VitalAttributeContextHandle);
	Asc->ApplyGameplayEffectSpecToSelf(*VitalAttributesSpecHandle.Data.Get());
}

void UAuraAbilitySystemLibrary::GiveStartupAbilities(const UObject* WorldContext, UAbilitySystemComponent* Asc,
                                                     ECharacterClass CharacterClass)
{
	const auto CharacterClassInfo = GetCharacterClassInfo(WorldContext);
	if (CharacterClassInfo == nullptr) return;
	for (const auto AbilityClass : CharacterClassInfo->CommonAbilities)
	{
		auto AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		Asc->GiveAbility(AbilitySpec);
	}
	const FCharacterClassDefaultInfo& DefaultInfo = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);
	for (TSubclassOf<UGameplayAbility> AbilityClass : DefaultInfo.StartupAbilities)
	{
		if (Asc->GetAvatarActor()->Implements<UCombatInterface>())
		{
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, ICombatInterface::Execute_GetPlayerLevel(Asc->GetAvatarActor()));
			Asc->GiveAbility(AbilitySpec);
		}
	}
}

UCharacterClassInfo* UAuraAbilitySystemLibrary::GetCharacterClassInfo(const UObject* WorldContext)
{
	const auto AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContext));
	if (AuraGameMode == nullptr) return nullptr;

	return AuraGameMode->CharacterClassInfo;
}

UAbilityInfo* UAuraAbilitySystemLibrary::GetAbilityInfo(const UObject* WorldContext)
{
	const auto AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContext));
	if (AuraGameMode == nullptr) return nullptr;

	return AuraGameMode->AbilityInfo.Get();
}

bool UAuraAbilitySystemLibrary::IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const auto AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return AuraEffectContext->IsBlockedHit();
	}
	return false;
}

bool UAuraAbilitySystemLibrary::IsSuccessfulDebuff(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const auto AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return AuraEffectContext->IsSuccessfulDebuff();
	}
	return false;
}

float UAuraAbilitySystemLibrary::GetDebuffDamage(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const auto AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return AuraEffectContext->GetDebuffDamage();
	}
	return 0.f;
}

float UAuraAbilitySystemLibrary::GetDebuffDuration(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const auto AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return AuraEffectContext->GetDebuffDuration();
	}
	return 0.f;
}

float UAuraAbilitySystemLibrary::GetDebuffFrequency(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const auto AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return AuraEffectContext->GetDebuffFrequency();
	}
	return 0.f;
}

FGameplayTag UAuraAbilitySystemLibrary::GetDamageType(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const auto AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		if (AuraEffectContext->GetDamageType().IsValid())
		{
			return *AuraEffectContext->GetDamageType();
		}
	}
	return FGameplayTag();
}

FVector UAuraAbilitySystemLibrary::GetDeathImpulse(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const auto AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return AuraEffectContext->GetDeathImpulse();
	}
	return FVector::ZeroVector;
}

bool UAuraAbilitySystemLibrary::IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const auto AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return AuraEffectContext->IsCriticalHit();
	}
	return false;
}

void UAuraAbilitySystemLibrary::SetIsBlockedHit(FGameplayEffectContextHandle& EffectContextHandle, bool IsBlocked)
{
	if (auto AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AuraEffectContext->SetIsBlockedHit(IsBlocked);
	}
}

void UAuraAbilitySystemLibrary::SetIsSuccessfulDebuff(FGameplayEffectContextHandle& EffectContextHandle,
	bool bInSuccessfulDebuff)
{
	if (auto AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AuraEffectContext->SetIsSuccessfulDebuff(bInSuccessfulDebuff);
	}
}

void UAuraAbilitySystemLibrary::SetDebuffDamage(FGameplayEffectContextHandle& EffectContextHandle, float InDamage)
{
	if (auto AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AuraEffectContext->SetDebuffDamage(InDamage);
	}
}

void UAuraAbilitySystemLibrary::SetDebuffDuration(FGameplayEffectContextHandle& EffectContextHandle, float InDuration)
{
	if (auto AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AuraEffectContext->SetDebuffDuration(InDuration);
	}
}

void UAuraAbilitySystemLibrary::SetDebuffFrequency(FGameplayEffectContextHandle& EffectContextHandle, float InFrequency)
{
	if (auto AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AuraEffectContext->SetDebuffFrequency(InFrequency);
	}
}

void UAuraAbilitySystemLibrary::SetDamageType(FGameplayEffectContextHandle& EffectContextHandle,
	const FGameplayTag& InDamageType)
{
	TSharedPtr<FGameplayTag> DamageType = MakeShareable(new FGameplayTag(InDamageType));
	if (auto AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AuraEffectContext->SetDamageType(DamageType);
	}
}

void UAuraAbilitySystemLibrary::SetIsCriticalHit(FGameplayEffectContextHandle& EffectContextHandle,
                                                 bool IsCritical)
{
	if (auto AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AuraEffectContext->SetIsCriticalHit(IsCritical);
	}
}

void UAuraAbilitySystemLibrary::SetDeathImpulse(FGameplayEffectContextHandle& EffectContextHandle,
                                                const FVector& InDeathImpulse)
{
	if (auto AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AuraEffectContext->SetDeathImpulse(InDeathImpulse);
	}
}

void UAuraAbilitySystemLibrary::GetLivePlayerWithinRadius(const UObject* WorldContext,
                                                          TArray<AActor*>& OutOverlappingActors,
                                                          const TArray<AActor*>& ActorsToIgnore, float Radius,
                                                          const FVector& SphereOrigin)
{
	FCollisionQueryParams SphereParams;
	SphereParams.AddIgnoredActors(ActorsToIgnore);

	TArray<FOverlapResult> Overlaps;
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
	{
		// Get all object in radius
		World->OverlapMultiByObjectType(Overlaps, SphereOrigin, FQuat::Identity,
		                                FCollisionObjectQueryParams(
			                                FCollisionObjectQueryParams::InitType::AllDynamicObjects),
		                                FCollisionShape::MakeSphere(Radius), SphereParams);

		for (auto& OverlapResult : Overlaps)
		{
			if (OverlapResult.GetActor()->Implements<UCombatInterface>()
				&& !ICombatInterface::Execute_IsDead(OverlapResult.GetActor()))
			{
				OutOverlappingActors.AddUnique(OverlapResult.GetActor());
			}
		}
	}
}

bool UAuraAbilitySystemLibrary::IsFriend(const AActor* FirstActor, const AActor* SecondActor)
{
	if (FirstActor == nullptr || SecondActor == nullptr) return false;

	const bool bFirstIsPlayer = FirstActor->ActorHasTag(FName("Player"));
	const bool bSecondIsPlayer = SecondActor->ActorHasTag(FName("Player"));
	const bool bFirstIsEnemy = FirstActor->ActorHasTag(FName("Enemy"));
	const bool bSecondIsEnemy = SecondActor->ActorHasTag(FName("Enemy"));

	if (bFirstIsPlayer && bSecondIsPlayer) return true;
	if (bFirstIsEnemy && bSecondIsEnemy) return true;

	return false;
}

FGameplayEffectContextHandle UAuraAbilitySystemLibrary::ApplyDamageEffect(const FDamageEffectParams& DamageEffectParams)
{
	const auto GameplayTags = FAuraGameplayTags::Get();
	const AActor* SourceAvatarActor = DamageEffectParams.SourceAbilitySystemComponent->GetAvatarActor();
	FGameplayEffectContextHandle EffectContextHandle = DamageEffectParams.SourceAbilitySystemComponent->MakeEffectContext();
	EffectContextHandle.AddSourceObject(SourceAvatarActor);
	SetDeathImpulse(EffectContextHandle, DamageEffectParams.DeathImpulse);

	const FGameplayEffectSpecHandle EffectSpecHandle = DamageEffectParams.SourceAbilitySystemComponent->MakeOutgoingSpec(DamageEffectParams.DamageGameplayEffectClass, DamageEffectParams.AbilityLevel, EffectContextHandle);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, DamageEffectParams.DamageType, DamageEffectParams.BaseDamage);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, GameplayTags.Debuff_Chance, DamageEffectParams.DebuffChange);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, GameplayTags.Debuff_Damage, DamageEffectParams.DebuffDamage);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, GameplayTags.Debuff_Frequency, DamageEffectParams.DebuffFrequency);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(EffectSpecHandle, GameplayTags.Debuff_Duration, DamageEffectParams.DebuffDuration);

	DamageEffectParams.TargetAbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data);

	return EffectContextHandle;
}

int32 UAuraAbilitySystemLibrary::GetXPRewardByClassAndLevel(const UObject* WorldContextObject,
                                                            ECharacterClass CharacterClass, int32 Level)
{
	const auto CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (CharacterClassInfo == nullptr) return 0;

	const auto Info = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);
	const auto XPReward = Info.XPReward.GetValueAtLevel(Level);

	return static_cast<int32>(XPReward);
}
