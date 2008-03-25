/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2008 Oliver Schmidt, oliver at luced dot de
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

#ifndef SCROLLABLE_TEXT_GUI_COMPOUND_HPP
#define SCROLLABLE_TEXT_GUI_COMPOUND_HPP

#include "GuiElement.hpp"
#include "TextEditorWidget.hpp"
#include "Flags.hpp"
#include "ScrollBar.hpp"
#include "GuiLayoutColumn.hpp"
#include "GuiLayoutRow.hpp"
#include "FramedGuiCompound.hpp"

namespace LucED
{

class ScrollableTextGuiCompound : public GuiElement
{
public:
    typedef OwningPtr<ScrollableTextGuiCompound> Ptr;
    
    enum Option {
        DYNAMIC_SCROLL_BAR_DISPLAY,
        WITHOUT_OUTER_FRAME
    };
    
    typedef Flags<Option> Options;
    
    static Ptr create(GuiWidget*            parent, 
                      TextEditorWidget::Ptr textWidget, 
                      Options               options = Options())
    {
        return Ptr(new ScrollableTextGuiCompound(parent, textWidget, options));
    }    

    virtual Measures getDesiredMeasures();
    virtual void setPosition(Position p);

protected:
    ScrollableTextGuiCompound(GuiWidget*            parent, 
                              TextEditorWidget::Ptr textWidget, 
                              Options               options);
private:
    TextEditorWidget::Ptr textWidget;
    bool dynamicScrollBarDisplayFlag;

    GuiLayoutColumn::Ptr layoutColumn1;
    GuiLayoutColumn::Ptr layoutColumn2;
    GuiElement::Ptr      rootElement;
    
    ScrollBar::Ptr         scrollBarH;
    ScrollBar::Ptr         scrollBarV;
    FramedGuiCompound::Ptr horizontalScrollBarFrame;
    GuiLayoutWidget::Ptr   scrollBarSpacer;
    GuiLayoutWidget::Ptr   verticalScrollBarLeftBorder;
};


} // namespace LucED

#endif // SCROLLABLE_TEXT_GUI_COMPOUND_HPP
