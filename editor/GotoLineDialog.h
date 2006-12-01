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

#ifndef GOTOLINEDIALOG_H
#define GOTOLINEDIALOG_H

#include "TopWin.h"
#include "EventDispatcher.h"
#include "Button.h"
#include "GuiLayoutColumn.h"
#include "DialogWin.h"
#include "Slot.h"
#include "SingleLineEditField.h"
#include "PanelDialogWin.h"

namespace LucED {

class GotoLineDialog : public PanelDialogWin
{
public:
    typedef WeakPtr<GotoLineDialog> Ptr;
    
    static GotoLineDialog::Ptr create(TopWin* referingWindow, TextEditorWidget* editorWidget) {
        return transferOwnershipTo(
                new GotoLineDialog(referingWindow, editorWidget),
                referingWindow);
    }
    
    virtual void treatFocusIn();
    virtual void requestCloseWindow();

private:
    GotoLineDialog(TopWin* referingWindow, TextEditorWidget* editorWidget);
    
    void handleButtonPressed(Button* button);
    void filterInsert(const byte** buffer, long* length);

    Button::Ptr gotoButton;
    Button::Ptr cancelButton;
    SingleLineEditField::Ptr editField;
  
    WeakPtr<TextEditorWidget> editorWidget;
    Slot1<Button*> slotForButtonPressed;
    ByteArray filterBuffer;
};

} // namespace LucED

#endif // GOTOLINEDIALOG_H
