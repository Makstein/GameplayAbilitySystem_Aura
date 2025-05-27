// Copyright Mercury Massif


#include "AbilitySystem/Data/LevelUpInfo.h"

int32 ULevelUpInfo::FindLevelForXP(const int32 XP) const
{
	for (auto i = 1; i < LevelUpInformation.Num(); i++)
	{
		if (XP < LevelUpInformation[i].LevelUpRequirement)
		{
			return i - 1;
		}
	}
	return 1;
}
