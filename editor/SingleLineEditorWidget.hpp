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

#ifndef SINGLE_LINE_EDITOR_WIDGET_HPP
#define SINGLE_LINE_EDITOR_WIDGET_HPP

#include "ByteArray.hpp"
#include "TextEditorWidget.hpp"

namespace LucED
{

class SingleLineEditorWidget : public TextEditorWidget
{
public:
    typedef OwningPtr<SingleLineEditorWidget> Ptr;

    static SingleLineEditorWidget::Ptr create(GuiWidget* parent, TextStyles::Ptr  textStyles, 
                                                                 HilitedText::Ptr hilitedText)
    {
        return SingleLineEditorWidget::Ptr(new SingleLineEditorWidget(parent, textStyles, 
                                                                              hilitedText));
    }

    virtual bool isFocusable() { return true; }
    virtual FocusType getFocusType() { return NORMAL_FOCUS; }
    

protected:
    SingleLineEditorWidget(GuiWidget* parent, TextStyles::Ptr  textStyles, 
                                              HilitedText::Ptr hilitedText);
    
private:
    void filterInsert(const byte** buffer, long* length);
    
    ByteArray filterBuffer;
};

} // namespapce LucED

#endif // SINGLE_LINE_EDITOR_WIDGET_HPP
