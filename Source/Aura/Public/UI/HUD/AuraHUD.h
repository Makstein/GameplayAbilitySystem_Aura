// Copyright Mercury Massif

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "UI/WidgetController/AttributeMenuController.h"
#include "UI/WidgetController/SpellMenuWidgetController.h"
#include "AuraHUD.generated.h"

class UAttributeSet;
class UAbilitySystemComponent;
struct FWidgetControllerParams;
class UOverlayWidgetController;
class UAuraUserWidget;
/**
 * 
 */
UCLASS()
class AURA_API AAuraHUD : public AHUD
{
	GENERATED_BODY()

public:
	UOverlayWidgetController* GetOverlayWidgetController(const FWidgetControllerParams& Params);
	UAttributeMenuController* GetAttributeMenuController(const FWidgetControllerParams& Params);
	USpellMenuWidgetController* GetSpellMenuWidgetController(const FWidgetControllerParams& Params);
	
	void InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS);

protected:

private:
	UPROPERTY()
	TObjectPtr<UAuraUserWidget> OverlayWidget;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UAuraUserWidget> OverlayWidgetClass;

	UPROPERTY()
	TObjectPtr<UOverlayWidgetController> OverlayWidgetController;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UOverlayWidgetController> OverlayWidgetControllerClass;

	UPROPERTY()
	TObjectPtr<UAttributeMenuController> AttributeMenuController;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UAttributeMenuController> AttributeMenuControllerClass;

	UPROPERTY()
	TObjectPtr<USpellMenuWidgetController> SpellMenuController;

	UPROPERTY(EditAnywhere)
	TSubclassOf<USpellMenuWidgetController> SpellMenuControllerClass;
};
