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
#include "TextStyleCache.hpp"
#include "GuiClipping.hpp"

namespace LucED
{

class GuiWidgetSingletonData : public HeapObject
{
public:
    static GuiWidgetSingletonData* getInstance() { return instance.getPtr(); }
 
    GC                getGcid()         { return gcid; }
    RawPtr<TextStyle> getGuiTextStyle() { return guiTextStyle; }

    RawPtr<GuiClipping> getClipping() {
        return &clipping;
    }

private:
    friend class SingletonInstance<GuiWidgetSingletonData>;
    
    GuiWidgetSingletonData()
        : gcid(XCreateGC(GuiRoot::getInstance()->getDisplay(), 
                         GuiRoot::getInstance()->getRootWid(), 0, NULL)),
          guiTextStyle(TextStyleCache::getInstance()
                                     ->getTextStyle(GlobalConfig::getConfigData()->getGeneralConfig()->getGuiFont(), 
                                                    GlobalConfig::getConfigData()->getGeneralConfig()->getGuiFontColor())),
          clipping(GuiRoot::getInstance()->getDisplay(), 
                   gcid)
    {
        Display* display = GuiRoot::getInstance()->getDisplay();
        GuiRoot* guiRoot = GuiRoot::getInstance();

        XSetForeground(display, gcid, guiRoot->getBlackColor());
        XSetBackground(display, gcid, guiRoot->getGreyColor());

        XSetLineAttributes(display, gcid, 0, LineSolid, CapProjecting, JoinMiter);

        XSetGraphicsExposures(display, gcid, True);

        XSetFont(display, gcid, guiTextStyle->getFontHandle());
    }
    
    ~GuiWidgetSingletonData()
    {
        clipping.clear();

        XFreeGC(GuiRoot::getInstance()->getDisplay(), gcid);
    }
    
    static SingletonInstance<GuiWidgetSingletonData> instance;

    GC             gcid;
    TextStyle::Ptr guiTextStyle;
    GuiClipping    clipping;
};

} // namespace LucED

using namespace LucED;

SingletonInstance<GuiWidgetSingletonData> GuiWidgetSingletonData::instance;


GuiWidget::GuiWidget(RawPtr<GuiWidget>     parentWidget, 
                     RawPtr<EventListener> eventListener,
                     const Position&       position, 
                     int                   borderWidth)
    : borderWidth(borderWidth),
      isTopWindow(!parentWidget.isValid()),
      parent(parentWidget),
      eventListener(eventListener),
      eventMask(0),
      position(position),
      width (position.w - 2 * borderWidth),
      height(position.h - 2 * borderWidth),
      gcid(GuiWidgetSingletonData::getInstance()->getGcid()),
      isHandlingGraphicsExpose(false),
      ignoreRedrawRegion(false)
{
#if 0
    wid = WidgetId(XCreateSimpleWindow(getDisplay(), parent->getWid(), 
                                       x, y, width, height, border_width, 
                                       GuiRoot::getInstance()->getBlackColor(), 
                                       GuiRoot::getInstance()->getWhiteColor()));
#endif

//    GuiElement::hide();
    
    XSetWindowAttributes at;
    at.background_pixmap = None;
    at.backing_store = WhenMapped;

    if (parent.isValid())
    {
        wid = WidgetId(XCreateWindow(getDisplay(), parent->getWid(), 
                                     position.x, position.y, width, height, borderWidth, 
                                     CopyFromParent, // <-- depth from parent
                                     InputOutput,
                                     CopyFromParent, // <-- visual from parent
                                     CWBackPixmap, //CWBackPixmap|CWBackingStore, // at least CWBackingStore caused problems on some display
                                     &at));
    } else
    {
        wid = WidgetId(XCreateSimpleWindow(getDisplay(), getRootWid(), 
                                           position.x, position.y, width, height, borderWidth, 
                                           GuiRoot::getInstance()->getGreyColor(), 
                                           GuiRoot::getInstance()->getGreyColor()));
    } 
    EventDispatcher::getInstance()->registerEventReceiver(EventRegistration(this, wid));
    addToXEventMask(StructureNotifyMask);
}

GuiWidget::~GuiWidget()
{
    EventDispatcher::getInstance()->removeEventReceiver(EventRegistration(this, wid));
    if (redrawRegion.isValid()) {
        XDestroyRegion(redrawRegion.get());
        redrawRegion.invalidate();
    }
    if (isTopWindow) {
        XUnmapWindow(getDisplay(), wid); // Workaround: without "unmap" strange focus changed happened under the Exceed xserver
        XDestroyWindow(getDisplay(), wid);
    }
}

RawPtr<GuiClipping> GuiWidget::getClipping() const
{
    return GuiWidgetSingletonData::getInstance()->getClipping();
}

