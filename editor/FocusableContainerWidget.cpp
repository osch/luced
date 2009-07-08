#include "FocusableContainerWidget.hpp"

using namespace LucED;

void FocusableContainerWidget::requestHotKeyRegistrationFor(const KeyMapping::Id& id, RawPtr<FocusableElement> w)
{
    focusManagerForChilds->requestHotKeyRegistrationFor(id, w);
}


void FocusableContainerWidget::requestRemovalOfHotKeyRegistrationFor(const KeyMapping::Id& id, RawPtr<FocusableElement> w)
{
    focusManagerForChilds->requestRemovalOfHotKeyRegistrationFor(id, w);
}


void FocusableContainerWidget::requestFocusFor(RawPtr<FocusableElement> w)
{
    focusManagerForThis->requestFocusFor(this);
}


void FocusableContainerWidget::reportMouseClickFrom(RawPtr<FocusableElement> w)
{
    focusManagerForThis->reportMouseClickFrom(this);
}

void FocusableContainerWidget::adopt(RawPtr<GuiElement>   parentElement,
                                     RawPtr<GuiWidget>    parentWidget,
                                     RawPtr<FocusManager> focusManagerForThis,
                                     RawPtr<FocusManager> focusManagerForChilds)
{
    this->focusManagerForThis = focusManagerForThis;
    this->focusManagerForChilds = focusManagerForChilds;
    
    FocusableWidget::adopt(parentElement,
                           parentWidget,
                           focusManagerForThis,
                           this); // focusManagerForChilds
}

