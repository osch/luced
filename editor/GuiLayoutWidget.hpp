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

#ifndef GUI_LAYOUT_WIDGET_HPP
#define GUI_LAYOUT_WIDGET_HPP

#include "String.hpp"

#include "GuiWidget.hpp"
#include "OwningPtr.hpp"
#include "GuiRoot.hpp"
#include "NonFocusableWidget.hpp"

namespace LucED
{

class GuiLayoutWidget : public NonFocusableWidget
{
public:
    typedef OwningPtr<GuiLayoutWidget> Ptr;
    
    static Ptr create(int minWidth, int minHeight, int bestWidth, int bestHeight, int maxWidth, int maxHeight,
                      GuiColor color = GuiRoot::getInstance()->getGuiColor01())
    {
        return Ptr(new GuiLayoutWidget(Measures(minWidth, minHeight, bestWidth, bestHeight, maxWidth, maxHeight), color));
    }

private:
    virtual void processGuiWidgetCreatedEvent();
    virtual Measures internalGetDesiredMeasures();

private:
    GuiLayoutWidget(const Measures& m, GuiColor color);

    GuiColor color;
    Measures measures;
};

} // namespace LucED

#endif // GUI_LAYOUT_WIDGET_HPP
