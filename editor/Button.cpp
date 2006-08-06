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

#include "Button.h"
#include "GuiRoot.h"
#include "TopWin.h"

using namespace LucED;

#define BUTTON_BORDER 2

Button::Button(GuiWidget* parent, string buttonText)
      : GuiWidget(parent, 0, 0, 1, 1, 0),
        position(0, 0, 1, 1),
        buttonText(buttonText),
        isButtonPressed(false),
        isMouseButtonPressed(false),
        isMouseOverButton(false),
        isDefaultButton(false),
        hasFocus(false)
{
    addToXEventMask(ExposureMask|ButtonPressMask|ButtonReleaseMask|ButtonMotionMask|EnterWindowMask|LeaveWindowMask);
    setBackgroundColor(getGuiRoot()->getGuiColor03());
}

GuiElement::Measures Button::getDesiredMeasures()
{
    int minWidth = getGuiTextStyle()->getTextWidth(buttonText) + 2 * getRaisedBoxBorderWidth() + 6 + 2*BUTTON_BORDER;
    int minHeight = getGuiTextHeight() + 2 * getRaisedBoxBorderWidth() + 6 + 2*BUTTON_BORDER;

    int bestWidth = minWidth + 4 * getGuiTextStyle()->getSpaceWidth() + 2*BUTTON_BORDER;
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
    int textOffset = 0;
    GuiColor color;
    if (isMouseOverButton) {
        color = GuiRoot::getInstance()->getGuiColor04();
    } else {
        color = GuiRoot::getInstance()->getGuiColor03();
    }
    if (isButtonPressed) {
        drawPressedBox(BUTTON_BORDER, BUTTON_BORDER, 
                position.w - 2*BUTTON_BORDER, position.h - 2*BUTTON_BORDER, color);
        textOffset = 1;
    } else {
        drawRaisedBox(BUTTON_BORDER, BUTTON_BORDER, 
                position.w - 2*BUTTON_BORDER, position.h - 2*BUTTON_BORDER, color);
    }
    if (isDefaultButton) {
        const int d = BUTTON_BORDER - 1;
        drawFrame(d, d, position.w - 2 * d, position.h - 2 * d);
    } else {
        const int d = BUTTON_BORDER - 1;
        undrawFrame(d, d, position.w - 2 * d, position.h - 2 * d);
    }
    if (hasFocus) {
        const int d = BUTTON_BORDER + 1;
        if (isButtonPressed) {
            drawDottedFrame(d+1, d+1, position.w - 2 * d - 1, position.h - 2 * d - 1);
        } else {
            drawDottedFrame(d, d, position.w - 2 * d - 1, position.h - 2 * d - 1);
        }
    }
    int w = getGuiTextStyle()->getTextWidth(buttonText);
    int x = (position.w - 2*BUTTON_BORDER - w) / 2 + BUTTON_BORDER;
    if (x < BUTTON_BORDER) { x = BUTTON_BORDER; }
    int y = (position.h - 2*BUTTON_BORDER - getGuiTextHeight()) / 2 + BUTTON_BORDER;
    if (y < BUTTON_BORDER) { y = BUTTON_BORDER; }
    drawGuiText(x + textOffset, y + textOffset, buttonText);
}


bool Button::isMouseInsideButtonArea(int mouseX, int mouseY)
{
    int x = mouseX;
    int y = mouseY;
    return (x >= BUTTON_BORDER - 1 && x <= position.w - 2*BUTTON_BORDER + 1
                            && y >= BUTTON_BORDER - 1 && y <= position.h - 2*BUTTON_BORDER + 1);
}

static const int shortTime = 50 * 1000;

static void waitShort(int microSecs = shortTime)
{
    if (microSecs > 0) {
        TimeVal timeVal;
        timeVal.addMicroSecs(microSecs);
        LucED::select(0, NULL, NULL, NULL, &timeVal);
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
                if (event->xbutton.button == Button1)
                {
                    earliestButtonReleaseTime.setToCurrentTime().addMicroSecs(shortTime);
                    
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
                    XFlush(getDisplay());
                    int x = event->xbutton.x;
                    int y = event->xbutton.y;
                    if (isMouseInsideButtonArea(x, y)) {
                        pressedCallback.call(this);
                    }
                }
                return EVENT_PROCESSED;
            }

            case MotionNotify:
            {
                bool mustDraw = false;
                XEvent newEvent;

                XFlush(getDisplay());
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

void Button::treatLostDefaultButtonState()
{
    isDefaultButton = false;
    drawButton();
}

void Button::treatNewDefaultButtonState()
{
    isDefaultButton = true;
    drawButton();
}


void Button::treatFocusIn()
{
    hasFocus = true;
    requestToBeActualDefaultButtonWidget(this);
    drawButton();
}


void Button::treatFocusOut()
{
    hasFocus = false;
    requestNotToBeActualDefaultButtonWidget(this);
    drawButton();
}
void Button::emulateButtonPress()
{
    bool oldIsButtonPressed = isButtonPressed;
    isButtonPressed = true;
    drawButton();
    XFlush(getDisplay()); waitShort();
    isButtonPressed = false;
    drawButton();
    XFlush(getDisplay()); waitShort();
    isButtonPressed = oldIsButtonPressed;
    drawButton();
    XFlush(getDisplay()); waitShort();
    pressedCallback.call(this);    
}
