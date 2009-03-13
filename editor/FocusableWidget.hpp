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

#ifndef FOCUSABLE_WIDGET_HPP
#define FOCUSABLE_WIDGET_HPP

#include "GuiWidget.hpp"

namespace LucED
{

class FocusableWidget : public GuiWidget
{
public:
    typedef OwningPtr<FocusableWidget> Ptr;
    typedef OwningPtr<const FocusableWidget> ConstPtr;

    enum FocusType {
        NO_FOCUS,
        NORMAL_FOCUS,
        TOTAL_FOCUS
    };

    bool      hasFocus()     const { return focusFlag; }
    bool      isFocusable()  const { return focusableFlag; }
    FocusType getFocusType() const { return focusType; }

    virtual void treatFocusIn()  = 0;
    virtual void treatFocusOut() = 0;

    void setNextFocusWidget(RawPtr<FocusableWidget> n) { nextFocusWidget = n; n->prevFocusWidget = this; }

    RawPtr<FocusableWidget> getNextFocusWidget() { return nextFocusWidget; }
    RawPtr<FocusableWidget> getPrevFocusWidget() { return prevFocusWidget; }
    
protected:
    FocusableWidget(GuiWidget* parent,
                    int x, int y, unsigned int width, unsigned int height, unsigned border_width)
        : GuiWidget(parent, x, y, width, height, border_width),
          focusableFlag(true),
          focusFlag(false),
          focusType(NORMAL_FOCUS)
    {}
            
    virtual ~FocusableWidget();

    void setFocusFlag(bool focusFlag) {
        this->focusFlag = focusFlag;
    }
    void setFocusableFlag(bool focusableFlag) {
        this->focusableFlag = focusableFlag;
    }
    
private:
    bool focusableFlag;
    bool focusFlag;
    FocusType focusType;
    WeakPtr<FocusableWidget> nextFocusWidget;
    WeakPtr<FocusableWidget> prevFocusWidget;
};

} // namespace LucED

#endif // FOCUSABLE_WIDGET_HPP
