// Copyright Mercury Massif

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AuraEffectActor.generated.h"

class UGameplayEffect;
class USphereComponent;

UCLASS()
class AURA_API AAuraEffectActor : public AActor
{
	GENERATED_BODY()

public:
	AAuraEffectActor();

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void ApplyEffectToTarget(UPARAM(ref) AActor* TargetActor);

	UPROPERTY(EditAnywhere, Category = "Applied Effects")
	TSubclassOf<UGameplayEffect> InstanceGameplayEffectClass;
};
