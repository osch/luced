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

#include "ScrollBar.hpp"
#include "GuiRoot.hpp"
#include "util.hpp"
#include "EventDispatcher.hpp"
#include "GlobalConfig.hpp"

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


ScrollBar::ScrollBar(GuiWidget* parent, Orientation::Type orientation)
    : GuiWidget(parent, 0, 0, 1, 1, 0),
      position(0, 0, 1, 1),
      isV(orientation == Orientation::VERTICAL),
      hilitedPart(NONE)
{
    totalValue = 0;
    originalTotalValue = 0;
    heightValue = 0;
    value = 0;
    movingBar = false;
    movingYOffset = 0;
    isButtonPressedForScrollStep = false;
    addToXEventMask(ExposureMask|ButtonPressMask|ButtonReleaseMask|ButtonMotionMask|EnterWindowMask|LeaveWindowMask);
    calculateValuesFromPosition();

    if (!scrollBarInitialized) {
        initStatically();
        scrollBarInitialized = true;
    }
    setBackgroundColor(getGuiRoot()->getGuiColor02());
 }


GuiElement::ProcessingResult ScrollBar::processEvent(const XEvent *event)
{
    if (GuiWidget::processEvent(event) == EVENT_PROCESSED) {
        return EVENT_PROCESSED;
    } else {
        
        switch (event->type) {
            
            case GraphicsExpose:
            case Expose: {
                this->drawArrows();
                this->drawArea();
                return EVENT_PROCESSED;
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
                        if (value == 0 && scrollY != 0) {
                            this->scrollY = calcScrollY();
                            //printf("setValue -> scrollY = %d\n", this->scrollY);
                            drawArea();
                        }
                        scrollStepCallback.call(scrollStep = ScrollStep::LINE_UP);
                    }
                    else if (h - arrowLength <= y && y < h) {
                        isButtonPressedForScrollStep = true;
                        drawPressedDownButton();
                        if (value == totalValue - heightValue && scrollY != calcHighestScrollY()) {
                            this->scrollY = calcHighestScrollY();
                            //printf("setValue -> scrollY = %d\n", this->scrollY);
                            drawArea();
                        }
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
                        EventDispatcher::getInstance()->registerTimerCallback(Seconds(0), 
                                GlobalConfig::getInstance()->getScrollBarRepeatFirstMicroSecs(),
                                Callback0(this, &ScrollBar::handleScrollStepRepeating));
                    }
                    return EVENT_PROCESSED;
                }
                break;
            }

            case ButtonRelease: {
                if (this->movingBar) {
                    //printf("ButtonReleased\n");
                    this->movingBar = false;
                    return EVENT_PROCESSED;
                }
                if (isButtonPressedForScrollStep) {
                    if (scrollStep == ScrollStep::LINE_UP) {
                        drawUpButton();
                    } else if (scrollStep == ScrollStep::LINE_DOWN) {
                        drawDownButton();
                    }
                    isButtonPressedForScrollStep = false;
                    return EVENT_PROCESSED;
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
                hiliteScrollBarPartAtMousePosition(event->xmotion.x, event->xmotion.y);
                return EVENT_PROCESSED;
            }
            case EnterNotify: {
                int x = event->xcrossing.x;
                int y = event->xcrossing.y;
                hiliteScrollBarPartAtMousePosition(x, y);
                addToXEventMask(PointerMotionMask);
                return EVENT_PROCESSED;
            }
            
            case LeaveNotify: {
                int x = event->xcrossing.x;
                int y = event->xcrossing.y;
                hiliteScrollBarPartAtMousePosition(x, y);
                removeFromXEventMask(PointerMotionMask);
                return EVENT_PROCESSED;
            }
        }
        return propagateEventToParentWidget(event);
    }
}
    
