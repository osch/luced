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

#ifndef DIALOG_WIN_HPP
#define DIALOG_WIN_HPP

#include "TopWin.hpp"
#include "HeapObjectArray.hpp"
#include "WeakPtrQueue.hpp"
#include "ActionKeySequenceHandler.hpp"

namespace LucED
{

class DialogWin : public TopWin
{
public:
    typedef WeakPtr<DialogWin> Ptr;

    virtual void treatNewWindowPosition(Position newPosition);

    virtual ProcessingResult processKeyboardEvent(const KeyPressEvent& keyPressEvent);

    virtual void show();
    
    void setReferingWindowForPositionHintsOnly(TopWin* referingWindow);

    virtual void requestCloseWindow(TopWin::CloseReason reason);

    virtual void treatFocusIn();
    virtual void treatFocusOut();

    virtual ActionKeyConfig::Ptr getActionKeyConfig() {
        return actionKeyConfig;
    }
    
protected:
    DialogWin(TopWin* referingWindow);
    
    void setRootElement(OwningPtr<GuiWidget> rootElement);
    GuiWidget* getRootElement() {return rootElement.getRawPtr();}
    
    
private:
    void requestCloseWindowByUser();
    
    void prepareSizeHints();
    
    void notifyAboutReferingWindowMapping(bool isReferingWindowMapped);

    OwningPtr<GuiWidget> rootElement;
    bool wasNeverShown;
    WeakPtr<TopWin> referingWindow;
    
    bool shouldBeMapped;
    
    ActionKeyConfig::Ptr     actionKeyConfig;
    ActionKeySequenceHandler actionKeySequenceHandler;
};

} // namespace LucED

#endif // DIALOG_WIN_HPP
