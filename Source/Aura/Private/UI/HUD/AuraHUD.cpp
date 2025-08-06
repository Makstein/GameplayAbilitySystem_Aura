// Copyright Mercury Massif


#include "UI/HUD/AuraHUD.h"
#include "UI/Widget/AuraUserWidget.h"
#include "Blueprint/UserWidget.h"
#include "UI/WidgetController/OverlayWidgetController.h"

UOverlayWidgetController* AAuraHUD::GetOverlayWidgetController(const FWidgetControllerParams& Params)
{
	if (OverlayWidgetController != nullptr) return OverlayWidgetController;
	OverlayWidgetController = NewObject<UOverlayWidgetController>(this, OverlayWidgetControllerClass);
	OverlayWidgetController->SetWidgetControllerParams(Params);
	OverlayWidgetController->BindCallbacksToDependencies();

	return OverlayWidgetController;
}

UAttributeMenuController* AAuraHUD::GetAttributeMenuController(const FWidgetControllerParams& Params)
{
	if (AttributeMenuController != nullptr) return AttributeMenuController;

	AttributeMenuController = NewObject<UAttributeMenuController>(this, AttributeMenuControllerClass);
	AttributeMenuController->SetWidgetControllerParams(Params);
	AttributeMenuController->BindCallbacksToDependencies();
	
	return AttributeMenuController;
}

USpellMenuWidgetController* AAuraHUD::GetSpellMenuWidgetController(const FWidgetControllerParams& Params)
{
	if (SpellMenuController == nullptr)
	{
		SpellMenuController = NewObject<USpellMenuWidgetController>(this, SpellMenuControllerClass);
		SpellMenuController->SetWidgetControllerParams(Params);
		SpellMenuController->BindCallbacksToDependencies();
	}

	return SpellMenuController;
}

void AAuraHUD::InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	checkf(OverlayWidgetClass, TEXT("OverlayWidgetClass undefined, please fill out BP_AuraHUD!"));
	checkf(OverlayWidgetControllerClass, TEXT("OverlayWidgetControllerClass undefined, please fill out BP_AuraHUD!"));

	UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), OverlayWidgetClass);
	OverlayWidget = Cast<UAuraUserWidget>(Widget);

	const FWidgetControllerParams Params(PC, PS, ASC, AS);
	UOverlayWidgetController* Controller = GetOverlayWidgetController(Params);

	OverlayWidget->SetWidgetController(Controller);
	Controller->BroadcastInitialParams();
	
	Widget->AddToViewport();
}
