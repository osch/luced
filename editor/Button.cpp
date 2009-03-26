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

#include <X11/keysym.h>

#include "Button.hpp"
#include "GuiRoot.hpp"
#include "TopWin.hpp"
#include "GlobalConfig.hpp"
#include "System.hpp"

using namespace LucED;

const int BUTTON_OUTER_BORDER = 1;

Button::Button(GuiWidget* parent, String buttonText)
      : FocusableElement(parent, 0, 0, 1, 1, 0),
        position(0, 0, 1, 1),
        isButtonPressed(false),
        isMouseButtonPressed(false),
        isMouseOverButton(false),
        isDefaultButton(false),
        hasHotKeyFlag(false),
        showHotKeyFlag(false),
        isExplicitDefaultButton(false),
        hasForcedMeasuresFlag(false)
{
    addToXEventMask(ExposureMask|ButtonPressMask|ButtonReleaseMask|ButtonMotionMask|EnterWindowMask|LeaveWindowMask);
    setBackgroundColor(getGuiRoot()->getGuiColor03());
    setButtonText(buttonText);
    
    KeyActionHandler::Ptr keyActionHandler = KeyActionHandler::create();
    keyActionHandler->addActionMethods(Actions::create(this));
    setKeyActionHandler(keyActionHandler);
}

void Button::setButtonText(String buttonText)
{
    if (hasHotKeyFlag) {
        String keySymString;
        keySymString.appendUpperChar(hotKeyChar);
        requestRemovalOfHotKeyRegistrationFor(KeyMapping::Id(KeyModifier("Alt"), KeyId(keySymString)), this);
        hasHotKeyFlag = false;
    }
    int p1 = buttonText.findFirstOf(']', Pos(1));
    
    String oldButtonText = this->buttonText;
    char oldHotkeyChar   = this->hotKeyChar;
    
    if (p1 != -1) {
        hotKeyChar = buttonText[p1 - 1];
        this->buttonText = String() << buttonText.getHead(p1) << buttonText.getTail(p1 + 1);
        hotKeyPixX = getGuiTextStyle()->getTextWidth(buttonText.getSubstring(Pos(0), Len(p1 - 1)));
        hotKeyPixW = getGuiTextStyle()->getCharWidth(hotKeyChar);
        hasHotKeyFlag = true;
        // showHotKeyFlag = true;
        String keySymString;
        keySymString.appendUpperChar(hotKeyChar);
        requestHotKeyRegistrationFor(KeyMapping::Id(KeyModifier("Alt"), KeyId(keySymString)), this);
    } else {
        this->buttonText = buttonText;
    }
    if (isVisible() && (oldButtonText != this->buttonText
                     || oldHotkeyChar != this->hotKeyChar)) {
        drawButton();
    }
}

int Button::getStandardHeight()
{
    int guiSpacing = GlobalConfig::getInstance()->getGuiSpacing();
    int buttonInnerSpacing = GlobalConfig::getInstance()->getButtonInnerSpacing();

    return getGuiTextHeight() + 2 * getRaisedBoxBorderWidth()  + 2*buttonInnerSpacing + 2*BUTTON_OUTER_BORDER + guiSpacing;
}

void Button::setDesiredMeasures(Measures m)
{
    hasForcedMeasuresFlag = true;
    forcedMeasures = m;
}

GuiElement::Measures Button::getDesiredMeasures()
{
    if (hasForcedMeasuresFlag) {
        return forcedMeasures;
    } else {
        return getOwnDesiredMeasures();
    }
}

GuiElement::Measures Button::getOwnDesiredMeasures()
{
    int guiSpacing = GlobalConfig::getInstance()->getGuiSpacing();

    int buttonInnerSpacing = GlobalConfig::getInstance()->getButtonInnerSpacing();
    int minWidth = getGuiTextStyle()->getTextWidth(buttonText) + 2 * getRaisedBoxBorderWidth() + 2*buttonInnerSpacing + 2*BUTTON_OUTER_BORDER + guiSpacing;
    int minHeight = getGuiTextHeight() + 2 * getRaisedBoxBorderWidth()  + 2*buttonInnerSpacing + 2*BUTTON_OUTER_BORDER + guiSpacing;

    int bestWidth = minWidth + 4 * getGuiTextStyle()->getSpaceWidth() + 2*BUTTON_OUTER_BORDER + guiSpacing;
    int bestHeight = minHeight;

    return Measures(minWidth, minHeight, bestWidth, bestHeight, bestWidth, bestHeight);
}


