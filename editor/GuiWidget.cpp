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

#include <X11/Xatom.h>

#include "util.hpp"
#include "GuiRoot.hpp"
#include "GuiWidget.hpp"
#include "EventDispatcher.hpp"
#include "GlobalConfig.hpp"
#include "SingletonInstance.hpp"

namespace LucED
{

class GuiWidgetSingletonData : public HeapObject
{
public:
    static GuiWidgetSingletonData* getInstance() { return instance.getPtr(); }
 
    GC         getGcid()         { return gcid; }
    TextStyle* getGuiTextStyle() { return &guiTextStyle; }

private:
    friend class SingletonInstance<GuiWidgetSingletonData>;
    
    GuiWidgetSingletonData()
        : guiTextStyle(GlobalConfig::getInstance()->getGuiFont(), GlobalConfig::getInstance()->getGuiFontColor())
    {
        Display* display = GuiRoot::getInstance()->getDisplay();
        GuiRoot* guiRoot = GuiRoot::getInstance();

        gcid = XCreateGC(display, guiRoot->getRootWid(), 0, NULL);

        XSetForeground(display, gcid, guiRoot->getBlackColor());
        XSetBackground(display, gcid, guiRoot->getGreyColor());

        XSetLineAttributes(display, gcid, 0, 
            LineSolid, CapProjecting, JoinMiter);

        XSetGraphicsExposures(display, gcid, True);

        XSetFont(display, gcid, guiTextStyle.getFontHandle());
    }
    
    ~GuiWidgetSingletonData()
    {
        XFreeGC(GuiRoot::getInstance()->getDisplay(), gcid);
    }
    
    static SingletonInstance<GuiWidgetSingletonData> instance;
    GC gcid;
    TextStyle guiTextStyle;
};

} // namespace LucED

using namespace LucED;

SingletonInstance<GuiWidgetSingletonData> GuiWidgetSingletonData::instance;


Display* GuiWidget::getDisplay()
{
    return GuiRoot::getInstance()->getDisplay();
}

WidgetId GuiWidget::getRootWid()
{
    return GuiRoot::getInstance()->getRootWid();
}

EventDispatcher* GuiWidget::getEventDispatcher()
{
    return EventDispatcher::getInstance();
}

GuiWidget::GuiWidget(int x, int y, unsigned int width, unsigned int height, unsigned border_width)
    : isTopWindow(true),
      parent(NULL),
      eventMask(0),
      position(x, y, width, height),
      visible(false),
      gcid(GuiWidgetSingletonData::getInstance()->getGcid())
{
    wid = WidgetId(XCreateSimpleWindow(getDisplay(), getRootWid(), 
                                       x, y, width, height, border_width, 
                                       GuiRoot::getInstance()->getBlackColor(), 
                                       GuiRoot::getInstance()->getGreyColor()));
 
    EventDispatcher::getInstance()->registerEventReceiver(EventRegistration(this, wid));
    addToXEventMask(StructureNotifyMask);
}
    
GuiWidget::GuiWidget(GuiWidget* parent,
            int x, int y, unsigned int width, unsigned int height, unsigned border_width)
    : isTopWindow(false),
      parent(parent),
      eventMask(0),
      position(x, y, width, height),
      visible(false),
      gcid(GuiWidgetSingletonData::getInstance()->getGcid())
{
    wid = WidgetId(XCreateSimpleWindow(getDisplay(), parent->getWid(), 
                                       x, y, width, height, border_width, 
                                       GuiRoot::getInstance()->getBlackColor(), 
                                       GuiRoot::getInstance()->getGreyColor()));
    
    EventDispatcher::getInstance()->registerEventReceiver(EventRegistration(this, wid));
    addToXEventMask(StructureNotifyMask);
}

GuiWidget::~GuiWidget()
{
    if (nextFocusWidget.isValid() && nextFocusWidget->prevFocusWidget == this) {
        nextFocusWidget->prevFocusWidget = prevFocusWidget;
    }
    if (prevFocusWidget.isValid() && prevFocusWidget->nextFocusWidget == this) {
        prevFocusWidget->nextFocusWidget = nextFocusWidget;
    }
    EventDispatcher::getInstance()->removeEventReceiver(EventRegistration(this, wid));
    if (isTopWindow) {
        XDestroyWindow(getDisplay(), wid);
    }
}

