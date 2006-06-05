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

#ifndef DIALOGWIN_H
#define DIALOGWIN_H

#include "TopWin.h"

namespace LucED {

class DialogWin : public TopWin
{
public:
    typedef WeakPtr<DialogWin> Ptr;


    virtual void treatNewWindowPosition(Position newPosition);
    virtual bool processEvent(const XEvent *event);
    virtual bool processKeyboardEvent(const XEvent *event);

    virtual void treatFocusIn();
    virtual void treatFocusOut();
    
protected:
    DialogWin(TopWin* referingWindow, int x, int y, unsigned int width, unsigned int height);
    
    void setRootElement(OwningPtr<GuiElement> rootElement);
    GuiElement* getRootElement() {return rootElement.getRawPtr();}
private:
    OwningPtr<GuiElement> rootElement;
};

} // namespace LucED

#endif // DIALOGWIN_H
