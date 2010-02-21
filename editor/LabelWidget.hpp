/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2009 Oliver Schmidt, oliver at luced dot de
//
//   This program is free software; you can redistribute it and/or modify it
//   under the terms of the GNU General Public License Version 2 as published
//   by the Free Software Foundation in June 1991.
//
//   This program is distributed in the hope that it will be useful, but WITHOUT
//   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//   more details.
//
//   You should have received a copy of the GNU General Public License along with 
//   this program; if not, write to the Free Software Foundation, Inc., 
//   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
/////////////////////////////////////////////////////////////////////////////////////

#ifndef LABEL_WIDGET_HPP
#define LABEL_WIDGET_HPP

#include "String.hpp"

#include "types.hpp"
#include "GuiWidget.hpp"
#include "Callback.hpp"
#include "OwningPtr.hpp"
#include "NonFocusableWidget.hpp"

namespace LucED
{

class LabelWidget : public NonFocusableWidget
{
public:

    typedef OwningPtr<LabelWidget> Ptr;

    static Ptr create(const String& labelText)
    {
        return Ptr(new LabelWidget(labelText));
    }

    void setDesiredMeasures(GuiElement::Measures m);

    virtual GuiElement::Measures internalGetDesiredMeasures();
    GuiElement::Measures getOwnDesiredMeasures();

    void setLayoutHeight(int height, VerticalAdjustment::Type adjust);

    void setMiddleMouseButtonCallback(Callback<>::Ptr middleMouseButtonCallback) {
        this->middleMouseButtonCallback = middleMouseButtonCallback;
    }
    
private:
    virtual void processGuiWidgetCreatedEvent();

private: // GuiWidget::EventListener interface implementation

    virtual GuiWidget::ProcessingResult processGuiWidgetEvent(const XEvent* event);

private:
    explicit LabelWidget(const String& labelText);

    void draw();
    
    Char2bArray labelText;
    VerticalAdjustment::Type adjustment;
    int layoutHeight;
    bool hasForcedMeasuresFlag;
    GuiElement::Measures forcedMeasures;
    Callback<>::Ptr middleMouseButtonCallback;
};

} // namespace LucED

#endif // LABEL_WIDGET_HPP
