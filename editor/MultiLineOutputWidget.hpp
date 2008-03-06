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

#ifndef MULTI_LINE_OUTPUT_WIDGET_HPP
#define MULTI_LINE_OUTPUT_WIDGET_HPP

#include "GuiWidget.hpp"
#include "OwningPtr.hpp"
#include "TextEditorWidget.hpp"

namespace LucED
{

class MultiLineOutputWidget : public GuiWidget
{
public:
    typedef OwningPtr<MultiLineOutputWidget> Ptr;

    enum Style {
        STYLE_OUTPUT,
        STYLE_GUI
    };

    static Ptr create(GuiWidget* parent, Style style, TextData::Ptr textData)
    {
        return Ptr(new MultiLineOutputWidget(parent, style, textData));
    }
    
    virtual Measures getDesiredMeasures();
    virtual void setPosition(Position p);

private:
    MultiLineOutputWidget(GuiWidget* parent, Style style, TextData::Ptr textData);

    Style                 style;
    TextStyles::Ptr       textStyles;
    TextData::Ptr         textData;
    TextEditorWidget::Ptr textWidget;
};

} // namespace LucED

#endif // MULTI_LINE_OUTPUT_WIDGET_HPP
