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

#include "ScrollBar.h"
#include "GuiRoot.h"
#include "util.h"
#include "EventDispatcher.h"
#include "GlobalConfig.h"

using namespace LucED;


static GC scrollBar_gcid;
static bool scrollBarInitialized = false;


void ScrollBar::initStatically()
{
    Display* display = getDisplay();
    GuiRoot* guiRoot = getGuiRoot();

    scrollBar_gcid = XCreateGC(display, guiRoot->getRootWid(), 0, NULL);

    XSetForeground(display, scrollBar_gcid, guiRoot->getBlackColor());
    XSetBackground(display, scrollBar_gcid, guiRoot->getGreyColor());

    XSetLineAttributes(display, scrollBar_gcid, 0, 
        LineSolid, CapProjecting, JoinMiter);

    XSetGraphicsExposures(display, scrollBar_gcid, True);
}


ScrollBar::ScrollBar(GuiWidget* parent,
            int x, int y, unsigned int width, unsigned int height)
    : GuiWidget(parent, x, y, width, height, 0),
      position(x, y, width, height),
      
      slotForSetValue(this, &ScrollBar::setValue),
      slotForSetValueRange(this, &ScrollBar::setValueRange),
      slotForScrollStepRepeating(this, &ScrollBar::handleScrollStepRepeating)
{
    totalValue = 0;
    originalTotalValue = 0;
    heightValue = 0;
    value = 0;
    movingBar = false;
    movingYOffset = 0;
    isButtonPressedForScrollStep = false;
    addToXEventMask(ExposureMask|ButtonPressMask|ButtonReleaseMask|ButtonMotionMask);
    calculateValuesFromPosition();

    if (!scrollBarInitialized) {
        initStatically();
        scrollBarInitialized = true;
    }
    setBackgroundColor(getGuiRoot()->getGuiColor02());
 }


