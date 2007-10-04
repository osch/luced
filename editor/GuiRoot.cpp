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
#include "SystemException.hpp"

using namespace LucED;

SingletonInstance<GuiRoot> GuiRoot::instance;

static char buffer[4000];

static int myX11ErrorHandler(Display *display, XErrorEvent *errorEvent)
{
    XGetErrorText(display, errorEvent->error_code, buffer, sizeof(buffer));
    buffer[sizeof(buffer) - 1] = '\0';
    fprintf(stderr, "LucED: %s\n", buffer);
}

static bool GuiRoot_originalKeyboardModeWasAutoRepeat = false;
static bool GuiRoot_wasKeyboardModeModified           = false;

static int myFatalX11ErrorHandler(Display* display)
{
    // Handling of broken connections: try to open display again 
    // to restore AutoRepeat. This could be useful if xkill was 
    // used.

    if (GuiRoot_wasKeyboardModeModified)
    {
        Display* tryAgainDisplay = XOpenDisplay(NULL);
        if (tryAgainDisplay != NULL)
        {
            if (GuiRoot_originalKeyboardModeWasAutoRepeat) {
                XAutoRepeatOn(tryAgainDisplay);
            } else {
                XAutoRepeatOff(tryAgainDisplay);
            }
            XCloseDisplay(tryAgainDisplay);
        }
    }
    // xlib will terminate the program after this handler
    return 0;
}


GuiRoot::GuiRoot()
    : wasKeyboardModeModified(false)
{
    XSetErrorHandler(myX11ErrorHandler);
    XSetIOErrorHandler(myFatalX11ErrorHandler);

    display = XOpenDisplay(NULL);
    if (display == NULL) {
        throw SystemException(String() << "Cannot open display \"" << XDisplayName(NULL) << "\"");
    }
    screenId = XDefaultScreen(display);
    screen = XScreenOfDisplay(display, screenId);
    rootWid = XRootWindow(display, screenId);

    XKeyboardState keybstate;
    XGetKeyboardControl(display, &keybstate);
    originalKeyboardModeWasAutoRepeat = (keybstate.global_auto_repeat == AutoRepeatModeOn);

    GuiRoot_originalKeyboardModeWasAutoRepeat = originalKeyboardModeWasAutoRepeat;
    GuiRoot_wasKeyboardModeModified           = wasKeyboardModeModified;
    
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
    if (wasKeyboardModeModified) {
        if (originalKeyboardModeWasAutoRepeat) {
            XAutoRepeatOn(display);
        } else {
            XAutoRepeatOff(display);
        }
    }
    XSync(display, False);
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
    GuiRoot_wasKeyboardModeModified = wasKeyboardModeModified;

    XAutoRepeatOn(display);
}


void GuiRoot::setKeyboardAutoRepeatOff()
{
            wasKeyboardModeModified = originalKeyboardModeWasAutoRepeat;
    GuiRoot_wasKeyboardModeModified = wasKeyboardModeModified;

    XAutoRepeatOff(display);
}

void GuiRoot::setKeyboardAutoRepeatOriginal()
{
    if (wasKeyboardModeModified) {
        if (originalKeyboardModeWasAutoRepeat) {
            XAutoRepeatOn(display);
        } else {
            XAutoRepeatOff(display);
        }
    }
}

