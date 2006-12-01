/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, osch at luced dot de
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

#ifndef LABELWIDGET_H
#define LABELWIDGET_H

#include <string>

#include "types.h"
#include "GuiWidget.h"
#include "Callback.h"
#include "Slot.h"
#include "OwningPtr.h"

namespace LucED {

using std::string;

class LabelWidget : public GuiWidget
{
public:

    typedef OwningPtr<LabelWidget> Ptr;

    static LabelWidget::Ptr create(GuiWidget* parent, const string& leftText, const string& rightText = "")
    {
        return LabelWidget::Ptr(new LabelWidget(parent, leftText, rightText));
    }

    virtual ProcessingResult processEvent(const XEvent *event);
    virtual void setPosition(Position newPosition);
    virtual Measures getDesiredMeasures();

    void setLayoutHeight(int height, VerticalAdjustment::Type adjust);

    virtual FocusType getFocusType() { return fakeFocusFlag ? NORMAL_FOCUS : NO_FOCUS; }
    virtual bool isFocusable()       { return fakeFocusFlag ? true : false; }
    
    void setFakeFocus(bool fakeFocusFlag) {
        this->fakeFocusFlag = fakeFocusFlag;
    }
    
private:

    LabelWidget(GuiWidget* parent, const string& leftText, const string& rightText);

    void draw();
    
    Position position;
    string leftText;
    string rightText;
    VerticalAdjustment::Type adjustment;
    int layoutHeight;
    bool fakeFocusFlag;
};


} // namespace LucED


#endif // LABELWIDGET_H