bool ScrollBar::processEvent(const XEvent *event)
{
    if (GuiWidget::processEvent(event)) {
        return true;
    } else {
        
        switch (event->type) {
            
            case GraphicsExpose:
            case Expose: {
                this->drawArrows();
                this->drawArea();
                return true;
            }

            case ButtonPress: {
                if (event->xbutton.button == Button1)
                {
                    int y;
                    int h;
                    isButtonPressedForScrollStep = false;
                    if (this->isV) {
                        y = event->xbutton.y;
                        h = position.h;
                    } else {
                        y = event->xbutton.x;
                        h = position.w;
                    }
                    if ((y >= this->scrollY + arrowLength) 
                            && (y < this->scrollY + arrowLength + this->scrollHeight)) {
                        //printf("Button pressed\n");
                        this->movingBar = true;
                        this->movingYOffset = y - this->scrollY;
                    }
                    else if (0 <= y && y < arrowLength) {
                        isButtonPressedForScrollStep = true;
                        drawPressedUpButton();
                        scrollStepCallback.call(scrollStep = ScrollStep::LINE_UP);
                    }
                    else if (h - arrowLength <= y && y < h) {
                        isButtonPressedForScrollStep = true;
                        drawPressedDownButton();
                        scrollStepCallback.call(scrollStep = ScrollStep::LINE_DOWN);
                    }
                    else if (arrowLength <= y && y < this->scrollY + arrowLength) {
                        isButtonPressedForScrollStep = true;
                        scrollStepCallback.call(scrollStep = ScrollStep::PAGE_UP);
                    }
                    else if (this->scrollY + arrowLength + scrollHeight <= y && y < h - arrowLength) {
                        isButtonPressedForScrollStep = true;
                        scrollStepCallback.call(scrollStep = ScrollStep::PAGE_DOWN);
                    }
                    if (isButtonPressedForScrollStep) {
                        EventDispatcher::getInstance()->registerTimerCallback(0, 
                                GlobalConfig::getInstance()->getScrollBarRepeatFirstMicroSecs(),
                                slotForScrollStepRepeating);
                    }
                    return true;
                }
                break;
            }

            case ButtonRelease: {
                if (this->movingBar) {
                    //printf("ButtonReleased\n");
                    this->movingBar = false;
                    return true;
                }
                if (isButtonPressedForScrollStep) {
                    if (scrollStep == ScrollStep::LINE_UP) {
                        drawUpButton();
                    } else if (scrollStep == ScrollStep::LINE_DOWN) {
                        drawDownButton();
                    }
                    isButtonPressedForScrollStep = false;
                    return true;
                }
                break;
            }

            case MotionNotify: {
                if (this->movingBar) {
                    int newY;
                    XEvent newEvent;

                    XFlush(getDisplay());
                    if (XCheckWindowEvent(getDisplay(), getWid(), ButtonMotionMask, &newEvent) == True) {
                        event = &newEvent;
                        while (XCheckWindowEvent(getDisplay(), getWid(), ButtonMotionMask, &newEvent) == True);
                    }

                    if (this->isV) {
                        newY = event->xmotion.y - this->movingYOffset;
                    } else {
                        newY = event->xmotion.x - this->movingYOffset;
                    }
                    if (newY < 0) 
                        newY = 0;
                    else if (newY > calcHighestScrollY())
                        newY = calcHighestScrollY();
                    //printf("newY %d (%d, %d) %d %d\n", newY, this->scrollY, highestScrollY(), this->scrollHeight, getWid()getHeight);
                    if (newY != this->scrollY) {
                        long newValue;
                        this->scrollY = newY;
                        newValue = calcValue();

                        this->totalValue = this->originalTotalValue;
                        this->scrollHeight = calcScrollHeight();
                        if (this->scrollY + this->scrollHeight > scrollAreaLength) {
                            this->scrollHeight = scrollAreaLength - this->scrollY;
                        }
                        if (newValue + this->heightValue > this->totalValue) {
                            this->totalValue = newValue + this->heightValue;
                        }
            
                        drawArea();
                        if (newValue != this->value) {
                            //printf("Motion %d\n", newValue);
                            //ScrollBarV_setValue(, newValue);
                            this->value = newValue;
                            this->changedValueCallback.call(newValue);
                        }
                    }
                }
                return true;
            }
        }
        return propagateEventToParentWidget(event);
    }
}
    
void ScrollBar::setPosition(Position newPosition)
{
    if (position != newPosition) {
        GuiWidget::setPosition(newPosition);
        this->position = newPosition;
        calculateValuesFromPosition();
        drawArea();
    }
}

void ScrollBar::drawUpButton()
{
    Display* display = getDisplay();
    GuiRoot* guiRoot = getGuiRoot();

    if (this->isV)
    {
        int w = position.w; //scrollAreaWidth;
        int h = position.h; //scrollAreaLength;

        drawRaisedBox(1, 0, arrowLength, arrowLength);
        drawArrow(    1, 0, arrowLength, arrowLength, Direction::UP);
    }
    else
    {
        int w = position.w; // scrollAreaLength;
        int h = position.h; // scrollAreaWidth;

        drawRaisedBox(0, 1, arrowLength, arrowLength);
        drawArrow(    0, 1, arrowLength, arrowLength, Direction::LEFT);
    }
}

void ScrollBar::drawDownButton()
{
    Display* display = getDisplay();
    GuiRoot* guiRoot = getGuiRoot();

    if (this->isV)
    {
        int w = position.w; //scrollAreaWidth;
        int h = position.h; //scrollAreaLength;
        
        drawRaisedBox(1, h - arrowLength, arrowLength, arrowLength);
        drawArrow(    1, h - arrowLength, arrowLength, arrowLength, Direction::DOWN);
    }
    else
    {
        int w = position.w; // scrollAreaLength;
        int h = position.h; // scrollAreaWidth;

        drawRaisedBox(w - arrowLength, 1, arrowLength, arrowLength);
        drawArrow(    w - arrowLength, 1, arrowLength, arrowLength, Direction::RIGHT);
    }
}

