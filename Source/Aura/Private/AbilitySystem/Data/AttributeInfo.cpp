// Copyright Mercury Massif


#include "AbilitySystem/Data/AttributeInfo.h"

FAuraAttributeInfo UAttributeInfo::FindAttributeTag(const FGameplayTag& AttributeTag, bool bLogNotFound)
{
	for (const FAuraAttributeInfo Info : AttributeInformation)
	{
		if (Info.AttributeTag.MatchesTagExact(AttributeTag))
		{
			return Info;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't find info for Attribute Tag [%s] on AttributeInfo [%s]"),
		       *AttributeTag.ToString(), *GetNameSafe(this));
	}

	return FAuraAttributeInfo();
}
