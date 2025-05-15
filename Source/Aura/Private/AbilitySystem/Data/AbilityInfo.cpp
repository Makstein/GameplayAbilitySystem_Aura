// Copyright Mercury Massif


#include "AbilitySystem/Data/AbilityInfo.h"

#include "Aura/AuraLogChannel.h"

FAuraAbilityInfo UAbilityInfo::FindAbilityInfoForTag(const FGameplayTag AbilityTag, bool bLogNotFound) const
{
	for (const auto Info : AbilityInformation)
	{
		if (Info.AbilityTag == AbilityTag)
		{
			return Info;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogAura, Error, TEXT("Can't find info for tag [%s]"), *AbilityTag.ToString())
	}
	return FAuraAbilityInfo();
}
