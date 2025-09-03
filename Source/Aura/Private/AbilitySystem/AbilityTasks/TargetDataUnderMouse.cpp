// Copyright Mercury Massif


#include "AbilitySystem/AbilityTasks/TargetDataUnderMouse.h"

#include "AbilitySystemComponent.h"
#include "Aura/Aura.h"
#include "Kismet/GameplayStatics.h"
#include "Player/AuraPlayerController.h"

UTargetDataUnderMouse* UTargetDataUnderMouse::CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility)
{
	UTargetDataUnderMouse* MyObj = NewAbilityTask<UTargetDataUnderMouse>(OwningAbility);
	return MyObj;
}

/*
 * Trigger when this task running
 * will be replicated to server, so if we are on client, we need to send the data to server too
 */
void UTargetDataUnderMouse::Activate()
{
	if (Ability->GetActorInfo().IsLocallyControlled())
	{
		SendMouseCursorData();
	}
	else
	{
		// Server, listen and wait for client's target data
		const FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
		const FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();

		AbilitySystemComponent.Get()->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).AddLambda(
			[SpecHandle, ActivationPredictionKey, this](const FGameplayAbilityTargetDataHandle& DataHandle,
			                                            FGameplayTag ActivationTag)
			{
				AbilitySystemComponent->ConsumeClientReplicatedTargetData(SpecHandle, ActivationPredictionKey);
				if (ShouldBroadcastAbilityTaskDelegates())
				{
					ValidData.Broadcast(DataHandle);
				}
			});

		const bool bCalledDelegate = AbilitySystemComponent.Get()->CallReplicatedTargetDataDelegatesIfSet(
			SpecHandle, ActivationPredictionKey);
		if (!bCalledDelegate)
		{
			SetWaitingOnRemotePlayerData();
		}
	}
}

void UTargetDataUnderMouse::SendMouseCursorData()
{
	FScopedPredictionWindow ScopedPredictionWindow(AbilitySystemComponent.Get());

	const APlayerController* PC = Ability->GetActorInfo().PlayerController.Get();
	FHitResult HitResult;
	PC->GetHitResultUnderCursor(ECC_Target, false, HitResult);

	FGameplayAbilityTargetDataHandle DataHandle;
	FGameplayAbilityTargetData_SingleTargetHit* Data = new FGameplayAbilityTargetData_SingleTargetHit();
	Data->HitResult = HitResult;
	DataHandle.Add(Data);

	AbilitySystemComponent->ServerSetReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey(),
	                                                      DataHandle, FGameplayTag(),
	                                                      AbilitySystemComponent->ScopedPredictionKey);

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(DataHandle);
	}
}
