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

#ifndef FRAMED_GUI_COMPOUND_HPP
#define FRAMED_GUI_COMPOUND_HPP

#include "GuiElement.hpp"
#include "GuiLayoutWidget.hpp"
#include "GuiRoot.hpp"
#include "Flags.hpp"

namespace LucED
{

class FramedGuiCompound : public GuiElement
{
public:
    typedef OwningPtr<FramedGuiCompound> Ptr;

    enum Border {
        TOP,
        RIGHT,
        BOTTOM,
        LEFT
    };
    
    typedef Flags<Border> Borders;
    
    static Ptr create(GuiWidget*      parent, 
                      GuiElement::Ptr rootElement, 
                      Borders         borders,
                      GuiColor        color = GuiRoot::getInstance()->getGuiColor01())
    {
        return Ptr(new FramedGuiCompound(parent, rootElement, borders, color));
    }

    virtual Measures getDesiredMeasures();
    virtual void setPosition(Position p);
    
    virtual void show();
    virtual void hide();
    
private:
    FramedGuiCompound(GuiWidget* parent, GuiElement::Ptr rootElement, Borders borders, GuiColor color);

    GuiElement::Ptr root;

    GuiLayoutWidget::Ptr top;
    GuiLayoutWidget::Ptr right;
    GuiLayoutWidget::Ptr bottom;
    GuiLayoutWidget::Ptr left;
    
    int ti;
    int ri;
    int bi;
    int li;
};

} // namespace LucED

#endif // FRAMED_GUI_COMPOUND_HPP
