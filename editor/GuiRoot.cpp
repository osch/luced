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

#include <stdio.h>
#include <stdlib.h>

#include "GuiRoot.hpp"
#include "GlobalConfig.hpp"

using namespace LucED;

SingletonInstance<GuiRoot> GuiRoot::instance;

static char buffer[4000];

static int myX11ErrorHandler(Display *display, XErrorEvent *errorEvent)
{
    XGetErrorText(display, errorEvent->error_code, buffer, sizeof(buffer));
    buffer[sizeof(buffer) - 1] = '\0';
    fprintf(stderr, "LucED: %s\n", buffer);
}

GuiRoot::GuiRoot()
    : wasKeyboardModeModified(false)
{
    XSetErrorHandler(myX11ErrorHandler);

    display = XOpenDisplay(NULL);
    screenId = XDefaultScreen(display);
    screen = XScreenOfDisplay(display, screenId);
    rootWid = XRootWindow(display, screenId);

    XKeyboardState keybstate;
    XGetKeyboardControl(display, &keybstate);
    originalKeyboardModeWasAutoRepeat = (keybstate.global_auto_repeat == AutoRepeatModeOn);
    
    blackColor = GuiColor(BlackPixel(display, screenId));
    whiteColor = GuiColor(WhitePixel(display, screenId));

    XColor xcolor1_st, xcolor2_st;
    
    XGetWindowAttributes(display, rootWid, &rootWinAttr);

    XAllocNamedColor(display, rootWinAttr.colormap, "grey",
            &xcolor1_st, &xcolor2_st);
    greyColor  = GuiColor(xcolor1_st.pixel);
    
    XAllocNamedColor(display, rootWinAttr.colormap, GlobalConfig::getInstance()->getGuiColor01().toCString(),
            &xcolor1_st, &xcolor2_st);
    guiColor01 = GuiColor(xcolor1_st.pixel);
    
    XAllocNamedColor(display, rootWinAttr.colormap, GlobalConfig::getInstance()->getGuiColor02().toCString(),
            &xcolor1_st, &xcolor2_st);
    guiColor02 = GuiColor(xcolor1_st.pixel);
    
    XAllocNamedColor(display, rootWinAttr.colormap, GlobalConfig::getInstance()->getGuiColor03().toCString(),
            &xcolor1_st, &xcolor2_st);
    guiColor03 = GuiColor(xcolor1_st.pixel);
    
    XAllocNamedColor(display, rootWinAttr.colormap, GlobalConfig::getInstance()->getGuiColor04().toCString(),
            &xcolor1_st, &xcolor2_st);
    guiColor04 = GuiColor(xcolor1_st.pixel);

    XAllocNamedColor(display, rootWinAttr.colormap, GlobalConfig::getInstance()->getGuiColor05().toCString(),
            &xcolor1_st, &xcolor2_st);
    guiColor05 = GuiColor(xcolor1_st.pixel);
}

GuiRoot::~GuiRoot()
{
    if (wasKeyboardModeModified && originalKeyboardModeWasAutoRepeat) {
        XAutoRepeatOn(display);
    }
    XFlush(display);
    XCloseDisplay(display);
}

GuiColor GuiRoot::getGuiColor(const String& colorName)
{
    XColor xcolor1_st, xcolor2_st;

    XAllocNamedColor(display, rootWinAttr.colormap, colorName.toCString(),
            &xcolor1_st, &xcolor2_st);
    return GuiColor(xcolor1_st.pixel);
}

void GuiRoot::setKeyboardAutoRepeatOn()
{
    wasKeyboardModeModified = !originalKeyboardModeWasAutoRepeat;
    XAutoRepeatOn(display);
}


void GuiRoot::setKeyboardAutoRepeatOff()
{
    wasKeyboardModeModified = originalKeyboardModeWasAutoRepeat;
    XAutoRepeatOff(display);
}

void GuiRoot::setKeyboardAutoRepeatOriginal()
{
    if (wasKeyboardModeModified && originalKeyboardModeWasAutoRepeat) {
       XAutoRepeatOn(display);
    }
}

