// Copyright Mercury Massif


#include "UI/Widget/AuraUserWidget.h"

void UAuraUserWidget::SetWidgetController(UAuraWidgetController* Controller)
{
	WidgetController = Controller;
	WidgetControllerSet();
}