void ScrollBar::hiliteScrollBarPartAtMousePosition(int mouseX, int mouseY)
{
    int x, y;
    int w, h;
    if (this->isV) {
        x = mouseX;
        y = mouseY;
        w = position.w;
        h = position.h;
    } else {
        x = mouseY;
        y = mouseX;
        w = position.h;
        h = position.w;
    }
    HilitedPart newPart = NONE;
    if (0 <= x && x < w)
    {
        if ((y >= this->scrollY + arrowLength) 
                && (y < this->scrollY + arrowLength + this->scrollHeight))
        {
            if (value > 0 || value + heightValue < totalValue) {
                newPart = SCROLLER;
            }
        }
        else if (0 <= y && y < arrowLength) {
            if (value > 0) {
                newPart = TOP_ARROW;
            }
        }
        else if (h - arrowLength <= y && y < h) {
            if (value + heightValue < totalValue) {
                newPart = BOTTOM_ARROW;
            }
        }
        else if (arrowLength <= y && y < this->scrollY + arrowLength) {
        }
        else if (this->scrollY + arrowLength + scrollHeight <= y && y < h - arrowLength) {
        }
    }
    if (newPart != hilitedPart)
    {
        HilitedPart oldPart = hilitedPart;
        
        hilitedPart = newPart;
        
        switch (oldPart) {
            case SCROLLER: {
                drawArea();
                break;
            }
            case TOP_ARROW: {
                drawUpButton();
                break;
            }
            case BOTTOM_ARROW: {
                drawDownButton();
                break;
            }
        }

        switch (hilitedPart) {
            case SCROLLER: {
                drawArea();
                break;
            }
            case TOP_ARROW: {
                drawUpButton();
                break;
            }
            case BOTTOM_ARROW: {
                drawDownButton();
                break;
            }
        }
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

#define MIN_SCROLLER_HEIGHT 10

GuiElement::Measures ScrollBar::getDesiredMeasures()
{
    int scrollBarWidth = GlobalConfig::getInstance()->getScrollBarWidth();

    if (this->isV) {
        return Measures(scrollBarWidth, 2*scrollBarWidth + MIN_SCROLLER_HEIGHT, scrollBarWidth, 
                        2*scrollBarWidth + 2*MIN_SCROLLER_HEIGHT , 
                        scrollBarWidth, INT_MAX);
    } else {
        return Measures(2*scrollBarWidth + MIN_SCROLLER_HEIGHT, scrollBarWidth, 
                        2*scrollBarWidth + 2*MIN_SCROLLER_HEIGHT, scrollBarWidth, 
                        INT_MAX, scrollBarWidth);
    }
}

void ScrollBar::drawUpButton()
{
    Display* display = getDisplay();
    GuiRoot* guiRoot = getGuiRoot();

    GuiColor color;
    if (hilitedPart == TOP_ARROW) {
        color = GuiRoot::getInstance()->getGuiColor04();
    } else {
        color = GuiRoot::getInstance()->getGuiColor03();
    }

    if (this->isV)
    {
        int w = position.w; //scrollAreaWidth;
        int h = position.h; //scrollAreaLength;

        drawRaisedBox(0, 0, arrowLength, arrowLength, color);
        drawArrow(    0, 0, arrowLength, arrowLength, Direction::UP);
    }
    else
    {
        int w = position.w; // scrollAreaLength;
        int h = position.h; // scrollAreaWidth;

        drawRaisedBox(0, 0, arrowLength, arrowLength, color);
        drawArrow(    0, 0, arrowLength, arrowLength, Direction::LEFT);
    }
}

void ScrollBar::drawDownButton()
{
    Display* display = getDisplay();
    GuiRoot* guiRoot = getGuiRoot();

    GuiColor color;
    if (hilitedPart == BOTTOM_ARROW) {
        color = GuiRoot::getInstance()->getGuiColor04();
    } else {
        color = GuiRoot::getInstance()->getGuiColor03();
    }

    if (this->isV)
    {
        int w = position.w; //scrollAreaWidth;
        int h = position.h; //scrollAreaLength;
        
        drawRaisedBox(0, h - arrowLength, arrowLength, arrowLength, color);
        drawArrow(    0, h - arrowLength, arrowLength, arrowLength, Direction::DOWN);
    }
    else
    {
        int w = position.w; // scrollAreaLength;
        int h = position.h; // scrollAreaWidth;

        drawRaisedBox(w - arrowLength, 0, arrowLength, arrowLength, color);
        drawArrow(    w - arrowLength, 0, arrowLength, arrowLength, Direction::RIGHT);
    }
}

void ScrollBar::drawPressedUpButton()
{
    Display* display = getDisplay();
    GuiRoot* guiRoot = getGuiRoot();

    GuiColor color;
    if (hilitedPart == TOP_ARROW) {
        color = GuiRoot::getInstance()->getGuiColor04();
    } else {
        color = GuiRoot::getInstance()->getGuiColor03();
    }

    if (this->isV)
    {
        int w = position.w; //scrollAreaWidth;
        int h = position.h; //scrollAreaLength;

        drawPressedBox(0, 0, arrowLength, arrowLength, color);
        drawArrow(     1 + 0, 1 + 0, arrowLength, arrowLength, Direction::UP);
    }
    else
    {
        int w = position.w; // scrollAreaLength;
        int h = position.h; // scrollAreaWidth;

        drawPressedBox(0, 0, arrowLength, arrowLength, color);
        drawArrow(     1 + 0, 1 + 0, arrowLength, arrowLength, Direction::LEFT);
    }
}

void ScrollBar::drawPressedDownButton()
{
    Display* display = getDisplay();
    GuiRoot* guiRoot = getGuiRoot();

    GuiColor color;
    if (hilitedPart == BOTTOM_ARROW) {
        color = GuiRoot::getInstance()->getGuiColor04();
    } else {
        color = GuiRoot::getInstance()->getGuiColor03();
    }

    if (this->isV)
    {
        int w = position.w; //scrollAreaWidth;
        int h = position.h; //scrollAreaLength;
        
        drawPressedBox(0, h - arrowLength, arrowLength, arrowLength, color);
        drawArrow(     1 + 0, 1 + h - arrowLength, arrowLength, arrowLength, Direction::DOWN);
    }
    else
    {
        int w = position.w; // scrollAreaLength;
        int h = position.h; // scrollAreaWidth;

        drawPressedBox(w - arrowLength, 0, arrowLength, arrowLength, color);
        drawArrow(     1 + w - arrowLength, 1 + 0, arrowLength, arrowLength, Direction::RIGHT);
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

//        XSetForeground(display, scrollBar_gcid, guiRoot->getGuiColor01());
//        XDrawLine(display, getWid(), scrollBar_gcid, 
//                0, 0, 0, h - 1);

        XSetForeground(display, scrollBar_gcid, guiRoot->getGuiColor02()); // orig war 03
        XDrawLine(display, getWid(), scrollBar_gcid, 
                w - 1, arrowLength, w - 1, h - 1 - arrowLength);
                
        // Scroller
        
        int scrollY = this->scrollY + arrowLength;

        if (this->scrollY > 0) {
            XSetForeground(display, scrollBar_gcid, guiRoot->getGuiColor02());
            XFillRectangle(display, getWid(), scrollBar_gcid,
                    0, arrowLength, w - 1, this->scrollY);
        }

        GuiColor color;
        if (hilitedPart == SCROLLER) {
            color = GuiRoot::getInstance()->getGuiColor04();
        } else {
            color = GuiRoot::getInstance()->getGuiColor03();
        }
        drawRaisedBox(0, scrollY, arrowLength, this->scrollHeight, color);

        if (this->scrollY < calcHighestScrollY()) {
            XSetForeground(display, scrollBar_gcid, guiRoot->getGuiColor02());
            XFillRectangle(display, getWid(), scrollBar_gcid,
                    0, scrollY + this->scrollHeight, w - 1, h - scrollY - this->scrollHeight - arrowLength);
        }

    } else {

        int w = position.w; // scrollAreaLength;
        int h = position.h; // scrollAreaWidth;

//        XSetForeground(display, scrollBar_gcid, guiRoot->getGuiColor01());
//        XDrawLine(display, getWid(), scrollBar_gcid, 
//                0, 0, w - 1, 0);

        XSetForeground(display, scrollBar_gcid, guiRoot->getGuiColor02()); // orig war 03
        XDrawLine(display, getWid(), scrollBar_gcid, 
                arrowLength, h - 1,  w - 1 - arrowLength, h - 1);

        // Scroller
        
        int scrollY = this->scrollY + arrowLength;

        if (this->scrollY > 0) {
            XSetForeground(display, scrollBar_gcid, guiRoot->getGuiColor02());
            XFillRectangle(display, getWid(), scrollBar_gcid,
                    arrowLength, 0, this->scrollY, h - 1);
        }

        GuiColor color;
        if (hilitedPart == SCROLLER) {
            color = GuiRoot::getInstance()->getGuiColor04();
        } else {
            color = GuiRoot::getInstance()->getGuiColor03();
        }
        drawRaisedBox(scrollY, 0, this->scrollHeight, arrowLength, color);

        if (this->scrollY < calcHighestScrollY()) {
            XSetForeground(display, scrollBar_gcid, guiRoot->getGuiColor02());
            XFillRectangle(display, getWid(), scrollBar_gcid,
                    scrollY + this->scrollHeight, 0, w - scrollY - this->scrollHeight - arrowLength, h - 1);
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
        if (rslt < MIN_SCROLLER_HEIGHT)
            rslt = MIN_SCROLLER_HEIGHT;
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
    if (isV) {
        scrollAreaWidth  = position.w;
        arrowLength = scrollAreaWidth;
        scrollAreaLength = position.h - 2 * arrowLength;
    } else {
        scrollAreaWidth  = position.h;
        arrowLength = scrollAreaWidth;
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
        EventDispatcher::getInstance()->registerTimerCallback(Seconds(0), 
                GlobalConfig::getInstance()->getScrollBarRepeatNextMicroSecs(), 
                Callback0(this, &ScrollBar::handleScrollStepRepeating));
    }
}

