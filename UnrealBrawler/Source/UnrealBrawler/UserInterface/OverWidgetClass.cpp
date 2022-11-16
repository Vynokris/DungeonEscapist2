


#include "OverWidgetClass.h"

bool UOverWidget::Initialize()
{
    if(!Super::Initialize()) return false;

    this->GetRestartButton()->SetText("Restart Game");
    this->GetQuitButton()->SetText("Quit Game");
    
    return true;
}

void UOverWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UOverWidget::NativeDestruct()
{
    Super::NativeDestruct();
}

UButtonWidget* UOverWidget::GetRestartButton() const
{
    if(IsValid(this->RestartButtonWidget)) return CastChecked<UButtonWidget>(this->RestartButtonWidget);
    return nullptr;
}

UButtonWidget* UOverWidget::GetQuitButton() const
{
    if(IsValid(this->QuitButtonWidget)) return CastChecked<UButtonWidget>(this->QuitButtonWidget);
    return nullptr;
}