GuiElement::ProcessingResult GuiWidget::processEvent(const XEvent *event)
{
    switch (event->type) {

        case ConfigureNotify: {
            if (event->xconfigure.window == getWid()
             && event->xconfigure.event  == getWid()) {
                int x = event->xconfigure.x;
                int y = event->xconfigure.y;
                int w = event->xconfigure.width;
                int h = event->xconfigure.height;
                Position newPosition(x,y,w,h);
                if (position != newPosition) {
                    this->treatNewWindowPosition(newPosition);
                    position = newPosition;
                }
                return EVENT_PROCESSED;
            } else {
                return NOT_PROCESSED;
            }
        }
        default: {
            return NOT_PROCESSED;
        }
    }
}

void GuiWidget::setPosition(Position p)
{
    XMoveResizeWindow(getDisplay(), wid, p.x, p.y, p.w, p.h);
    // this->position is set via ConfigureNotify event
}

Position GuiWidget::getAbsolutePosition()
{
    Position rslt(this->position);
    Window child_return;
    XTranslateCoordinates(getDisplay(), getWid(), getGuiRoot()->getRootWid(), 0, 0, &rslt.x, 
                          &rslt.y, &child_return);
    return rslt;
}

void GuiWidget::setSize(int width, int height)
{
    XResizeWindow(getDisplay(), wid, width, height);
    // this->position is set via ConfigureNotify event
}

void GuiWidget::show()
{
    XMapWindow(getDisplay(), wid);
    visible = true;
}

void GuiWidget::hide()
{
    XUnmapWindow(getDisplay(), wid);
    visible = false;
}

void GuiWidget::setBackgroundColor(GuiColor color)
{
    XSetWindowBackground(getDisplay(), getWid(), color);
}

void GuiWidget::setBorderColor(GuiColor color)
{
    XSetWindowBorder(getDisplay(), getWid(), color);
}

void GuiWidget::addToXEventMask(long eventMask)
{
    this->eventMask |= eventMask;
    XSelectInput(getDisplay(), getWid(), this->eventMask);    
}

void GuiWidget::removeFromXEventMask(long eventMask)
{
    this->eventMask &= ~eventMask;
    XSelectInput(getDisplay(), getWid(), this->eventMask);    
}


static inline void drawLine(Display *display, WidgetId wid, GC gc, int x, int y, int dx, int dy)
{
    XDrawLine(display, wid,  gc, 
            x, y, x + dx, y + dy);
}

void GuiWidget::drawLine(int x, int y, int dx, int dy)
{
    Display* display = getDisplay();
    WidgetId wid = getWid();
    XSetForeground(display, gcid, getGuiRoot()->getBlackColor());
    XDrawLine(display, wid,  gcid, 
            x, y, x + dx, y + dy);
}

void GuiWidget::drawLine(int x, int y, int dx, int dy, GuiColor color)
{
    Display* display = getDisplay();
    WidgetId wid = getWid();
    XSetForeground(display, gcid, color);
    XDrawLine(display, wid,  gcid, 
            x, y, x + dx, y + dy);
}

void GuiWidget::drawRaisedSurface(int x, int y, int w, int h)
{
    Display* display = getDisplay();
    WidgetId wid = getWid();

    XSetForeground(display, gcid, GuiRoot::getInstance()->getGuiColor03());
    XFillRectangle(display, wid, gcid,
            x, y, w, h);
}

void GuiWidget::drawRaisedSurface(int x, int y, int w, int h, GuiColor color)
{
    Display* display = getDisplay();
    WidgetId wid = getWid();

    XSetForeground(display, gcid, color);
    XFillRectangle(display, wid, gcid,
            x, y, w, h);
}


int GuiWidget::getRaisedBoxBorderWidth()
{
    return 1;
}

void GuiWidget::drawRaisedBox(int x, int y, int w, int h, GuiColor color)
{
    Display* display = getDisplay();
    WidgetId wid = getWid();
    
    int x1 = x, y1 = y, x2 = x + w -1, y2 = y + h - 1;
    int dx = w - 1, dy = h - 1;
        
/*        XSetForeground(display, gcid, GuiRoot::getInstance()->getGuiColor03());
        drawLine(display, wid, gcid, 
                x + dx, y + 1, 0, dy - 1);
*/
        XSetForeground(display, gcid, GuiRoot::getInstance()->getGuiColor05());
        ::drawLine(display, wid, gcid,
                x, y, dx - 1, 0);
        ::drawLine(display, wid, gcid,
                x, y, 0, dy - 1);

        XSetForeground(display, gcid, color);
        XFillRectangle(display, wid, gcid,
                x + 1, y + 1, dx - 1, dy - 1);

        XSetForeground(display, gcid, GuiRoot::getInstance()->getGuiColor01());
        ::drawLine(display, wid, gcid,
                x + dx, y, 0, dy);
        ::drawLine(display, wid, gcid,
                x, y + dy, dx, 0);


}

