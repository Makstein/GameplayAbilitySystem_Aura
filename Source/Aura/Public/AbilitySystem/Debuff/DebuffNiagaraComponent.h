// Copyright Mercury Massif

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NiagaraComponent.h"
#include "DebuffNiagaraComponent.generated.h"


UCLASS(ClassGroup=(Debuff), meta=(BlueprintSpawnableComponent))
class AURA_API UDebuffNiagaraComponent : public UNiagaraComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UDebuffNiagaraComponent();

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag DebuffTag;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void DebuffTagChanged(FGameplayTag CallbackTag, int32 NewCount);

	UFUNCTION()
	void OnOwnerDead(AActor* DeadActor);
	
};