void ScrollBar::drawPressedUpButton()
{
    Display* display = getDisplay();
    GuiRoot* guiRoot = getGuiRoot();

    if (this->isV)
    {
        int w = position.w; //scrollAreaWidth;
        int h = position.h; //scrollAreaLength;

        drawPressedBox(1, 0, arrowLength, arrowLength);
        drawArrow(     1 + 1, 1 + 0, arrowLength, arrowLength, Direction::UP);
    }
    else
    {
        int w = position.w; // scrollAreaLength;
        int h = position.h; // scrollAreaWidth;

        drawPressedBox(0, 1, arrowLength, arrowLength);
        drawArrow(     1 + 0, 1 + 1, arrowLength, arrowLength, Direction::LEFT);
    }
}

void ScrollBar::drawPressedDownButton()
{
    Display* display = getDisplay();
    GuiRoot* guiRoot = getGuiRoot();

    if (this->isV)
    {
        int w = position.w; //scrollAreaWidth;
        int h = position.h; //scrollAreaLength;
        
        drawPressedBox(1, h - arrowLength, arrowLength, arrowLength);
        drawArrow(     1 + 1, 1 + h - arrowLength, arrowLength, arrowLength, Direction::DOWN);
    }
    else
    {
        int w = position.w; // scrollAreaLength;
        int h = position.h; // scrollAreaWidth;

        drawPressedBox(w - arrowLength, 1, arrowLength, arrowLength);
        drawArrow(     1 + w - arrowLength, 1 + 1, arrowLength, arrowLength, Direction::RIGHT);
    }
}


inline void ScrollBar::drawArrows()
{
    drawUpButton();
    drawDownButton();
}

void ScrollBar::drawArea()
{
    Display* display = getDisplay();
    GuiRoot* guiRoot = getGuiRoot();
    
    if (this->isV) {
        int w = position.w; //scrollAreaWidth;
        int h = position.h; //scrollAreaLength;

        XSetForeground(display, scrollBar_gcid, guiRoot->getGuiColor01());
        XDrawLine(display, getWid(), scrollBar_gcid, 
                0, 0, 0, h - 1);

        XSetForeground(display, scrollBar_gcid, guiRoot->getGuiColor02()); // orig war 03
        XDrawLine(display, getWid(), scrollBar_gcid, 
                w - 1, arrowLength, w - 1, h - 1 - arrowLength);
                
        // Scroller
        
        int scrollY = this->scrollY + arrowLength;

        if (this->scrollY > 0) {
            XSetForeground(display, scrollBar_gcid, guiRoot->getGuiColor02());
            XFillRectangle(display, getWid(), scrollBar_gcid,
                    1, arrowLength, w - 2, this->scrollY);
        }

        drawRaisedBox(1, scrollY, arrowLength, this->scrollHeight);

        if (this->scrollY < calcHighestScrollY()) {
            XSetForeground(display, scrollBar_gcid, guiRoot->getGuiColor02());
            XFillRectangle(display, getWid(), scrollBar_gcid,
                    1, scrollY + this->scrollHeight, w - 2, h - scrollY - this->scrollHeight - arrowLength);
        }

    } else {

        int w = position.w; // scrollAreaLength;
        int h = position.h; // scrollAreaWidth;

        XSetForeground(display, scrollBar_gcid, guiRoot->getGuiColor01());
        XDrawLine(display, getWid(), scrollBar_gcid, 
                0, 0, w - 1, 0);

        XSetForeground(display, scrollBar_gcid, guiRoot->getGuiColor02()); // orig war 03
        XDrawLine(display, getWid(), scrollBar_gcid, 
                arrowLength, h - 1,  w - 1 - arrowLength, h - 1);

        // Scroller
        
        int scrollY = this->scrollY + arrowLength;

        if (this->scrollY > 0) {
            XSetForeground(display, scrollBar_gcid, guiRoot->getGuiColor02());
            XFillRectangle(display, getWid(), scrollBar_gcid,
                    arrowLength, 1, this->scrollY, h - 2);
        }

        drawRaisedBox(scrollY, 1, this->scrollHeight, arrowLength);

        if (this->scrollY < calcHighestScrollY()) {
            XSetForeground(display, scrollBar_gcid, guiRoot->getGuiColor02());
            XFillRectangle(display, getWid(), scrollBar_gcid,
                    scrollY + this->scrollHeight, 1, w - scrollY - this->scrollHeight - arrowLength, h - 2);
        }
    }                
}

