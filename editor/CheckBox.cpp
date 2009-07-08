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
#include "GlobalConfig.hpp"

using namespace LucED;

const int BUTTON_OUTER_BORDER = 1;

CheckBox::CheckBox(String buttonText)
      : buttonText(buttonText),
        isBoxChecked(false),
        isMouseButtonPressed(false),
        isMouseOverButton(false),
        hasHotKey(false),
        showHotKey(false)
{
    KeyActionHandler::Ptr keyActionHandler = KeyActionHandler::create();
    keyActionHandler->addActionMethods(Actions::create(this));
    setKeyActionHandler(keyActionHandler);
}


GuiElement::Measures CheckBox::internalGetDesiredMeasures()
{
    int guiSpacing = GlobalConfig::getInstance()->getGuiSpacing();

    int buttonInnerSpacing = GlobalConfig::getInstance()->getButtonInnerSpacing();
    int minWidth  = GuiWidget::getGuiTextStyle()->getLineHeight() * 2;
    int minHeight = GuiWidget::getGuiTextHeight() + 2 * GuiWidget::getRaisedBoxBorderWidth()  + 2*buttonInnerSpacing + 2*BUTTON_OUTER_BORDER + guiSpacing;

    int bestWidth = (GuiWidget::getGuiTextStyle()->getTextWidth(buttonText) + 2 * GuiWidget::getRaisedBoxBorderWidth() + 2*buttonInnerSpacing + 2*BUTTON_OUTER_BORDER + guiSpacing) 
                  + 4 * GuiWidget::getGuiTextStyle()->getSpaceWidth() + 2*BUTTON_OUTER_BORDER + guiSpacing;
    int bestHeight = minHeight;

    return Measures(minWidth, minHeight, bestWidth, bestHeight, bestWidth, bestHeight);
}



void CheckBox::draw()
{
    int guiSpacing = GlobalConfig::getInstance()->getGuiSpacing();

    int textOffset = 0;
    int w = GuiWidget::getGuiTextStyle()->getTextWidth(buttonText);
    int x = (getPosition().w - GuiWidget::getGuiTextStyle()->getLineHeight() - 2*BUTTON_OUTER_BORDER - w - guiSpacing) / 2 + BUTTON_OUTER_BORDER + guiSpacing + GuiWidget::getGuiTextStyle()->getLineHeight();
    if (x < BUTTON_OUTER_BORDER + guiSpacing + GuiWidget::getGuiTextStyle()->getLineHeight()) { x = BUTTON_OUTER_BORDER + guiSpacing + GuiWidget::getGuiTextStyle()->getLineHeight(); }
    int y = (getPosition().h - 2*BUTTON_OUTER_BORDER - GuiWidget::getGuiTextHeight() - guiSpacing) / 2 + BUTTON_OUTER_BORDER + guiSpacing;
    if (y < BUTTON_OUTER_BORDER + guiSpacing) { y = BUTTON_OUTER_BORDER + guiSpacing; }

    GuiColor color;
    if (isMouseOverButton) {
        color = GuiRoot::getInstance()->getGuiColor04();
    } else {
        color = GuiRoot::getInstance()->getGuiColor03();
    }
    {
//        drawRaisedBox(BUTTON_OUTER_BORDER + guiSpacing, BUTTON_OUTER_BORDER + guiSpacing, 
//                getPosition().w - 2*BUTTON_OUTER_BORDER - guiSpacing, getPosition().h - 2*BUTTON_OUTER_BORDER - guiSpacing, color);
        getGuiWidget()->drawRaisedSurface(BUTTON_OUTER_BORDER + guiSpacing, BUTTON_OUTER_BORDER + guiSpacing, 
                getPosition().w - 2*BUTTON_OUTER_BORDER - guiSpacing, getPosition().h - 2*BUTTON_OUTER_BORDER - guiSpacing, GuiRoot::getInstance()->getGuiColor03());

        int bx = BUTTON_OUTER_BORDER + guiSpacing + 3;
        int by = y + 1; // +2
        int bw = GuiWidget::getGuiTextStyle()->getLineHeight() - 1; // -2
        getGuiWidget()->drawPressedBox(bx, by, bw, bw, color);
        if (isBoxChecked) {
            int cx = bx + 3;
            int cy = by + 3;
            int cw = bw - 7;
            int cw1 = cw / 3;
            int cw2 = cw - cw1;
            int ch  = cw;
            int ch1 = cw1;
            int ch2 = ch - ch1;
            
            getGuiWidget()->drawLine(cx,       cy +     ch1,      cw1,       ch1, GuiRoot::getInstance()->getGuiColor01());
            getGuiWidget()->drawLine(cx + cw1, cy + 2 * ch1,  2 * cw1, - 2 * ch1, GuiRoot::getInstance()->getGuiColor01());

            getGuiWidget()->drawLine(cx,       cy + 1 +     ch1,      cw1,       ch1, GuiRoot::getInstance()->getGuiColor01());
            getGuiWidget()->drawLine(cx + cw1, cy + 1 + 2 * ch1,  2 * cw1, - 2 * ch1, GuiRoot::getInstance()->getGuiColor01());
        }
    }
    const int d = BUTTON_OUTER_BORDER - 1;
    getGuiWidget()->undrawFrame(d + guiSpacing, d + guiSpacing, getPosition().w - 2 * d - guiSpacing, getPosition().h - 2 * d - guiSpacing);
    if (hasFocus()) {
        const int d = BUTTON_OUTER_BORDER;
        getGuiWidget()->drawDottedFrame(d + guiSpacing, d + guiSpacing + 1, getPosition().w - 2 * d - 1 - guiSpacing, getPosition().h - 2 * d - 1 - guiSpacing);
    }
    if (showHotKey) {
        //drawLine(x + textOffset + hotKeyPixX, y + textOffset + GuiWidget::getGuiTextHeight(), hotKeyPixW, 0);
        int lineY = GuiWidget::getGuiTextStyle()->getLineAscent() + 1;
        if (lineY > GuiWidget::getGuiTextHeight() - 1) {
            lineY = GuiWidget::getGuiTextHeight() - 1;
        }
        getGuiWidget()->drawLine(x + textOffset + hotKeyPixX, y + textOffset + lineY, hotKeyPixW, 0);
    }
    getGuiWidget()->drawGuiText(x + textOffset, y + textOffset, buttonText);
}


