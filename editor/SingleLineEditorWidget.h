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

#ifndef SINGLELINEEDITORWIDGET_H
#define SINGLELINEEDITORWIDGET_H

#include "Slot.h"
#include "ByteArray.h"
#include "StandardEditActions.h"
#include "TextEditorWidget.h"

namespace LucED {

class SingleLineEditorWidget : public TextEditorWidget
{
public:
    typedef OwningPtr<SingleLineEditorWidget> Ptr;

    static SingleLineEditorWidget::Ptr create(GuiWidget *parent, 
            TextStyles::Ptr textStyles, HilitedText::Ptr hilitedText)
    {
        return SingleLineEditorWidget::Ptr(new SingleLineEditorWidget(parent, textStyles, hilitedText));
    }

    virtual bool isFocusable() { return true; }
    virtual FocusType getFocusType() { return NORMAL_FOCUS; }
    

protected:
    SingleLineEditorWidget(GuiWidget *parent, 
            TextStyles::Ptr textStyles, HilitedText::Ptr hilitedText);
    
private:
    void filterInsert(const byte** buffer, long* length);
    
    Slot2<const byte**, long*> slotForInsertFilter;
    ByteArray filterBuffer;
    StandardEditActions::Ptr standardActions;
};

} // namespapce LucED

#endif // SINGLELINEEDITORWIDGET_H
