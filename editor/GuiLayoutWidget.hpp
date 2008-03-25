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

#ifndef GUILAYOUTWIDGET_H
#define GUILAYOUTWIDGET_H

#include "String.hpp"

#include "GuiWidget.hpp"
#include "OwningPtr.hpp"
#include "GuiRoot.hpp"

namespace LucED {



class GuiLayoutWidget : public GuiWidget
{
public:
    typedef OwningPtr<GuiLayoutWidget> Ptr;
    
    static Ptr create(GuiWidget* parent, int minWidth, int minHeight, int bestWidth, int bestHeight, int maxWidth, int maxHeight,
                      GuiColor color = GuiRoot::getInstance()->getGuiColor01())
    {
        return Ptr(new GuiLayoutWidget(parent, Measures(minWidth, minHeight, bestWidth, bestHeight, maxWidth, maxHeight), color));
    }

    //virtual bool processEvent(const XEvent *event);
    virtual Measures getDesiredMeasures();
    virtual void setPosition(Position newPosition);
    
private:
    GuiLayoutWidget(GuiWidget* parent, const Measures& m, GuiColor color);
    Measures measures;
    Position position;
};

} // namespace LucED

#endif // GUILAYOUTWIDGET_H
