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

#ifndef NON_FOCUSABLE_WIDGET_HPP
#define NON_FOCUSABLE_WIDGET_HPP

#include "GuiWidget.hpp"
#include "Position.hpp"
#include "RawPtr.hpp"
#include "GuiWidgetMixin.hpp"

namespace LucED
{

class NonFocusableWidget : public GuiWidgetMixin<GuiElement>
{
public:
    typedef GuiWidgetMixin<GuiElement> BaseClass;
    
    virtual void adopt(RawPtr<GuiElement>   parentElement,
                       RawPtr<GuiWidget>    parentWidget,
                       RawPtr<FocusManager> focusManager);

protected:
    NonFocusableWidget(Visibility                       defaultVisibility = VISIBLE, 
                       int                              borderWidth = 0)
        : BaseClass(defaultVisibility, borderWidth)
    {}

    virtual void processGuiWidgetCreatedEvent() = 0;

};

} // namespace LucED

#endif // NON_FOCUSABLE_WIDGET_HPP