int GuiWidget::internalProcessExposureEvent(const XEvent* event)
{
    XRectangle r;
    int count;
    if (event->type == GraphicsExpose) {
        r.x      = event->xgraphicsexpose.x;
        r.y      = event->xgraphicsexpose.y;
        r.width  = event->xgraphicsexpose.width;
        r.height = event->xgraphicsexpose.height;
        count    = event->xgraphicsexpose.count; // Anzahl der noch folgenden Events
        if (!isHandlingGraphicsExpose) {
            isHandlingGraphicsExpose = true;
            if (scrollRequests.getLength() > 0) {
                scrollRequests.remove(0);
            }
        }
        if (count == 0) {
            isHandlingGraphicsExpose = false;
        }
    } else {
        r.x      = event->xexpose.x;
        r.y      = event->xexpose.y;
        r.width  = event->xexpose.width;
        r.height = event->xexpose.height;
        count    = event->xexpose.count; // Anzahl der noch folgenden Events
    }
    {
        if (!redrawRegion.isValid()) {
            redrawRegion = XCreateRegion();
            ignoreRedrawRegion = false;
        }
        if (r.x < 0 && r.y < 0) {
            ignoreRedrawRegion = true; // Workaround for broken RedrawEvents (xnest)
        }
        Region transformedUpdateRegion = calculateScrolledUpdateRegion(&r);
        XUnionRegion(redrawRegion.get(), 
                     transformedUpdateRegion,
                     redrawRegion.get());
        XDestroyRegion(transformedUpdateRegion);
    }
    return count;   
}

GuiWidget::ProcessingResult GuiWidget::processEvent(const XEvent* event)
{
    switch (event->type)
    {
        case ConfigureNotify: {
            if (   event->xconfigure.window == getWid()
                && event->xconfigure.event  == getWid())
            {
                int x = event->xconfigure.x;
                int y = event->xconfigure.y;
                int w = event->xconfigure.width;
                int h = event->xconfigure.height;
                Position newPosition(x, y, w + 2 * borderWidth,
                                           h + 2 * borderWidth);
                if (position != newPosition) {
                    position = newPosition;
                    this->width  = position.w - 2 * borderWidth;
                    this->height = position.h - 2 * borderWidth;
                    eventListener->processGuiWidgetNewPositionEvent(newPosition);
                }
                return EVENT_PROCESSED;
            }
            else {
                return NOT_PROCESSED;
            }
        }
        case NoExpose: {
            if (scrollRequests.getLength() > 0) {
                scrollRequests.remove(0);
            }
            isHandlingGraphicsExpose = false;
            return EVENT_PROCESSED;
        }
        
        case GraphicsExpose:
        case Expose:
        {
            int count = internalProcessExposureEvent(event);

            XEvent newEvent;
            while (XCheckWindowEvent(getDisplay(), getWid(), -1, &newEvent) == True)
            {
                if (   newEvent.type == GraphicsExpose
                    || newEvent.type == Expose)
                {
                    count = internalProcessExposureEvent(&newEvent);
                } else {
                    XPutBackEvent(getDisplay(), &newEvent);
                    break;
                }
            }

            if (count == 0 && redrawRegion.isValid())
            {
                if (ignoreRedrawRegion)
                {
                    // Workaround for broken RedrawEvents (xnest)
                
                    XDestroyRegion(redrawRegion.get());
                    redrawRegion.invalidate();
                    
                    redrawRegion = XCreateRegion();
                    XRectangle r;
                    r.x = 0;
                    r.y = 0;
                    r.width = getWidth();
                    r.height = getHeight();
                    XUnionRectWithRegion(&r, redrawRegion.get(), redrawRegion.get());
                }
                GuiClipping::Holder clippingHolder(GuiWidgetSingletonData::getInstance()->getClipping(),
                                                   redrawRegion.get());

                eventListener->processGuiWidgetRedrawEvent(redrawRegion.get());

                XDestroyRegion(redrawRegion.get());
                redrawRegion.invalidate();
            }
            return EVENT_PROCESSED;
        }
        default: {
            return eventListener->processGuiWidgetEvent(event);
        }
    }
}

void GuiWidget::setPosition(const Position& p)
{
    int w = p.w - 2 * borderWidth;
    int h = p.h - 2 * borderWidth;

    if (w <= 0) { w = 2 * borderWidth; }
    if (h <= 0) { h = 2 * borderWidth; }
    
    XMoveResizeWindow(getDisplay(), wid, p.x, p.y, w, h);
    // this->position is set via ConfigureNotify event
    lastRequestedPosition = p;
}

Position GuiWidget::getAbsolutePosition() const
{
    Position rslt(this->position);
    Window child_return;
    XTranslateCoordinates(getDisplay(), getWid(), getGuiRoot()->getRootWid(), 0, 0, &rslt.x, 
                          &rslt.y, &child_return);
    return rslt;
}


void GuiWidget::show()
{
    XMapWindow(getDisplay(), wid);
//    GuiElement::show();
}

void GuiWidget::hide()
{
    XUnmapWindow(getDisplay(), wid);
//    GuiElement::hide();
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
    if (w > 0 && h > 0)
    {
        Display* display = getDisplay();
        WidgetId wid = getWid();
    
        XSetForeground(display, gcid, GuiRoot::getInstance()->getGuiColor03());
        XFillRectangle(display, wid, gcid,
                x, y, w, h);
    }
}

