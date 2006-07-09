/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, osch@luced.de
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

#ifndef FINDTOPWIN_H
#define FINDTOPWIN_H

#include "TopWin.h"
#include "EventDispatcher.h"
#include "Button.h"
#include "GuiLayoutColumn.h"
#include "DialogWin.h"
#include "Slot.h"
#include "SingleLineEditorWidget.h"

namespace LucED {

class FindDialog : public DialogWin
{
public:
    typedef WeakPtr<FindDialog> Ptr;
    
    static FindDialog::Ptr create(TopWin* referingWindow, 
            int x, int y, unsigned int width, unsigned int height) {
        return transferOwnershipTo(
                new FindDialog(referingWindow, x, y, width, height),
                referingWindow);
    }
    virtual bool processKeyboardEvent(const XEvent *event);
    virtual void treatFocusIn();
    virtual void treatFocusOut();
    
    
private:
    FindDialog(TopWin* referingWindow, int x, int y, unsigned int width, unsigned int height);
    void handleButtonPressed(Button* button);

    Button::Ptr findButton, cancelButton;
    SingleLineEditorWidget::Ptr editField;
    
    Slot1<Button*> slotForButtonPressed;
};

} // namespace LucED

#endif // FINDTOPWIN_H
