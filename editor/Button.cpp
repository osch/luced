/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2009 Oliver Schmidt, oliver at luced dot de
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
#include "GlobalConfig.hpp"
#include "System.hpp"

using namespace LucED;

const int BUTTON_OUTER_BORDER = 1;

Button::Button(String buttonText)
      : isButtonPressed(false),
        isMouseButtonPressed(false),
        isMouseOverButton(false),
        isDefaultButton(false),
        hasHotKeyFlag(false),
        showHotKeyFlag(false),
        isExplicitDefaultButton(false),
        hasForcedMeasuresFlag(false)
{
    KeyActionHandler::Ptr keyActionHandler = KeyActionHandler::create();
    keyActionHandler->addActionMethods(Actions::create(this));
    setKeyActionHandler(keyActionHandler);
    setButtonText(buttonText);
}

void Button::setButtonText(String buttonText)
{
    if (buttonText != this->buttonText)
    {
        if (hasHotKeyFlag) {
            String keySymString;
            keySymString.appendUpperChar(hotKeyChar);
            requestRemovalOfHotKeyRegistration(KeyMapping::Id(KeyModifier("Alt"), KeyId(keySymString)));
            hasHotKeyFlag = false;
        }
        buttonTextWChars.setToUtf8String(buttonText);
        int p1 = buttonTextWChars.findFirstOf(']', Pos(1));
        
        if (p1 > 0) {
            int hotKeyWChar = buttonTextWChars[p1 - 1].toInt();
            if (0 <= hotKeyWChar && hotKeyWChar <= 0xff)
            {
                hotKeyChar = hotKeyWChar;
                buttonTextWChars.removeAmount(p1, 1);
                hotKeyPixX = GuiWidget::getGuiTextStyle()->getTextWidth(buttonTextWChars.getPtr(0), p1 - 1);
                hotKeyPixW = GuiWidget::getGuiTextStyle()->getCharWidth(hotKeyChar);
                hasHotKeyFlag = true;
                // showHotKeyFlag = true;
                String keySymString;
                keySymString.appendUpperChar(hotKeyChar);
                requestHotKeyRegistration(KeyMapping::Id(KeyModifier("Alt"), KeyId(keySymString)));
            }
        }
        if (isVisible()) {
            drawButton();
        }
        this->buttonText = buttonText;
    }
}

int Button::getStandardHeight()
{
    int guiSpacing = GlobalConfig::getInstance()->getGuiSpacing();
    int buttonInnerSpacing = GlobalConfig::getInstance()->getButtonInnerSpacing();

    return GuiWidget::getGuiTextHeight() + 2 * GuiWidget::getRaisedBoxBorderWidth()  + 2*buttonInnerSpacing + 2*BUTTON_OUTER_BORDER + guiSpacing;
}

void Button::setDesiredMeasures(Measures m)
{
    hasForcedMeasuresFlag = true;
    forcedMeasures = m;
}

GuiElement::Measures Button::internalGetDesiredMeasures()
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
    int minWidth  = GuiWidget::getGuiTextStyle()->getTextWidth(buttonTextWChars) + 2 * GuiWidget::getRaisedBoxBorderWidth() + 2*buttonInnerSpacing + 2*BUTTON_OUTER_BORDER + guiSpacing;
    int minHeight = GuiWidget::getGuiTextHeight() + 2 * GuiWidget::getRaisedBoxBorderWidth()  + 2*buttonInnerSpacing + 2*BUTTON_OUTER_BORDER + guiSpacing;

    int bestWidth = minWidth + 4 * GuiWidget::getGuiTextStyle()->getSpaceWidth() + 2*BUTTON_OUTER_BORDER + guiSpacing;
    int bestHeight = minHeight;

    return Measures(minWidth, minHeight, bestWidth, bestHeight, bestWidth, bestHeight);
}


void Button::drawButton()
{
    if (getGuiWidget().isValid())
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
            getGuiWidget()->drawPressedBox(BUTTON_OUTER_BORDER + guiSpacing, BUTTON_OUTER_BORDER + guiSpacing, 
                    getPosition().w - 2*BUTTON_OUTER_BORDER - guiSpacing, getPosition().h - 2*BUTTON_OUTER_BORDER - guiSpacing, color);
            textOffset = 1;
        } else {
            getGuiWidget()->drawRaisedBox(BUTTON_OUTER_BORDER + guiSpacing, BUTTON_OUTER_BORDER + guiSpacing, 
                    getPosition().w - 2*BUTTON_OUTER_BORDER - guiSpacing, getPosition().h - 2*BUTTON_OUTER_BORDER - guiSpacing, color);
        }
        if (isDefaultButton) {
            const int d = BUTTON_OUTER_BORDER - 1;
            getGuiWidget()->drawFrame(d + guiSpacing, d + guiSpacing, getPosition().w - 2 * d - guiSpacing, getPosition().h - 2 * d - guiSpacing);
        } else {
            const int d = BUTTON_OUTER_BORDER - 1;
            getGuiWidget()->undrawFrame(d + guiSpacing, d + guiSpacing, getPosition().w - 2 * d - guiSpacing, getPosition().h - 2 * d - guiSpacing);
        }
        if (hasFocus()) {
            const int d = BUTTON_OUTER_BORDER + 1;
            if (isButtonPressed) {
                getGuiWidget()->drawDottedFrame(d+1 + guiSpacing, d+1 + guiSpacing, getPosition().w - 2 * d - 1 - guiSpacing, getPosition().h - 2 * d - 1 - guiSpacing);
            } else {
                getGuiWidget()->drawDottedFrame(d + guiSpacing, d + guiSpacing, getPosition().w - 2 * d - 1 - guiSpacing, getPosition().h - 2 * d - 1 - guiSpacing);
            }
        }
        int w = GuiWidget::getGuiTextStyle()->getTextWidth(buttonTextWChars);
        int x = (getPosition().w - 2*BUTTON_OUTER_BORDER - w - guiSpacing) / 2 + BUTTON_OUTER_BORDER + guiSpacing;
        if (x < BUTTON_OUTER_BORDER + guiSpacing) { x = BUTTON_OUTER_BORDER + guiSpacing; }
        int y = (getPosition().h - 2*BUTTON_OUTER_BORDER - GuiWidget::getGuiTextHeight() - guiSpacing) / 2 + BUTTON_OUTER_BORDER + guiSpacing;
        if (y < BUTTON_OUTER_BORDER + guiSpacing) { y = BUTTON_OUTER_BORDER + guiSpacing; }
        if (showHotKeyFlag) {
            //drawLine(x + textOffset + hotKeyPixX, y + textOffset + getGuiTextHeight(), hotKeyPixW, 0);
            int lineY = GuiWidget::getGuiTextStyle()->getLineAscent() + 1;
            if (lineY > GuiWidget::getGuiTextHeight() - 1) {
                lineY = GuiWidget::getGuiTextHeight() - 1;
            }
            getGuiWidget()->drawLine(x + textOffset + hotKeyPixX, y + textOffset + lineY, hotKeyPixW, 0);
        }
        getGuiWidget()->drawGuiTextWChars(x + textOffset, y + textOffset, buttonTextWChars);
    }
}


