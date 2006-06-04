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

#ifndef GUIROOT_H
#define GUIROOT_H

#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xutil.h>

#include <string>

#include "HeapObject.h"
#include "GuiColor.h"
#include "SingletonInstance.h"

namespace LucED {

using std::string;

class GuiRoot : public HeapObject
{
public:

    static GuiRoot* getInstance();
    ~GuiRoot();
    
    Window getRootWid() const {
        return rootWid;
    }

    Display* getDisplay() const {
        return display;
    }
    
    int getScreenId() const {
        return screenId;
    }
    
    void setKeyboardAutoRepeatOn();
    void setKeyboardAutoRepeatOff();
    void setKeyboardAutoRepeatOriginal();
    
    GuiColor getBlackColor() const {return blackColor;}
    GuiColor getWhiteColor() const {return whiteColor;}
    GuiColor getGreyColor()  const {return greyColor;}

    GuiColor getGuiColor01() const {return guiColor01;}
    GuiColor getGuiColor02() const {return guiColor02;}
    GuiColor getGuiColor03() const {return guiColor03;}
    GuiColor getGuiColor04() const {return guiColor04;}
    GuiColor getGuiColor05() const {return guiColor05;}
    
    GuiColor getGuiColor(const string& colorName);

private:
    friend class SingletonInstance<GuiRoot>;
    
    GuiRoot();
    
    Display *display;
    XWindowAttributes  rootWinAttr;
    int screenId;
    Screen *screen;
    Window rootWid;
    bool originalKeyboardModeWasAutoRepeat;

    GuiColor blackColor;
    GuiColor whiteColor;
    GuiColor greyColor;

    GuiColor guiColor01;
    GuiColor guiColor02;
    GuiColor guiColor03;
    GuiColor guiColor04;
    GuiColor guiColor05;

    static void onexitFunc();
};


} // namespace LucED


#endif // GUIROOT_H
