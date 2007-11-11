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

#include <X11/keysym.h>

#include "CheckBox.hpp"
#include "GuiRoot.hpp"
#include "TopWin.hpp"
#include "GlobalConfig.hpp"

using namespace LucED;

const int BUTTON_OUTER_BORDER = 1;

CheckBox::CheckBox(GuiWidget* parent, String buttonText)
      : GuiWidget(parent, 0, 0, 1, 1, 0),
        position(0, 0, 1, 1),
        isBoxChecked(false),
        isMouseButtonPressed(false),
        isMouseOverButton(false),
        hasFocus(false),
        hasHotKey(false),
        showHotKey(false)
{
    addToXEventMask(ExposureMask|ButtonPressMask|ButtonReleaseMask|ButtonMotionMask|EnterWindowMask|LeaveWindowMask);
    setBackgroundColor(getGuiRoot()->getGuiColor03());
    int p1 = buttonText.findFirstOf(']', 1);
    if (p1 != -1) {
        hotKeyChar = buttonText[p1 - 1];
        this->buttonText = String() << buttonText.getSubstring(0, p1) << buttonText.getTail(p1 + 1);
        hotKeyPixX = getGuiTextStyle()->getTextWidth(buttonText.getSubstring(0, p1 - 1));
        hotKeyPixW = getGuiTextStyle()->getCharWidth(hotKeyChar);
        hasHotKey = true;
        // showHotKey = true;
        String keySymString;
        keySymString.appendLowerChar(hotKeyChar);
        requestHotKeyRegistrationFor(KeyMapping::Id(KeyModifier("Alt"), KeyId(keySymString)), this);
    } else {
        this->buttonText = buttonText;
    }
}

GuiElement::Measures CheckBox::getDesiredMeasures()
{
    int guiSpacing = GlobalConfig::getInstance()->getGuiSpacing();

    int buttonInnerSpacing = GlobalConfig::getInstance()->getButtonInnerSpacing();
    int minWidth = getGuiTextStyle()->getLineHeight() * 2;
    int minHeight = getGuiTextHeight() + 2 * getRaisedBoxBorderWidth()  + 2*buttonInnerSpacing + 2*BUTTON_OUTER_BORDER + guiSpacing;

    int bestWidth = (getGuiTextStyle()->getTextWidth(buttonText) + 2 * getRaisedBoxBorderWidth() + 2*buttonInnerSpacing + 2*BUTTON_OUTER_BORDER + guiSpacing) 
                  + 4 * getGuiTextStyle()->getSpaceWidth() + 2*BUTTON_OUTER_BORDER + guiSpacing;
    int bestHeight = minHeight;

    return Measures(minWidth, minHeight, bestWidth, bestHeight, bestWidth, bestHeight);
}


void CheckBox::setPosition(Position newPosition)
{
    if (position != newPosition) {
        GuiWidget::setPosition(newPosition);
        this->position = newPosition;
    }
}

void CheckBox::draw()
{
    int guiSpacing = GlobalConfig::getInstance()->getGuiSpacing();

    int textOffset = 0;
    int w = getGuiTextStyle()->getTextWidth(buttonText);
    int x = (position.w - getGuiTextStyle()->getLineHeight() - 2*BUTTON_OUTER_BORDER - w - guiSpacing) / 2 + BUTTON_OUTER_BORDER + guiSpacing + getGuiTextStyle()->getLineHeight();
    if (x < BUTTON_OUTER_BORDER + guiSpacing + getGuiTextStyle()->getLineHeight()) { x = BUTTON_OUTER_BORDER + guiSpacing + getGuiTextStyle()->getLineHeight(); }
    int y = (position.h - 2*BUTTON_OUTER_BORDER - getGuiTextHeight() - guiSpacing) / 2 + BUTTON_OUTER_BORDER + guiSpacing;
    if (y < BUTTON_OUTER_BORDER + guiSpacing) { y = BUTTON_OUTER_BORDER + guiSpacing; }

    GuiColor color;
    if (isMouseOverButton) {
        color = GuiRoot::getInstance()->getGuiColor04();
    } else {
        color = GuiRoot::getInstance()->getGuiColor03();
    }
    {
//        drawRaisedBox(BUTTON_OUTER_BORDER + guiSpacing, BUTTON_OUTER_BORDER + guiSpacing, 
//                position.w - 2*BUTTON_OUTER_BORDER - guiSpacing, position.h - 2*BUTTON_OUTER_BORDER - guiSpacing, color);
        drawRaisedSurface(BUTTON_OUTER_BORDER + guiSpacing, BUTTON_OUTER_BORDER + guiSpacing, 
                position.w - 2*BUTTON_OUTER_BORDER - guiSpacing, position.h - 2*BUTTON_OUTER_BORDER - guiSpacing, GuiRoot::getInstance()->getGuiColor03());

        int bx = BUTTON_OUTER_BORDER + guiSpacing + 3;
        int by = y + 1; // +2
        int bw = getGuiTextStyle()->getLineHeight() - 1; // -2
        drawPressedBox(bx, by, bw, bw, color);
        if (isBoxChecked) {
            int cx = bx + 3;
            int cy = by + 3;
            int cw = bw - 7;
            int cw1 = cw / 3;
            int cw2 = cw - cw1;
            int ch  = cw;
            int ch1 = cw1;
            int ch2 = ch - ch1;
            
            drawLine(cx,       cy +     ch1,      cw1,       ch1, GuiRoot::getInstance()->getGuiColor01());
            drawLine(cx + cw1, cy + 2 * ch1,  2 * cw1, - 2 * ch1, GuiRoot::getInstance()->getGuiColor01());

            drawLine(cx,       cy + 1 +     ch1,      cw1,       ch1, GuiRoot::getInstance()->getGuiColor01());
            drawLine(cx + cw1, cy + 1 + 2 * ch1,  2 * cw1, - 2 * ch1, GuiRoot::getInstance()->getGuiColor01());
        }
    }
    const int d = BUTTON_OUTER_BORDER - 1;
    undrawFrame(d + guiSpacing, d + guiSpacing, position.w - 2 * d - guiSpacing, position.h - 2 * d - guiSpacing);
    if (hasFocus) {
        const int d = BUTTON_OUTER_BORDER;
        drawDottedFrame(d + guiSpacing, d + guiSpacing + 1, position.w - 2 * d - 1 - guiSpacing, position.h - 2 * d - 1 - guiSpacing);
    }
    if (showHotKey) {
        //drawLine(x + textOffset + hotKeyPixX, y + textOffset + getGuiTextHeight(), hotKeyPixW, 0);
        int lineY = getGuiTextStyle()->getLineAscent() + 1;
        if (lineY > getGuiTextHeight() - 1) {
            lineY = getGuiTextHeight() - 1;
        }
        drawLine(x + textOffset + hotKeyPixX, y + textOffset + lineY, hotKeyPixW, 0);
    }
    drawGuiText(x + textOffset, y + textOffset, buttonText);
}


