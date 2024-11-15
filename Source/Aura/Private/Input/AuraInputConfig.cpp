// Copyright Mercury Massif


#include "Input/AuraInputConfig.h"

const UInputAction* UAuraInputConfig::FindAbilityInputActionForTag(const FGameplayTag& InputTag,
                                                                   bool bLogNotFound) const
{
	for (const auto& [InputAction, CurInputTag] : AbilityInputActions)
	{
		if (InputAction && CurInputTag.MatchesTagExact(InputTag))
		{
			return InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("Action not fount for tag: %s"), *InputTag.ToString())
	}

	return nullptr;
}
