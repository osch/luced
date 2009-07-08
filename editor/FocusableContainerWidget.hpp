#ifndef FOCUSABLE_CONTAINER_WIDGET_HPP
#define FOCUSABLE_CONTAINER_WIDGET_HPP

#include "FocusableWidget.hpp"
#include "FocusManager.hpp"

namespace LucED
{

class FocusableContainerWidget : public FocusableWidget,
                                 public FocusManager
{
public:
    // FocusManager methods
    virtual void requestHotKeyRegistrationFor(const KeyMapping::Id& id, RawPtr<FocusableElement> w);
    virtual void requestRemovalOfHotKeyRegistrationFor(const KeyMapping::Id& id, RawPtr<FocusableElement> w);
    virtual void requestFocusFor(RawPtr<FocusableElement> w);
    virtual void reportMouseClickFrom(RawPtr<FocusableElement> w);

public:
    // FocusableWidget methods
    virtual void adopt(RawPtr<GuiElement>   parentElement,
                       RawPtr<GuiWidget>    parentWidget,
                       RawPtr<FocusManager> focusManagerForThis,
                       RawPtr<FocusManager> focusManagerForChilds);
protected:
    FocusableContainerWidget(Visibility   defaultVisibility = VISIBLE, 
                             int          borderWidth = 0)
        : FocusableWidget(defaultVisibility, borderWidth)
    {}
    
private:
    RawPtr<FocusManager> focusManagerForThis;
    RawPtr<FocusManager> focusManagerForChilds;
};

} // namespace LucED

#endif // FOCUSABLE_CONTAINER_WIDGET_HPP