void GuiWidget::drawRaisedBox(int x, int y, int w, int h)
{
    drawRaisedBox(x, y, w, h, GuiRoot::getInstance()->getGuiColor03());
}


void GuiWidget::drawPressedBox(int x, int y, int w, int h, GuiColor color)
{
    Display* display = getDisplay();
    WidgetId wid = getWid();

    int x1 = x, y1 = y, x2 = x + w -1, y2 = y + h - 1;
    int dx = w - 1, dy = h - 1;
        
/*        XSetForeground(display, gcid, GuiRoot::getInstance()->getGuiColor03());
        ::drawLine(display, wid, gcid, 
                x + dx, y + 1, 0, dy - 1);
*/
        XSetForeground(display, gcid, GuiRoot::getInstance()->getGuiColor01());
        ::drawLine(display, wid, gcid,
                x, y, dx - 1, 0);
        ::drawLine(display, wid, gcid,
                x, y, 0, dy - 1);

        XSetForeground(display, gcid, color);
        XFillRectangle(display, wid, gcid,
                x + 1, y + 1, dx - 1, dy - 1);

        XSetForeground(display, gcid, GuiRoot::getInstance()->getGuiColor05());
        ::drawLine(display, wid, gcid,
                x + dx, y, 0, dy);
        ::drawLine(display, wid, gcid,
                x, y + dy, dx, 0);


}

void GuiWidget::drawPressedBox(int x, int y, int w, int h)
{
    drawPressedBox(x, y, w, h, GuiRoot::getInstance()->getGuiColor03());
}

void GuiWidget::drawArrow(int x, int y, int w, int h, const Direction::Type direct)
{
    Display* display = getDisplay();
    WidgetId wid = getWid();

    XSetForeground(display, gcid, GuiRoot::getInstance()->getGuiColor01());

    const int d = (w-5)/ 2;
    const int n = 6;
    XPoint points[n] = {{2 + d, 2 + d/2}, {-d, d}, {0, d/3}, {2*d, 0}, {0, -d/3}, {-d, -d}};
    
    if (direct == Direction::UP) {
        points[0].x += x;
        points[0].y += y;
    } else if (direct == Direction::DOWN) {
        points[0].x += x;
        points[0].y  = y + h - 1 - points[0].y;
        for (int i = 1; i < n; ++i) {
            points[i].y = -1 * points[i].y;
        }
    } else if (direct == Direction::LEFT) {
        int h;
        for (int i = 0; i < n; ++i) {
                      h = points[i].x;
            points[i].x = points[i].y;
            points[i].y = h;
        }
        points[0].x += x;
        points[0].y += y;
    } else if (direct == Direction::RIGHT) {
        int h;
        for (int i = 0; i < n; ++i) {
                      h = points[i].x;
            points[i].x = points[i].y;
            points[i].y = h;
        }
        points[0].x = x + w - 1 - points[0].x;
        points[0].y += y;
        for (int i = 1; i < n; ++i) {
            points[i].x = -1 * points[i].x;
        }
    }
    XFillPolygon(display, wid, gcid,
            points, n, Convex, CoordModePrevious);
    XDrawLines(display, wid, gcid,
            points, n, CoordModePrevious);
}


void GuiWidget::drawInactiveSunkenFrame(int x, int y, int w, int h)
{
    Display* display = getDisplay();
    WidgetId wid = getWid();
    
    w -= 1; 
    h -= 1;
    
    XSetForeground(display,  gcid, GuiRoot::getInstance()->getGuiColor02());
    XDrawLine(display, wid,  gcid,    x + 0, y + 0,    x + w, y + 0);
    XDrawLine(display, wid,  gcid,    x + 0, y + h,    x + 0, y + 0);

    XSetForeground(display,  gcid, GuiRoot::getInstance()->getGuiColor03());
    XDrawLine(display, wid,  gcid,    x + w, y + h,    x + 0, y + h);
    XDrawLine(display, wid,  gcid,    x + w, y + 0,    x + w, y + h);
}

void GuiWidget::drawActiveSunkenFrame(int x, int y, int w, int h)
{
    Display* display = getDisplay();
    WidgetId wid = getWid();
    
    w -= 1; 
    h -= 1;
    
    XSetForeground(display,  gcid, GuiRoot::getInstance()->getGuiColor01());
    XDrawLine(display, wid,  gcid,    x + 0, y + 0,    x + w, y + 0);
    XDrawLine(display, wid,  gcid,    x + 0, y + h,    x + 0, y + 0);

    XSetForeground(display,  gcid, GuiRoot::getInstance()->getGuiColor02());
    XDrawLine(display, wid,  gcid,    x + w, y + h,    x + 0, y + h);
    XDrawLine(display, wid,  gcid,    x + w, y + 0,    x + w, y + h);
}

