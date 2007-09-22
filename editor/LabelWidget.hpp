/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2007 Oliver Schmidt, oliver at luced dot de
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

#ifndef LABELWIDGET_HPP
#define LABELWIDGET_HPP

#include "String.hpp"

#include "types.hpp"
#include "GuiWidget.hpp"
#include "Callback.hpp"
#include "OwningPtr.hpp"

namespace LucED {



class LabelWidget : public GuiWidget
{
public:

    typedef OwningPtr<LabelWidget> Ptr;

    static LabelWidget::Ptr create(GuiWidget* parent, const String& leftText, const String& rightText = "")
    {
        return LabelWidget::Ptr(new LabelWidget(parent, leftText, rightText));
    }

    virtual ProcessingResult processEvent(const XEvent *event);
    virtual void setPosition(Position newPosition);

    void setDesiredMeasures(Measures m);

    virtual Measures getDesiredMeasures();
    Measures getOwnDesiredMeasures();

    void setLayoutHeight(int height, VerticalAdjustment::Type adjust);

    virtual FocusType getFocusType() { return fakeFocusFlag ? NORMAL_FOCUS : NO_FOCUS; }
    virtual bool isFocusable()       { return fakeFocusFlag ? true : false; }
    
    void setFakeFocus(bool fakeFocusFlag) {
        this->fakeFocusFlag = fakeFocusFlag;
    }
    
    void setMiddleMouseButtonCallback(Callback<>::Ptr middleMouseButtonCallback) {
        this->middleMouseButtonCallback = middleMouseButtonCallback;
    }
    
private:

    LabelWidget(GuiWidget* parent, const String& leftText, const String& rightText);

    void draw();
    
    Position position;
    String leftText;
    String rightText;
    VerticalAdjustment::Type adjustment;
    int layoutHeight;
    bool fakeFocusFlag;
    bool hasForcedMeasuresFlag;
    Measures forcedMeasures;
    Callback<>::Ptr middleMouseButtonCallback;
};


} // namespace LucED


#endif // LABELWIDGET_HPP