void ScrollBar::setValue(long value)
{
    if (this->value != value) {
        totalValue = originalTotalValue;
        if (value + heightValue > totalValue) {
            totalValue = value + heightValue;
        }
        this->value = value;
        this->scrollY = calcScrollY();
        //printf("setValue -> scrollY = %d\n", this->scrollY);
        drawArea();
    }
}

void ScrollBar::setValueRange(long totalValue, long heightValue, long value)
{
    bool changed = false;
    
    if (this->totalValue != totalValue || this->heightValue != heightValue
            || this->value != value) {

        int scrollHeight, scrollY;
        
        this->originalTotalValue = totalValue;
        
        if (value + heightValue > totalValue) {
            totalValue = value + heightValue;
        }

        this->totalValue  = totalValue;
        this->heightValue = heightValue;
        this->value       = value;
        
        scrollHeight = calcScrollHeight();
        if (scrollHeight != this->scrollHeight) {
            changed = true;
            this->scrollHeight = scrollHeight;
        }
        scrollY      = calcScrollY();

        if (changed || scrollY != this->scrollY) {
            this->scrollY = scrollY;
            drawArea();
        }
    }
}

#define SCROLLY_TOP_OFFSET 0

inline long ScrollBar::calcScrollHeight()
{
    if (totalValue != 0 && totalValue > heightValue) {
        long rslt;
        rslt = ROUNDED_DIV(heightValue * (scrollAreaLength - SCROLLY_TOP_OFFSET), 
                totalValue);
        if (rslt < 10)
            rslt = 10;
        return rslt;
    } else {
        return scrollAreaLength - SCROLLY_TOP_OFFSET;
    }
}

inline long ScrollBar::calcHighestScrollY()
{
    return scrollAreaLength - scrollHeight;
}

inline long ScrollBar::calcScrollY()
{
    if (totalValue != 0 && totalValue > heightValue) {
        return SCROLLY_TOP_OFFSET + ROUNDED_DIV(value * (calcHighestScrollY() - SCROLLY_TOP_OFFSET),
                totalValue - heightValue);
    } else {
        return 0;
    }
}

inline long ScrollBar::calcValue()
{
    if (totalValue != 0 && totalValue > heightValue) {
        return ROUNDED_DIV((scrollY - SCROLLY_TOP_OFFSET) * (totalValue - heightValue),
                (calcHighestScrollY() - SCROLLY_TOP_OFFSET));
    } else {
        return 1;
    }
}


void ScrollBar::calculateValuesFromPosition()
{
    if (position.w < position.h) {
        isV = true;
        scrollAreaWidth  = position.w;
        arrowLength = scrollAreaWidth - 1;
        scrollAreaLength = position.h - 2 * arrowLength;
    } else {
        isV = false;
        scrollAreaWidth  = position.h;
        arrowLength = scrollAreaWidth - 1;
        scrollAreaLength = position.w - 2 * arrowLength;
    }

    scrollHeight = calcScrollHeight();
    scrollY      = calcScrollY();
}

void ScrollBar::handleScrollStepRepeating()
{
    if (isButtonPressedForScrollStep)
    {
        scrollStepCallback.call(scrollStep);
        EventDispatcher::getInstance()->registerTimerCallback(0, 
                GlobalConfig::getInstance()->getScrollBarRepeatNextMicroSecs(), 
                slotForScrollStepRepeating);
    }
}