void GuiWidget::drawFrame(int x, int y, int w, int h)
{
    Display* display = getDisplay();
    WidgetId wid = getWid();
    
    w -= 1; 
    h -= 1;
    
    XSetForeground(display,  gcid, GuiRoot::getInstance()->getGuiColor01());
    XDrawLine(display, wid,  gcid,    x + 0, y + 0,    x + w, y + 0);
    XDrawLine(display, wid,  gcid,    x + 0, y + h,    x + 0, y + 0);

//    XSetForeground(display,  gcid, GuiRoot::getInstance()->getGuiColor05());
    XDrawLine(display, wid,  gcid,    x + w, y + h,    x + 0, y + h);
    XDrawLine(display, wid,  gcid,    x + w, y + 0,    x + w, y + h);
}

void GuiWidget::undrawFrame(int x, int y, int w, int h)
{
    Display* display = getDisplay();
    WidgetId wid = getWid();
    
    w -= 1; 
    h -= 1;
    
    XSetForeground(display,  gcid, GuiRoot::getInstance()->getGuiColor03());
    XDrawLine(display, wid,  gcid,    x + 0, y + 0,    x + w, y + 0);
    XDrawLine(display, wid,  gcid,    x + 0, y + h,    x + 0, y + 0);

//    XSetForeground(display,  gcid, GuiRoot::getInstance()->getGuiColor05());
    XDrawLine(display, wid,  gcid,    x + w, y + h,    x + 0, y + h);
    XDrawLine(display, wid,  gcid,    x + w, y + 0,    x + w, y + h);
}

void GuiWidget::drawDottedFrame(int x, int y, int w, int h)
{
    Display* display = getDisplay();
    WidgetId wid = getWid();

    XSetLineAttributes(display, gcid, 1, 
        LineOnOffDash, CapButt, JoinMiter);

    char dashList[] = { 1, 1 };
    XSetDashes(display, gcid, 0, dashList, sizeof(dashList));

    w -= 1; 
    h -= 1;
    
    XSetForeground(display,  gcid, GuiRoot::getInstance()->getGuiColor01());
    XDrawLine(display, wid,  gcid,    x + 0, y + 0,    x + w, y + 0);
    XDrawLine(display, wid,  gcid,    x + 0, y + h,    x + 0, y + 0);

//    XSetForeground(display,  gcid, GuiRoot::getInstance()->getGuiColor05());
    XDrawLine(display, wid,  gcid,    x + w, y + h,    x + 0, y + h);
    XDrawLine(display, wid,  gcid,    x + w, y + 0,    x + w, y + h);

    XSetLineAttributes(display, gcid, 0, 
        LineSolid, CapButt, JoinMiter);
}



TextStyle* GuiWidget::getGuiTextStyle()
{
    return GuiWidgetSingletonData::getInstance()->getGuiTextStyle();
}

void GuiWidget::drawGuiText(int x, int y, const char* ptr, long length)
{

    XSetForeground(getDisplay(), gcid, getGuiTextStyle()->getColor());
    XDrawString(getDisplay(), getWid(), 
                gcid, x, y + getGuiTextStyle()->getLineAscent(), ptr, length);
}

int GuiWidget::getGuiTextHeight()
{
    return getGuiTextStyle()->getLineHeight();
}

GuiWidget::GuiClipping GuiWidget::obtainGuiClipping(int x, int y, int w, int h)
{
    XRectangle r;
    r.x = x;
    r.y = y;
    r.width  = w;
    r.height = h;
    XSetClipRectangles(getDisplay(), gcid, 
            0, 0, &r, 1, Unsorted);    
    return GuiClipping(this);
}

GuiWidget::GuiClipping::~GuiClipping()
{
    XSetClipMask(guiWidget->getDisplay(), guiWidget->gcid, None);
}


void GuiWidget::setWinGravity(int winGravity)
{
    XSetWindowAttributes at;
    at.win_gravity = winGravity;
    XChangeWindowAttributes(getDisplay(), getWid(), 
            CWWinGravity, &at);
}


void GuiWidget::setBitGravity(int bitGravity)
{
    XSetWindowAttributes at;
    at.bit_gravity = bitGravity;
    XChangeWindowAttributes(getDisplay(), getWid(), 
            CWBitGravity, &at);
}