void Button::setPosition(Position newPosition)
{
    if (position != newPosition) {
        GuiWidget::setPosition(newPosition);
        this->position = newPosition;
    }
}

void Button::drawButton()
{
    int guiSpacing = GlobalConfig::getInstance()->getGuiSpacing();

    int textOffset = 0;
    GuiColor color;
    if (isMouseOverButton) {
        color = GuiRoot::getInstance()->getGuiColor04();
    } else {
        color = GuiRoot::getInstance()->getGuiColor03();
    }
    if (isButtonPressed) {
        drawPressedBox(BUTTON_OUTER_BORDER + guiSpacing, BUTTON_OUTER_BORDER + guiSpacing, 
                position.w - 2*BUTTON_OUTER_BORDER - guiSpacing, position.h - 2*BUTTON_OUTER_BORDER - guiSpacing, color);
        textOffset = 1;
    } else {
        drawRaisedBox(BUTTON_OUTER_BORDER + guiSpacing, BUTTON_OUTER_BORDER + guiSpacing, 
                position.w - 2*BUTTON_OUTER_BORDER - guiSpacing, position.h - 2*BUTTON_OUTER_BORDER - guiSpacing, color);
    }
    if (isDefaultButton) {
        const int d = BUTTON_OUTER_BORDER - 1;
        drawFrame(d + guiSpacing, d + guiSpacing, position.w - 2 * d - guiSpacing, position.h - 2 * d - guiSpacing);
    } else {
        const int d = BUTTON_OUTER_BORDER - 1;
        undrawFrame(d + guiSpacing, d + guiSpacing, position.w - 2 * d - guiSpacing, position.h - 2 * d - guiSpacing);
    }
    if (hasFocus()) {
        const int d = BUTTON_OUTER_BORDER + 1;
        if (isButtonPressed) {
            drawDottedFrame(d+1 + guiSpacing, d+1 + guiSpacing, position.w - 2 * d - 1 - guiSpacing, position.h - 2 * d - 1 - guiSpacing);
        } else {
            drawDottedFrame(d + guiSpacing, d + guiSpacing, position.w - 2 * d - 1 - guiSpacing, position.h - 2 * d - 1 - guiSpacing);
        }
    }
    int w = getGuiTextStyle()->getTextWidth(buttonText);
    int x = (position.w - 2*BUTTON_OUTER_BORDER - w - guiSpacing) / 2 + BUTTON_OUTER_BORDER + guiSpacing;
    if (x < BUTTON_OUTER_BORDER + guiSpacing) { x = BUTTON_OUTER_BORDER + guiSpacing; }
    int y = (position.h - 2*BUTTON_OUTER_BORDER - getGuiTextHeight() - guiSpacing) / 2 + BUTTON_OUTER_BORDER + guiSpacing;
    if (y < BUTTON_OUTER_BORDER + guiSpacing) { y = BUTTON_OUTER_BORDER + guiSpacing; }
    if (showHotKeyFlag) {
        //drawLine(x + textOffset + hotKeyPixX, y + textOffset + getGuiTextHeight(), hotKeyPixW, 0);
        int lineY = getGuiTextStyle()->getLineAscent() + 1;
        if (lineY > getGuiTextHeight() - 1) {
            lineY = getGuiTextHeight() - 1;
        }
        drawLine(x + textOffset + hotKeyPixX, y + textOffset + lineY, hotKeyPixW, 0);
    }
    drawGuiText(x + textOffset, y + textOffset, buttonText);
}


