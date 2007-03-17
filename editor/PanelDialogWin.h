/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, oliver at luced dot de
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

#ifndef PANELDIALOGWIN_H
#define PANELDIALOGWIN_H

#include "TopWin.h"
#include "EventDispatcher.h"
#include "Button.h"
#include "GuiLayoutColumn.h"
#include "DialogWin.h"
#include "Slot.h"
#include "SingleLineEditField.h"
#include "DialogPanel.h"

namespace LucED {

class PanelDialogWin : public DialogWin
{
public:
    typedef WeakPtr<PanelDialogWin> Ptr;
    
    virtual void treatFocusIn();
    virtual void treatFocusOut();
    
    virtual ProcessingResult processKeyboardEvent(const XEvent *event);
    virtual void requestHotKeyRegistrationFor(const KeyMapping::Id& id, GuiWidget* w);
    virtual void requestRemovalOfHotKeyRegistrationFor(const KeyMapping::Id& id, GuiWidget* w);
    void setRootElement(OwningPtr<GuiElement> rootElement);
    
protected:
    PanelDialogWin(TopWin* referingWindow);
    void setFocus(GuiWidget* element);

private:
    DialogPanel::Ptr dialogPanel;
};

} // namespace LucED

#endif // PANELDIALOGWIN_H