bool CheckBox::isMouseInsideButtonArea(int mouseX, int mouseY)
{
    int x = mouseX;
    int y = mouseY;
    return (x >= BUTTON_OUTER_BORDER - 1 && x <= position.w - 2*BUTTON_OUTER_BORDER + 1
                            && y >= BUTTON_OUTER_BORDER - 1 && y <= position.h - 2*BUTTON_OUTER_BORDER + 1);
}

static const MicroSeconds shortTime = MicroSeconds(50 * 1000);

static void waitShort(MicroSeconds microSecs = shortTime)
{
    if (microSecs > 0) {
        TimeVal timeVal;
        timeVal.add(microSecs);
        LucED::select(0, NULL, NULL, NULL, &timeVal);
    }
}

GuiElement::ProcessingResult CheckBox::processEvent(const XEvent *event)
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
                draw();
                return EVENT_PROCESSED;
            }

            case ButtonPress: {
                if (event->xbutton.button == Button1)
                {
                    earliestButtonReleaseTime.setToCurrentTime().add(shortTime);
                    
                    isMouseButtonPressed = true;
                    int x = event->xbutton.x;
                    int y = event->xbutton.y;
                    
                    if (isMouseInsideButtonArea(x, y))
                    {
                        isBoxChecked = !isBoxChecked;
                    } else {
                    }
                    draw();
                    return EVENT_PROCESSED;
                }
                break;
            }

            case ButtonRelease: {
                isMouseButtonPressed = false;
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
                    draw();
                }
                return EVENT_PROCESSED;
            }
            
            case EnterNotify: {
                int x = event->xcrossing.x;
                int y = event->xcrossing.y;
                if (isMouseInsideButtonArea(x, y) && !isMouseOverButton) {
                    isMouseOverButton = true;
                    draw();
                }            
                addToXEventMask(PointerMotionMask);
                return EVENT_PROCESSED;
            }
            
            case LeaveNotify: {
                if (isMouseOverButton) {
                    isMouseOverButton = false;
                    draw();
                }
                removeFromXEventMask(PointerMotionMask);
                return EVENT_PROCESSED;
            }
        }
        return propagateEventToParentWidget(event);
    }
}


GuiElement::ProcessingResult CheckBox::processKeyboardEvent(const XEvent *event)
{
    KeyId       pressedKey  = KeyId(XLookupKeysym((XKeyEvent*)&event->xkey, 0));
    KeyModifier keyModifier = KeyModifier(event->xkey.state);

    bool processed = false;
    KeyMapping::Id keyMappingId(keyModifier, pressedKey);
    if (KeyMapping::Id(0, KeyId("space")) == keyMappingId) {
        isBoxChecked = !isBoxChecked;
        draw();
        processed = true;
    }
    return processed ? EVENT_PROCESSED : NOT_PROCESSED;
}


void CheckBox::treatLostHotKeyRegistration(const KeyMapping::Id& id)
{
    if (showHotKey) {
        showHotKey = false;
        draw();
    }
}


void CheckBox::treatNewHotKeyRegistration(const KeyMapping::Id& id)
{
    if (!showHotKey) {
        showHotKey = true;
        draw();
    }
}


void CheckBox::treatFocusIn()
{
    if (!hasFocus) {
        hasFocus = true;
        draw();
    }
}


void CheckBox::treatFocusOut()
{
    hasFocus = false;
    draw();
}


void CheckBox::treatHotKeyEvent(const KeyMapping::Id& id)
{
    isBoxChecked = !isBoxChecked;
    draw();
}

void CheckBox::setChecked(bool checked)
{
    if (checked != isBoxChecked) {
        isBoxChecked = checked;
        draw();
    }
}

bool CheckBox::isChecked() const
{
    return isBoxChecked;
}