bool Button::isMouseInsideButtonArea(int mouseX, int mouseY)
{
    int x = mouseX;
    int y = mouseY;
    return (x >= BUTTON_OUTER_BORDER - 1 && x <= position.w - 2*BUTTON_OUTER_BORDER + 1
                            && y >= BUTTON_OUTER_BORDER - 1 && y <= position.h - 2*BUTTON_OUTER_BORDER + 1);
}

static const MicroSeconds shortTime = MicroSeconds(20 * 1000);

static void waitShort(MicroSeconds microSecs = shortTime)
{
    if (microSecs > 0) {
        TimeVal timeVal(microSecs);
        System::select(0, NULL, NULL, NULL, &timeVal);
    }
}

GuiElement::ProcessingResult Button::processEvent(const XEvent *event)
{
    if (GuiWidget::processEvent(event) == EVENT_PROCESSED) {
        return EVENT_PROCESSED;
    } else {
        
        switch (event->type) {
            
            case GraphicsExpose:
                if (event->xgraphicsexpose.count > 0) {
                    break;
                }
            case Expose: {
                if (event->xexpose.count > 0) {
                    break;
                }
                drawButton();
                return EVENT_PROCESSED;
            }

            case ButtonPress: {
                if (!hasFocus()) {
                    reportMouseClickFrom(this);
                }
                if (event->xbutton.button == Button1 || (this->doesReactOnRightClick() && event->xbutton.button == Button3))
                {
                    earliestButtonReleaseTime.setToCurrentTime().add(shortTime);
                    
                    isMouseButtonPressed = true;
                    int x = event->xbutton.x;
                    int y = event->xbutton.y;
                    
                    if (isMouseInsideButtonArea(x, y))
                    {
                        isButtonPressed = true;
                    } else {
                        isButtonPressed = false;
                    }
                    drawButton();
                    return EVENT_PROCESSED;
                }
                break;
            }

            case ButtonRelease: {
                isMouseButtonPressed = false;
                if (isButtonPressed) {
                    TimeVal currentTime; currentTime.setToCurrentTime();
                    if (earliestButtonReleaseTime.isLaterThan(currentTime)) {
                        waitShort(TimeVal::diffMicroSecs(currentTime, earliestButtonReleaseTime));
                    }
                    isButtonPressed = false;
                    drawButton();
                    XSync(getDisplay(), False);
                    int x = event->xbutton.x;
                    int y = event->xbutton.y;
                    if (isMouseInsideButtonArea(x, y))
                    {
                        if (event->xbutton.button == Button1) {
                            if (pressedCallback0->isEnabled()) {
                                pressedCallback0->call();
                            } else {
                                pressedCallback1->call(this, WAS_MOUSE_CLICK);
                            }
                        }
                        else if (event->xbutton.button == Button3) 
                        {
                            if (rightClickedCallback0->isEnabled()) {
                                rightClickedCallback0->call();
                            } else {
                                rightClickedCallback1->call(this, WAS_MOUSE_CLICK);
                            }
                        }
                    }
                }
                return EVENT_PROCESSED;
            }

            case MotionNotify:
            {
                bool mustDraw = false;
                XEvent newEvent;

                XSync(getDisplay(), False);
                if (XCheckWindowEvent(getDisplay(), getWid(), ButtonMotionMask|PointerMotionMask, &newEvent) == True) {
                    event = &newEvent;
                    while (XCheckWindowEvent(getDisplay(), getWid(), ButtonMotionMask|PointerMotionMask, &newEvent) == True);
                }
                int x = event->xmotion.x;
                int y = event->xmotion.y;

                if (isMouseButtonPressed)
                {
                    if (isMouseInsideButtonArea(x, y))
                    {
                        if (!isButtonPressed) {
                            isButtonPressed = true;
                            mustDraw = true;
                        }
                    } else {
                        if (isButtonPressed) {
                            isButtonPressed = false;
                            mustDraw = true;
                        }
                    }
                }
                if (isMouseInsideButtonArea(x, y)) {
                    if (!isMouseOverButton) {
                        isMouseOverButton = true;
                        mustDraw = true;
                    }
                } else {
                    if (isMouseOverButton) {
                        isMouseOverButton = false;
                        mustDraw = true;
                    }
                }
                if (mustDraw) {
                    drawButton();
                }
                return EVENT_PROCESSED;
            }
            
            case EnterNotify: {
                int x = event->xcrossing.x;
                int y = event->xcrossing.y;
                if (isMouseInsideButtonArea(x, y) && !isMouseOverButton) {
                    isMouseOverButton = true;
                    drawButton();
                }
                addToXEventMask(PointerMotionMask);
                return EVENT_PROCESSED;
            }
            
            case LeaveNotify: {
                if (isMouseOverButton) {
                    isMouseOverButton = false;
                    drawButton();
                }
                removeFromXEventMask(PointerMotionMask);
                return EVENT_PROCESSED;
            }
        }
        return propagateEventToParentWidget(event);
    }
}