void GuiWidget::drawRaisedSurface(int x, int y, int w, int h, GuiColor color)
{
    if (w > 0 && h > 0)
    {
        Display* display = getDisplay();
        WidgetId wid = getWid();
    
        XSetForeground(display, gcid, color);
        XFillRectangle(display, wid, gcid,
                x, y, w, h);
    }
}


int GuiWidget::getRaisedBoxBorderWidth()
{
    return 1;
}

void GuiWidget::drawRaisedBox(int x, int y, int w, int h, GuiColor color)
{
    if (w > 0 && h > 0)
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
}

void GuiWidget::drawRaisedBox(int x, int y, int w, int h)
{
    drawRaisedBox(x, y, w, h, GuiRoot::getInstance()->getGuiColor03());
}


void GuiWidget::drawPressedBox(int x, int y, int w, int h, GuiColor color)
{
    if (w > 0 && h > 0)
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
}

void GuiWidget::drawPressedBox(int x, int y, int w, int h)
{
    drawPressedBox(x, y, w, h, GuiRoot::getInstance()->getGuiColor03());
}

void GuiWidget::drawArrow(int x, int y, int w, int h, const Direction::Type direct)
{
    if (w > 0 && h > 0)
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
}


void GuiWidget::drawInactiveSunkenFrame(int x, int y, int w, int h)
{
    if (w > 0 && h > 0)
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
}

void GuiWidget::drawActiveSunkenFrame(int x, int y, int w, int h)
{
    if (w > 0 && h > 0)
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
}

void GuiWidget::drawFrame(int x, int y, int w, int h)
{
    if (w > 0 && h > 0)
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
}

void GuiWidget::undrawFrame(int x, int y, int w, int h)
{
    if (w > 0 && h > 0)
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
}

void GuiWidget::drawDottedFrame(int x, int y, int w, int h)
{
    if (w > 0 && h > 0)
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
}



RawPtr<TextStyle> GuiWidget::getGuiTextStyle()
{
    return GuiWidgetSingletonData::getInstance()->getGuiTextStyle();
}

void GuiWidget::drawGuiTextWChars(int x, int y, const Char2bArray& wcharArray)
{
    XSetForeground(getDisplay(), gcid, getGuiTextStyle()->getColor());
    XDrawString16(getDisplay(), getWid(), 
                  gcid, x, y + getGuiTextStyle()->getLineAscent(), 
                  wcharArray.getPtr(0), wcharArray.getLength());
}

void GuiWidget::drawGuiTextUtf8String(int x, int y, const String& utf8String)
{
    drawGuiTextWChars(x, y, Char2bArray().setToUtf8String(utf8String));
}

void GuiWidget::drawGuiTextUtf8String(int x, int y, const char* utf8String)
{
    drawGuiTextWChars(x, y, Char2bArray().setToUtf8String(utf8String));
}

int GuiWidget::getGuiTextHeight()
{
    return getGuiTextStyle()->getLineHeight();
}

void GuiWidget::setWinGravity(int winGravity)
{
    XSetWindowAttributes at;
    at.win_gravity = winGravity;
    XChangeWindowAttributes(getDisplay(), getWid(), CWWinGravity, &at);
}


void GuiWidget::setBitGravity(int bitGravity)
{
    XSetWindowAttributes at;
    at.bit_gravity = bitGravity;
    XChangeWindowAttributes(getDisplay(), getWid(), CWBitGravity, &at);
}

void GuiWidget::scrollArea(int srcX, int srcY, unsigned int width, unsigned int height, int destX, int destY)
{
    scrollRequests.append(ScrollRequest(srcX, srcY, width, height, destX, destY));

    XCopyArea(getDisplay(), getWid(), getWid(), gcid,
                            srcX, srcY, width, height, destX, destY);
}

Region GuiWidget::calculateScrolledUpdateRegion(XRectangle* r)
{
    Region updateRegion = XCreateRegion();
    XUnionRectWithRegion(r, updateRegion, updateRegion);
    
    for (int i = 0; i < scrollRequests.getLength(); ++i)
    {
        ScrollRequest s = scrollRequests[i];
        
        XRectangle scrolledRect;
        {
            scrolledRect.x      = s.srcX;
            scrolledRect.y      = s.srcY;
            scrolledRect.width  = s.width;
            scrolledRect.height = s.height;
        }
        
        Region scrolledRegion = XCreateRegion();
        {
            XUnionRectWithRegion(&scrolledRect, scrolledRegion, scrolledRegion);
            
            XIntersectRegion(updateRegion, scrolledRegion, scrolledRegion);
            XSubtractRegion (updateRegion, scrolledRegion, updateRegion);
    
            XOffsetRegion(scrolledRegion, s.destX - s.srcX, s.destY - s.srcY);
            
            XUnionRegion(updateRegion, scrolledRegion, updateRegion);
        }
        XDestroyRegion(scrolledRegion);
    }
    return updateRegion;
}

