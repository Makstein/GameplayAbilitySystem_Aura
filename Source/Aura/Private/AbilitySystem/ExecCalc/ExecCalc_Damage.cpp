// Copyright Mercury Massif


#include "AbilitySystem/ExecCalc/ExecCalc_Damage.h"

#include "AbilitySystemComponent.h"
#include "AuraAbilityTypes.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Interaction/CombatInterface.h"

struct AuraDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);
	DECLARE_ATTRIBUTE_CAPTUREDEF(BlockChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArmorPenetration);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalDamage);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalResistance);

	DECLARE_ATTRIBUTE_CAPTUREDEF(FireResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(LightningResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArcaneResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(PhysicalResistance);

	TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition> TagsToCaptureDefs;

	AuraDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, Armor, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, BlockChance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ArmorPenetration, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalChance, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalDamage, Source, false);

		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, FireResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, LightningResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ArcaneResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, PhysicalResistance, Target, false);

		const FAuraGameplayTags& Tags = FAuraGameplayTags::Get();

		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_Armor, ArmorDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_BlockChance, BlockChanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalResistance, CriticalResistanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_ArmorPenetration, ArmorPenetrationDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalChance, CriticalChanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalDamage, CriticalDamageDef);

		TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Fire, FireResistanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Lightning, LightningResistanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Arcane, ArcaneResistanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Physical, PhysicalResistanceDef);
	}
};

static const AuraDamageStatics& DamageStatics()
{
	static AuraDamageStatics DStatics;
	return DStatics;
}

UExecCalc_Damage::UExecCalc_Damage()
{
	RelevantAttributesToCapture.Add(DamageStatics().ArmorDef);
	RelevantAttributesToCapture.Add(DamageStatics().BlockChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().ArmorPenetrationDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalDamageDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalResistanceDef);

	RelevantAttributesToCapture.Add(DamageStatics().FireResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().LightningResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().ArcaneResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().PhysicalResistanceDef);
}

void UExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
                                              FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const auto SourceAsc = ExecutionParams.GetSourceAbilitySystemComponent();
	const auto TargetAsc = ExecutionParams.GetTargetAbilitySystemComponent();

	auto SourceAvatar = SourceAsc->GetAvatarActor();
	auto TargetAvatar = TargetAsc->GetAvatarActor();

	int32 SourceCharacterLevel = 1.f;
	if (SourceAvatar->Implements<UCombatInterface>())
	{
		SourceCharacterLevel = ICombatInterface::Execute_GetPlayerLevel(SourceAvatar);
	}
	int32 TargetCharacterLevel = 1.f;
	if (TargetAvatar->Implements<UCombatInterface>())
	{
		TargetCharacterLevel = ICombatInterface::Execute_GetPlayerLevel(TargetAvatar);
	}

	const auto Spec = ExecutionParams.GetOwningSpec();

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	FAggregatorEvaluateParameters EvaluateParams;
	EvaluateParams.SourceTags = SourceTags;
	EvaluateParams.TargetTags = TargetTags;

	const UCharacterClassInfo* CharacterClassInfo = UAuraAbilitySystemLibrary::GetCharacterClassInfo(SourceAvatar);

	// Get damage set by caller magnitude
	float Damage = 0.f;
	for (const auto& Pair : FAuraGameplayTags::Get().DamageTypesToResistance)
	{
		const FGameplayTag DamageTypeTag = Pair.Key;
		const FGameplayTag ResistanceTag = Pair.Value;

		checkf(AuraDamageStatics().TagsToCaptureDefs.Contains(ResistanceTag),
		       TEXT("TagToCaptureDefs doesn't contains Tag: [%s] in ExecCalc_Damage"), *ResistanceTag.ToString());
		const FGameplayEffectAttributeCaptureDefinition CaptureDefinition = AuraDamageStatics().TagsToCaptureDefs[
			ResistanceTag];

		float DamageTypeValue = Spec.GetSetByCallerMagnitude(Pair.Key, false);

		float Resistance = 0.f;
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CaptureDefinition, EvaluateParams, Resistance);
		Resistance = FMath::Clamp(Resistance, 0.f, 100.f);

		DamageTypeValue *= (100.f - Resistance) / 100.f;

		Damage += DamageTypeValue;
	}

	/// Capture BlockChance on target, and determine if there was a successful block
	/// If block, halve the damage
	float TargetBlockChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BlockChanceDef, EvaluateParams,
	                                                           TargetBlockChance);
	TargetBlockChance = FMath::Max<float>(0.f, TargetBlockChance);
	const bool bBlocked = FMath::RandRange(1, 100) < TargetBlockChance;
	if (bBlocked) Damage *= 0.5f;

	auto EffectContextHandle = Spec.GetContext();
	UAuraAbilitySystemLibrary::SetIsBlockedHit(EffectContextHandle, bBlocked);

	// Critical increase a percentage of damage
	float CriticalChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalChanceDef, EvaluateParams,
	                                                           CriticalChance);
	CriticalChance = FMath::Max<float>(0.f, CriticalChance);

	float CriticalResistance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalResistanceDef, EvaluateParams,
	                                                           CriticalResistance);
	CriticalResistance = FMath::Max<float>(0.f, CriticalResistance);
	const FRealCurve* CriticalResistanceCurve = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(
		FName("CriticalResistance"), FString());
	const float CriticalResistanceCoefficient = CriticalResistanceCurve->Eval(TargetCharacterLevel);

	float CriticalDamage = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalDamageDef, EvaluateParams,
	                                                           CriticalDamage);
	CriticalDamage = FMath::Max<float>(0.f, CriticalDamage);

	const float EffectiveCriticalChance = CriticalChance - CriticalResistance * CriticalResistanceCoefficient;
	const bool bCritical = FMath::RandRange(1, 100) < EffectiveCriticalChance;
	if (bCritical) Damage = Damage * 2.f + CriticalDamage;
	UAuraAbilitySystemLibrary::SetIsCriticalHit(EffectContextHandle, bCritical);

	// Target's Armor and ArmorPenetration also affect damage
	float TargetArmor = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvaluateParams, TargetArmor);
	TargetArmor = FMath::Max<float>(0.f, TargetArmor);

	float SourceArmorPenetration = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorPenetrationDef, EvaluateParams,
	                                                           SourceArmorPenetration);
	SourceArmorPenetration = FMath::Max<float>(0.f, SourceArmorPenetration);

	// Armor Penetration ignores a percentage of target's armor
	const FRealCurve* ArmorPenetrationCurve = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(
		FName("ArmorPenetration"), FString());
	const float ArmorPenetrationCoefficient = ArmorPenetrationCurve->Eval(SourceCharacterLevel);
	const float EffectiveArmor = TargetArmor * (100.f - SourceArmorPenetration * ArmorPenetrationCoefficient) / 100.f;

	// Armor ignores a percentage of incoming Damage
	const FRealCurve* EffectiveArmorCurve = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(
		FName("EffectiveArmor"), FString());
	const float EffectiveArmorCoefficient = EffectiveArmorCurve->Eval(TargetCharacterLevel);
	Damage *= (100.f - EffectiveArmor * EffectiveArmorCoefficient) / 100.f;

	const FGameplayModifierEvaluatedData EvaluatedData(UAuraAttributeSet::GetIncomingDamageAttribute(),
	                                                   EGameplayModOp::Additive, Damage);

	OutExecutionOutput.AddOutputModifier(EvaluatedData);
}