void Button::treatLostHotKeyRegistration(const KeyMapping::Id& id)
{
    if (id == KeyMapping::Id(KeyModifier(), KeyId("Return")))
    {
        if (isDefaultButton) {
            isDefaultButton = false;
            drawButton();
        }
    } else {
        if (showHotKeyFlag) {
            showHotKeyFlag = false;
            drawButton();
        }
    }
}


void Button::treatNewHotKeyRegistration(const KeyMapping::Id& id)
{
    if (id == KeyMapping::Id(KeyModifier(), KeyId("Return")))
    {
        if (!isDefaultButton) {
            isDefaultButton = true;
            drawButton();
        }
    } else {
        if (!showHotKeyFlag) {
            showHotKeyFlag = true;
            drawButton();
        }
    }
}


void Button::treatFocusIn()
{
    if (!isDefaultButton) {
        requestHotKeyRegistrationFor(KeyMapping::Id(KeyModifier(), KeyId("Return")),   this);
    }
    if (!hasFocus()) {
        setFocusFlag(true);
        drawButton();
    }
}


void Button::treatFocusOut()
{
    setFocusFlag(false);
    if (!isExplicitDefaultButton) {
        requestRemovalOfHotKeyRegistrationFor(KeyMapping::Id(KeyModifier(), KeyId("Return")),   this);
    }
    drawButton();
}


void Button::emulateButtonPress(bool isDefaultKey, bool isRightClicked)
{
    if (isRightClicked && !this->doesReactOnRightClick()) {
        return;
    }
    bool oldIsButtonPressed = isButtonPressed;
    isButtonPressed = true;
    drawButton();
    XSync(getDisplay(), False); waitShort();
    isButtonPressed = false;
    drawButton();
    XSync(getDisplay(), False); waitShort();
    isButtonPressed = oldIsButtonPressed;
    drawButton();
    XSync(getDisplay(), False); waitShort();

    if (isRightClicked)
    {
        if (rightClickedCallback0->isEnabled()) {
            rightClickedCallback0->call();
        } else {
            rightClickedCallback1->call(this, isDefaultKey ? WAS_DEFAULT_KEY
                                                           : WAS_HOT_KEY);
        }
    }
    else
    {
        if (pressedCallback0->isEnabled()) {
            pressedCallback0->call();
        } else {
            pressedCallback1->call(this, isDefaultKey ? WAS_DEFAULT_KEY
                                                      : WAS_HOT_KEY);
        }
    }
}

void Button::treatHotKeyEvent(const KeyMapping::Id& id)
{
    emulateButtonPress(id == KeyMapping::Id(KeyModifier(), KeyId("Return")),
                       id.getKeyModifier().containsShiftKey());
}

void Button::setAsDefaultButton(bool isDefault)
{
    if (isDefault != isExplicitDefaultButton) {
        if (isDefault) {
            requestHotKeyRegistrationFor(KeyMapping::Id(KeyModifier(), KeyId("Return")),   this);
            isExplicitDefaultButton = true;
        } else {
            isExplicitDefaultButton = false;
        }
        drawButton();
    }
}
