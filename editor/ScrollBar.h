/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, oliver at luced dot de
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

#ifndef SCROLLBAR_H
#define SCROLLBAR_H

#include "GuiWidget.h"
#include "Callback.h"
#include "Slot.h"
#include "types.h"
#include "OwningPtr.h"

namespace LucED {

class ScrollBar : public GuiWidget
{
public:

    typedef OwningPtr<ScrollBar> Ptr;

    static ScrollBar::Ptr create(GuiWidget* parent, Orientation::Type orientation)
    {
        return ScrollBar::Ptr(new ScrollBar(parent, orientation));
    }

    void setChangedValueCallback(const Callback1<long>& callback) {
        this->changedValueCallback = callback;
    }
    
    void setScrollStepCallback(const Callback1<ScrollStep::Type>& callback) {
        this->scrollStepCallback = callback;
    }

    virtual ProcessingResult processEvent(const XEvent *event);
    virtual void setPosition(Position newPosition);
    virtual Measures getDesiredMeasures();

    void setValue(long value);
    void setValueRange(long totalValue, long heightValue, long value);

    Slot1<long> slotForSetValue;
    Slot3<long,long,long> slotForSetValueRange;

    
private:

    enum HilitedPart { TOP_ARROW, SCROLLER, BOTTOM_ARROW, NONE };

    ScrollBar(GuiWidget* parent, Orientation::Type orientation);

    void calculateValuesFromPosition();
    long calcScrollHeight();
    long calcScrollY();
    long calcHighestScrollY();
    long calcValue();
    void initStatically();
    
    void drawUpButton();
    void drawPressedUpButton();
    void drawDownButton();
    void drawPressedDownButton();
    void drawArea();
    void drawArrows();
    void hiliteScrollBarPartAtMousePosition(int mouseX, int mouseY);

    
    Callback1<long> changedValueCallback;
    Callback1<ScrollStep::Type> scrollStepCallback;
    
    Position position;

    const bool isV;
    
    int scrollHeight;
    int scrollY;
    
    int scrollAreaLength;
    int scrollAreaWidth;
    
    long totalValue;
    long heightValue;
    long originalTotalValue;
    long value;
    bool movingBar;
    int  movingYOffset;
    int  arrowLength;
    
    bool isButtonPressedForScrollStep;
    ScrollStep::Type scrollStep;
    Slot0 slotForScrollStepRepeating;
    void handleScrollStepRepeating();
    HilitedPart hilitedPart;
};


} // namespace LucED


#endif // SCROLLBAR_H
