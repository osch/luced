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

#ifndef GOTOLINEPANEL_H
#define GOTOLINEPANEL_H

#include "DialogPanel.h"
#include "Button.h"
#include "CheckBox.h"
#include "TextEditorWidget.h"
#include "SingleLineEditField.h"

namespace LucED {

class GotoLinePanel : public DialogPanel
{
public:
    typedef OwningPtr<GotoLinePanel> Ptr;

    static Ptr create(GuiWidget* parent, TextEditorWidget* editorWidget) {
        return Ptr(new GotoLinePanel(parent, editorWidget));
    }
    
    virtual void treatFocusIn();
    
private:
    GotoLinePanel(GuiWidget* parent, TextEditorWidget* editorWidget);

    void handleButtonPressed(Button* button);

    void filterInsert(const byte** buffer, long* length);

    Button::Ptr gotoButton;
    Button::Ptr cancelButton;
    SingleLineEditField::Ptr editField;
  
    WeakPtr<TextEditorWidget> editorWidget;
    ByteArray filterBuffer;
};

} // namespace LucED

#endif // GOTOLINEPANEL_H
