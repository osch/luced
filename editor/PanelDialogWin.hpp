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

#ifndef PANEL_DIALOG_WIN_HPP
#define PANEL_DIALOG_WIN_HPP

#include "TopWin.hpp"
#include "EventDispatcher.hpp"
#include "Button.hpp"
#include "GuiLayoutColumn.hpp"
#include "DialogWin.hpp"
#include "DialogPanel.hpp"

namespace LucED
{

class PanelDialogWin : public DialogWin
{
public:
    typedef WeakPtr<PanelDialogWin> Ptr;
    
    virtual void treatFocusIn();
    virtual void treatFocusOut();
    
    void setRootElement(GuiElement::Ptr rootElement) {
        dialogPanel->setRootElement(rootElement);
    }
    
protected:
    PanelDialogWin(RawPtr<TopWin> referingWindow);
    
    void setFocus(RawPtr<FocusableElement> element) {
        dialogPanel->setFocus(element);
    }
    
    RawPtr<DialogPanel> getRootPanel() {
        return dialogPanel;
    }
    
    KeyActionHandler::Ptr getKeyActionHandler() const {
        return dialogPanel->getKeyActionHandler();
    }

private:
    DialogPanel::Ptr dialogPanel;
};

} // namespace LucED

#endif // PANEL_DIALOG_WIN_HPP