bool Button::isMouseInsideButtonArea(int mouseX, int mouseY)
{
    int x = mouseX;
    int y = mouseY;
    return (x >= BUTTON_OUTER_BORDER - 1 && x <= getPosition().w - 2*BUTTON_OUTER_BORDER + 1
                            && y >= BUTTON_OUTER_BORDER - 1 && y <= getPosition().h - 2*BUTTON_OUTER_BORDER + 1);
}

static const MicroSeconds shortTime = MicroSeconds(20 * 1000);

static void waitShort(MicroSeconds microSecs = shortTime)
{
    if (microSecs > 0) {
        TimeVal timeVal(microSecs);
        System::select(0, NULL, NULL, NULL, &timeVal);
    }
}


GuiWidget::ProcessingResult Button::processGuiWidgetEvent(const XEvent *event)
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
            drawButton();
            return GuiWidget::EVENT_PROCESSED;
        }

        case ButtonPress: {
            if (!hasFocus()) {
                reportMouseClick();
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
                return GuiWidget::EVENT_PROCESSED;
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
                XSync(getGuiWidget()->getDisplay(), False);
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
            return GuiWidget::EVENT_PROCESSED;
        }

        case MotionNotify:
        {
            bool mustDraw = false;
            XEvent newEvent;

            XSync(getGuiWidget()->getDisplay(), False);
            if (XCheckWindowEvent(getGuiWidget()->getDisplay(), getGuiWidget()->getWid(), ButtonMotionMask|PointerMotionMask, &newEvent) == True) {
                event = &newEvent;
                while (XCheckWindowEvent(getGuiWidget()->getDisplay(), getGuiWidget()->getWid(), ButtonMotionMask|PointerMotionMask, &newEvent) == True);
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
            return GuiWidget::EVENT_PROCESSED;
        }
        
        case EnterNotify: {
            int x = event->xcrossing.x;
            int y = event->xcrossing.y;
            if (isMouseInsideButtonArea(x, y) && !isMouseOverButton) {
                isMouseOverButton = true;
                drawButton();
            }
            getGuiWidget()->addToXEventMask(PointerMotionMask);
            return GuiWidget::EVENT_PROCESSED;
        }
        
        case LeaveNotify: {
            if (isMouseOverButton) {
                isMouseOverButton = false;
                drawButton();
            }
            getGuiWidget()->removeFromXEventMask(PointerMotionMask);
            return GuiWidget::EVENT_PROCESSED;
        }
    }
    return getGuiWidget()->propagateEventToParentWidget(event);
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
        requestHotKeyRegistration(KeyMapping::Id(KeyModifier(), KeyId("Return")));
    }
    if (!hasFocus()) {
        BaseClass::treatFocusIn();
        drawButton();
    }
}


void Button::treatFocusOut()
{
    BaseClass::treatFocusOut();
    if (!isExplicitDefaultButton) {
        requestRemovalOfHotKeyRegistration(KeyMapping::Id(KeyModifier(), KeyId("Return")));
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
    XSync(getGuiWidget()->getDisplay(), False); waitShort();
    isButtonPressed = false;
    drawButton();
    XSync(getGuiWidget()->getDisplay(), False); waitShort();
    isButtonPressed = oldIsButtonPressed;
    drawButton();
    XSync(getGuiWidget()->getDisplay(), False); waitShort();

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
            requestHotKeyRegistration(KeyMapping::Id(KeyModifier(), KeyId("Return")));
            isExplicitDefaultButton = true;
        } else {
            isExplicitDefaultButton = false;
        }
        drawButton();
    }
}

void Button::processGuiWidgetCreatedEvent()
{
    getGuiWidget()->addToXEventMask(ExposureMask|ButtonPressMask|ButtonReleaseMask|ButtonMotionMask|EnterWindowMask|LeaveWindowMask);
    getGuiWidget()->setBackgroundColor(GuiRoot::getInstance()->getGuiColor03());
}


