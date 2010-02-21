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

#ifndef GUI_WIDGET_HPP
#define GUI_WIDGET_HPP

#include "headers.hpp"
#include "types.hpp"
#include "WidgetId.hpp"
#include "GuiElement.hpp"
#include "GuiColor.hpp"
#include "GuiRoot.hpp"
#include "TextStyle.hpp"
#include "OwningPtr.hpp"
#include "KeyMapping.hpp"
#include "RawPtr.hpp"
#include "ObjectArray.hpp"
#include "KeyPressEvent.hpp"
#include "RawPointable.hpp"
#include "FocusManager.hpp"
#include "HeapObject.hpp"
#include "OwningPtr.hpp"
#include "Char2bArray.hpp"

namespace LucED
{

class GuiWidget : public HeapObject
{
public:
    typedef OwningPtr<GuiWidget> Ptr;
    
    enum ProcessingResult
    {
        NOT_PROCESSED = 0,
        EVENT_PROCESSED = 1
    };

    class EventListener : public RawPointable
    {
    public:
        virtual ProcessingResult processGuiWidgetEvent(const XEvent* event) = 0; // TODO: this X11 specific method should vanish some day...
        virtual void             processGuiWidgetNewPositionEvent(const Position& newPosition) = 0;
    };
    
    static Ptr create(RawPtr<GuiWidget>     parentWidget, 
                      RawPtr<EventListener> eventListener,
                      const Position&       position, 
                      int                   borderWidth = 0) {
        return Ptr(new GuiWidget(parentWidget, eventListener, position, borderWidth));
    }

    class EventProcessorAccess;
    
    class EventRegistration
    {
    private:
        friend class GuiWidget;
        friend class EventProcessorAccess;
        friend class EventDispatcher;

        EventRegistration(RawPtr<GuiWidget> guiWidget, WidgetId wid) : guiWidget(guiWidget), wid(wid) {}
        RawPtr<GuiWidget> guiWidget;
        WidgetId wid;
    };
    
    
    virtual ProcessingResult processEvent(const XEvent* event);
    virtual void setPosition(const Position& newPosition);
    Position getAbsolutePosition() const;
    
    virtual void show();
    virtual void hide();
    
    
    void setWinGravity(int winGravity);

    class EventProcessorAccess
    {
        friend class PasteDataReceiver;
        friend class SelectionOwner;
        
        static void addToXEventMaskForGuiWidget(GuiWidget* w, long eventMask) {
            w->addToXEventMask(eventMask);
        }
        static WidgetId getGuiWidgetWid(GuiWidget* w) {
            return w->getWid();
        }
        static GuiWidget::EventRegistration createEventRegistration(GuiWidget* guiWidget, WidgetId wid) {
            return GuiWidget::EventRegistration(guiWidget, wid);
        }
    };
    
protected:
    GuiWidget(RawPtr<GuiWidget>     parentWidget,
              RawPtr<EventListener> eventListener,
              const Position&       position,
              int borderWidth);
    
    virtual ~GuiWidget();

public:
    static WidgetId getRootWid() { return GuiRoot::getInstance()->getRootWid(); }
    static Display* getDisplay() { return GuiRoot::getInstance()->getDisplay(); }
    static GuiRoot* getGuiRoot() { return GuiRoot::getInstance(); }
    
    class GuiClipping
    {
    public:
        ~GuiClipping();
    private:
        friend class GuiWidget;
        GuiClipping(RawPtr<GuiWidget> guiWidget) : guiWidget(guiWidget) {}
        RawPtr<GuiWidget> guiWidget;
    };
    friend class GuiClipping;
    
    GuiClipping obtainGuiClipping(int x, int y, int w, int h);
    void setBitGravity(int bitGravity);

    int getWidth() const {
        return width;
    }
    int getHeight() const {
        return height;
    }

protected:
    RawPtr<GuiWidget> getParentWidget() const {
        return parent;
    }
    
public:
    WidgetId getWid() const {
        return wid;
    }

    virtual void setBackgroundColor(GuiColor color);
    virtual void setBorderColor(GuiColor color);

public:
    void addToXEventMask(long eventMask);
    void removeFromXEventMask(long eventMask);
    
    ProcessingResult propagateEventToParentWidget(const XEvent* event) {
        if (parent.isValid()) {
            return parent->processEvent(event);
        } else {
            return NOT_PROCESSED;
        }
    }
    
    static RawPtr<TextStyle> getGuiTextStyle();
    static int getGuiTextHeight();
    static int getRaisedBoxBorderWidth();

    void drawLine(int x, int y, int dx, int dy);
    void drawLine(int x, int y, int dx, int dy, GuiColor color);
    void drawRaisedSurface(int x, int y, int w, int h);
    void drawRaisedSurface(int x, int y, int w, int h, GuiColor color);
    void drawRaisedBox(int x, int y, int w, int h, GuiColor color);
    void drawRaisedBox(int x, int y, int w, int h);
    void drawPressedBox(int x, int y, int w, int h, GuiColor color);
    void drawPressedBox(int x, int y, int w, int h);
    void drawArrow(int x, int y, int w, int h, const Direction::Type direct);


    void drawGuiTextWChars    (int x, int y, const Char2bArray& wcharArray);
    void drawGuiTextUtf8String(int x, int y, const String&      utf8String);
    void drawGuiTextUtf8String(int x, int y, const char*        utf8String);

    void drawActiveSunkenFrame(int x, int y, int w, int h);
    void drawInactiveSunkenFrame(int x, int y, int w, int h);
    void drawFrame(int x, int y, int w, int h);
    void undrawFrame(int x, int y, int w, int h);
    void drawDottedFrame(int x, int y, int w, int h);
    
    
private:
    int  borderWidth;
    bool isTopWindow;
    WidgetId wid;
    long eventMask;
    Position position;
    GC gcid;

    RawPtr<GuiWidget> parent;
    RawPtr<EventListener> eventListener;
    
    int width;
    int height;
};


} // namespace LucED

#endif // GUI_WIDGET_HPP
