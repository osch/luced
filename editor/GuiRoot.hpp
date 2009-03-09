/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2007 Oliver Schmidt, oliver at luced dot de
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

#ifndef GUI_ROOT_HPP
#define GUI_ROOT_HPP

#include "WidgetId.hpp"
#include "String.hpp"
#include "HeapObject.hpp"
#include "GuiColor.hpp"
#include "SingletonInstance.hpp"
#include "Position.hpp"
          
namespace LucED
{

class GuiRoot : public HeapObject
{
public:

    static GuiRoot* getInstance() {
        return instance.getPtr();
    }
    
    ~GuiRoot();
    
    WidgetId getRootWid() const {
        return rootWid;
    }
    
    Position getRootPosition() const {
        return Position(rootWinAttr.x,
                        rootWinAttr.y,
                        rootWinAttr.width,
                        rootWinAttr.height);
    }
        
    void flushDisplay() const {
        XSync(getDisplay(), False);
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
    
    GuiColor getGuiColor(const String& colorName);

    bool hasXkbExtension() const {
        return xkbExtensionFlag;
    }
    
    bool setDetectableAutorepeat(bool flag);
    
    bool hasDetectableAutorepeat() const {
        return detecableAutorepeatFlag;
    }
    
    String getX11ServerVendorString() const {
        return x11ServerVendorString;
    }
    int getX11ServerVendorRelease() const {
        return x11ServerVendorRelease;
    }
    
private:
    friend class SingletonInstance<GuiRoot>;
    static SingletonInstance<GuiRoot> instance;
    
    GuiRoot();
    
    void evaluateConfig();
    
    Display* display;
    XWindowAttributes  rootWinAttr;
    int screenId;
    Screen* screen;
    WidgetId rootWid;

    GuiColor blackColor;
    GuiColor whiteColor;
    GuiColor greyColor;

    GuiColor guiColor01;
    GuiColor guiColor02;
    GuiColor guiColor03;
    GuiColor guiColor04;
    GuiColor guiColor05;

    bool xkbExtensionFlag;
    bool hadDetecableAutorepeatFlag;
    bool detecableAutorepeatFlag;
    
    String x11ServerVendorString;
    int    x11ServerVendorRelease;
};

} // namespace LucED

#endif // GUI_ROOT_HPP
