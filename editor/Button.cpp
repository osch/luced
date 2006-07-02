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

using namespace LucED;

#define BUTTON_BORDER 2

Button::Button(GuiWidget* parent, string buttonText)
      : GuiWidget(parent, 0, 0, 1, 1, 0),
        position(0, 0, 1, 1),
        buttonText(buttonText),
        isButtonPressed(false),
        isMouseButtonPressed(false),
        isMouseOverButton(false),
        isDefaultButton(false)
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

bool Button::processEvent(const XEvent *event)
{
    if (GuiWidget::processEvent(event)) {
        return true;
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
                return true;
            }

            case ButtonPress: {
                if (event->xbutton.button == Button1)
                {
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
                    return true;
                }
                break;
            }

            case ButtonRelease: {
                isMouseButtonPressed = false;
                if (isButtonPressed) {
                    isButtonPressed = false;
                    drawButton();
                    int x = event->xbutton.x;
                    int y = event->xbutton.y;
                    if (isMouseInsideButtonArea(x, y)) {
                        pressedCallback.call(this);
                    }
                }
                return true;
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
                return true;
            }
            
            case EnterNotify: {
                int x = event->xcrossing.x;
                int y = event->xcrossing.y;
                if (isMouseInsideButtonArea(x, y) && !isMouseOverButton) {
                    isMouseOverButton = true;
                    drawButton();
                }            
                addToXEventMask(PointerMotionMask);
                return true;
            }
            
            case LeaveNotify: {
                if (isMouseOverButton) {
                    isMouseOverButton = false;
                    drawButton();
                }
                removeFromXEventMask(PointerMotionMask);
                return true;
            }
        }
        return propagateEventToParentWidget(event);
    }
}


