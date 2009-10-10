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

#include <stdio.h>
#include <stdlib.h>

#include "GuiRoot.hpp"
#include "GlobalConfig.hpp"
#include "SystemException.hpp"
#include "GuiRootProperty.hpp"
#include "System.hpp"

#ifdef USE_X11_XKB_EXTENSION
#include <X11/XKBlib.h>
#endif

using namespace LucED;

SingletonInstance<GuiRoot> GuiRoot::instance;

static char buffer[4000];

static int myX11ErrorHandler(Display* display, XErrorEvent* errorEvent)
{
    XGetErrorText(display, errorEvent->error_code, buffer, sizeof(buffer));
    buffer[sizeof(buffer) - 1] = '\0';
    fprintf(stderr, "LucED: xlib error: %s\n", buffer);
    return 0;
}

static const char* KEYBOARD_WAS_AUTOREPEAT_PROPERTY_NAME = "LUCED_KEYBOARD_WAS_AUTOREPEAT";

static bool GuiRoot_knowsOriginalKeyboardMode = false;
static bool GuiRoot_originalKeyboardModeWasAutoRepeat = true;
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
            {
                // Delete RootProperty
             
                Atom    atom     = XInternAtom(tryAgainDisplay, KEYBOARD_WAS_AUTOREPEAT_PROPERTY_NAME, False);
                int     screenId = XDefaultScreen(tryAgainDisplay);
                Screen* screen   = XScreenOfDisplay(tryAgainDisplay, screenId);
                Window  rootWid  = XRootWindow(tryAgainDisplay, screenId);
                XDeleteProperty(tryAgainDisplay, rootWid, atom);
            }            
            XCloseDisplay(tryAgainDisplay);
        }
    }
    // xlib will terminate the program after this handler
    return 0;
}

static bool GuiRoot_queryOriginalKeyboardModeWasAutoRepeat()
{
    bool wasAutoRepeat = true;
    
    GuiRootProperty prop(KEYBOARD_WAS_AUTOREPEAT_PROPERTY_NAME);
    if (prop.exists()) {
        wasAutoRepeat = (prop.getValue() == "false" ? false : true);
    } else {
        XKeyboardState keybstate;
        XGetKeyboardControl(GuiRoot::getInstance()->getDisplay(), &keybstate);
        wasAutoRepeat = (keybstate.global_auto_repeat == AutoRepeatModeOn);
        prop.setValue(wasAutoRepeat ? "true" : "false");
    }
    return wasAutoRepeat;
}

GuiRoot::GuiRoot()
    : xkbExtensionFlag(false),
      hadDetecableAutorepeatFlag(false),
      detecableAutorepeatFlag(false),
      x11InputMethod(NULL)
{
    XSetErrorHandler(myX11ErrorHandler);
    XSetIOErrorHandler(myFatalX11ErrorHandler);

    display = XOpenDisplay(NULL);
    if (display == NULL) {
        throw SystemException(String() << "Cannot open display \"" << XDisplayName(NULL) << "\"");
    }
    XSetLocaleModifiers("");
    if (!XSupportsLocale()) {
        throw SystemException(String() << "Xlib: locale \"" << System::getInstance()->getLocaleName() << "\" not supported.");
    }

    x11InputMethod = XOpenIM(display, NULL, NULL, NULL);
    x11AtomForUtf8String = XInternAtom(display, "UTF8_STRING", False);

    screenId = XDefaultScreen(display);
    screen = XScreenOfDisplay(display, screenId);
    rootWid = WidgetId(XRootWindow(display, screenId));

    x11ServerVendorString  = XServerVendor(display);
    x11ServerVendorRelease = XVendorRelease(display);
    
    evaluateConfig();

    GlobalConfig::getInstance()->registerConfigChangedCallback(newCallback(this, &GuiRoot::evaluateConfig));
}

GuiRoot::~GuiRoot()
{
    if (GuiRoot_wasKeyboardModeModified) {
        if (GuiRoot_originalKeyboardModeWasAutoRepeat) {
            XAutoRepeatOn(display);
        } else {
            XAutoRepeatOff(display);
        }
        GuiRoot_wasKeyboardModeModified = false;
        GuiRoot_knowsOriginalKeyboardMode = false;
        {
            // Delete RootProperty
         
            Atom atom = XInternAtom(display, KEYBOARD_WAS_AUTOREPEAT_PROPERTY_NAME, False);
            XDeleteProperty(display, rootWid, atom);
        }            
    }
    if (x11InputMethod != NULL) {
        XCloseIM(x11InputMethod);
    }
    XSync(display, False);
    XCloseDisplay(display);
}

void GuiRoot::evaluateConfig()
{
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

    xkbExtensionFlag = false;
#ifdef USE_X11_XKB_EXTENSION
    if (!GlobalConfig::getInstance()->getDoNotUseX11XkbExtension())
    {
        int xkbMajorVersion = XkbMajorVersion;
        int xbkMinorVersion = XkbMinorVersion;
        xkbExtensionFlag = XkbLibraryVersion(&xkbMajorVersion, &xbkMinorVersion);
        if (xkbExtensionFlag)
        {
            int opcode;
            int event;
            int error;
            xkbExtensionFlag = XkbQueryExtension(display, 
                                                 &opcode,
                                                 &event,
                                                 &error,
                                                 &xkbMajorVersion,
                                                 &xbkMinorVersion);
        }
    }

    if (xkbExtensionFlag && !hasDetectableAutorepeat() && hadDetecableAutorepeatFlag)
    {
        setDetectableAutorepeat(true);
    }
    
    if (!xkbExtensionFlag && hasDetectableAutorepeat())
    {
        setDetectableAutorepeat(false);
        hadDetecableAutorepeatFlag = true;
    }
#endif

}

bool GuiRoot::setDetectableAutorepeat(bool flag)
{
    ASSERT(flag == false || xkbExtensionFlag);

    hadDetecableAutorepeatFlag = flag;

#ifdef USE_X11_XKB_EXTENSION
    int supportedFlag;
    detecableAutorepeatFlag = XkbSetDetectableAutoRepeat(display, flag, &supportedFlag);
    detecableAutorepeatFlag = detecableAutorepeatFlag && supportedFlag;
    return detecableAutorepeatFlag;
#else
    return false;
#endif
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
    if (!GuiRoot_knowsOriginalKeyboardMode) {
        GuiRoot_originalKeyboardModeWasAutoRepeat = GuiRoot_queryOriginalKeyboardModeWasAutoRepeat();
        GuiRoot_knowsOriginalKeyboardMode = true;
    }
    GuiRoot_wasKeyboardModeModified = !GuiRoot_originalKeyboardModeWasAutoRepeat;
    
    XAutoRepeatOn(display);
}


void GuiRoot::setKeyboardAutoRepeatOff()
{
    if (!GuiRoot_knowsOriginalKeyboardMode) {
        GuiRoot_originalKeyboardModeWasAutoRepeat = GuiRoot_queryOriginalKeyboardModeWasAutoRepeat();
        GuiRoot_knowsOriginalKeyboardMode = true;
    }
    GuiRoot_wasKeyboardModeModified = GuiRoot_originalKeyboardModeWasAutoRepeat;
    
    XAutoRepeatOff(display);
}

void GuiRoot::setKeyboardAutoRepeatOriginal()
{
    if (GuiRoot_wasKeyboardModeModified) {
        if (GuiRoot_originalKeyboardModeWasAutoRepeat) {
            XAutoRepeatOn(display);
        } else {
            XAutoRepeatOff(display);
        }
        GuiRoot_wasKeyboardModeModified = false;
    }
}

