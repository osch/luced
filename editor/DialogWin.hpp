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
#include "KeyMapping.hpp"
#include "HeapObjectArray.hpp"
#include "WeakPtrQueue.hpp"

namespace LucED
{

class DialogWin : public TopWin
{
public:
    typedef WeakPtr<DialogWin> Ptr;

    virtual void treatNewWindowPosition(Position newPosition);
    virtual ProcessingResult processKeyboardEvent(const XEvent *event);

    virtual void show();
    
    void setReferingWindowForPositionHintsOnly(TopWin* referingWindow);

protected:
    DialogWin(TopWin* referingWindow);
    
    void setRootElement(OwningPtr<GuiElement> rootElement);
    GuiElement* getRootElement() {return rootElement.getRawPtr();}
    
    
private:
    void requestCloseWindowByUser();
    
    void prepareSizeHints();
    
    void notifyAboutReferingWindowMapping(bool isReferingWindowMapped);

    OwningPtr<GuiElement> rootElement;
    bool wasNeverShown;
    WeakPtr<TopWin> referingWindow;
    
    KeyMapping::Ptr keyMapping;
    
    bool shouldBeMapped;
};

} // namespace LucED

#endif // DIALOG_WIN_HPP