bool CheckBox::isMouseInsideButtonArea(int mouseX, int mouseY)
{
    int x = mouseX;
    int y = mouseY;
    return (x >= BUTTON_OUTER_BORDER - 1 && x <= getPosition().w - 2*BUTTON_OUTER_BORDER + 1
                            && y >= BUTTON_OUTER_BORDER - 1 && y <= getPosition().h - 2*BUTTON_OUTER_BORDER + 1);
}


GuiWidget::ProcessingResult CheckBox::processGuiWidgetEvent(const XEvent* event)
{
    switch (event->type) 
    {
        case GraphicsExpose:
            if (event->xgraphicsexpose.count > 0) {
                break;
            }
        case Expose: {
            if (event->xexpose.count > 0) {
                break;
            }
            draw();
            return GuiWidget::EVENT_PROCESSED;
        }

        case ButtonPress: {
            if (!hasFocus()) {
                reportMouseClick();
            }
            if (event->xbutton.button == Button1)
            {
                isMouseButtonPressed = true;
                int x = event->xbutton.x;
                int y = event->xbutton.y;
                
                if (isMouseInsideButtonArea(x, y))
                {
                    isBoxChecked = !isBoxChecked;
                    draw();
                    pressedCallback->call(this);
                } else {
                    draw();
                }
                return GuiWidget::EVENT_PROCESSED;
            }
            break;
        }

        case ButtonRelease: {
            isMouseButtonPressed = false;
            return GuiWidget::EVENT_PROCESSED;
        }

        case MotionNotify:
        {
            bool mustDraw = false;
            XEvent newEvent;

            XSync(getDisplay(), False);
            if (XCheckWindowEvent(getDisplay(), getGuiWidget()->getWid(), ButtonMotionMask|PointerMotionMask, &newEvent) == True) {
                event = &newEvent;
                while (XCheckWindowEvent(getDisplay(), getGuiWidget()->getWid(), ButtonMotionMask|PointerMotionMask, &newEvent) == True);
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
            return GuiWidget::EVENT_PROCESSED;
        }
        
        case EnterNotify: {
            int x = event->xcrossing.x;
            int y = event->xcrossing.y;
            if (isMouseInsideButtonArea(x, y) && !isMouseOverButton) {
                isMouseOverButton = true;
                draw();
            }            
            getGuiWidget()->addToXEventMask(PointerMotionMask);
            return GuiWidget::EVENT_PROCESSED;
        }
        
        case LeaveNotify: {
            if (isMouseOverButton) {
                isMouseOverButton = false;
                draw();
            }
            getGuiWidget()->removeFromXEventMask(PointerMotionMask);
            return GuiWidget::EVENT_PROCESSED;
        }
    }
    return getGuiWidget()->propagateEventToParentWidget(event);
}

void CheckBox::processGuiWidgetCreatedEvent()
{
    getGuiWidget()->addToXEventMask(ExposureMask|ButtonPressMask|ButtonReleaseMask|ButtonMotionMask|EnterWindowMask|LeaveWindowMask);
    getGuiWidget()->setBackgroundColor(getGuiRoot()->getGuiColor03());

    int p1 = buttonText.findFirstOf(']', Pos(1));
    if (p1 != -1) {
        hotKeyChar = buttonText[p1 - 1];
        this->buttonText = String() << buttonText.getHead(p1) << buttonText.getTail(p1 + 1);
        hotKeyPixX = GuiWidget::getGuiTextStyle()->getTextWidth(buttonText.getHead(p1 - 1));
        hotKeyPixW = GuiWidget::getGuiTextStyle()->getCharWidth(hotKeyChar);
        hasHotKey = true;
        // showHotKey = true;
        String keySymString;
        keySymString.appendUpperChar(hotKeyChar);
        requestHotKeyRegistration(KeyMapping::Id(KeyModifier("Alt"), KeyId(keySymString)));
    }
    
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
    if (!hasFocus()) {
        setFocusFlag(true);
        draw();
    }
}


void CheckBox::treatFocusOut()
{
    setFocusFlag(false);
    draw();
}


void CheckBox::treatHotKeyEvent(const KeyMapping::Id& id)
{
    isBoxChecked = !isBoxChecked;
    draw();
    pressedCallback->call(this);
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

