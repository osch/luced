/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2008 Oliver Schmidt, oliver at luced dot de
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
#include "ActionMethods.hpp"
#include "KeyPressEvent.hpp"
#include "ActionKeyConfig.hpp"

namespace LucED
{

class EventDispatcher;
class GuiRoot;
class TopWin;

class GuiWidget : public GuiElement
{
public:
    typedef OwningPtr<GuiWidget> Ptr;
    typedef OwningPtr<const GuiWidget> ConstPtr;
    
    class EventRegistration
    {
    private:
        friend class GuiWidget;
        friend class EventDispatcher;
        friend class GuiWidgetAccessForEventProcessors;
        EventRegistration(RawPtr<GuiWidget> guiWidget, WidgetId wid) : guiWidget(guiWidget), wid(wid) {}
        RawPtr<GuiWidget> guiWidget;
        WidgetId wid;
    };
    
    virtual ProcessingResult processEvent(const XEvent *event);
    virtual void setPosition(Position newPosition);
    virtual void setSize(int width, int height);
    virtual void treatNewWindowPosition(Position newPosition) {}
    const Position& getPosition() { return position; }
    Position getAbsolutePosition();
    
    virtual void show();
    virtual void hide();
    
    enum FocusType {
        NO_FOCUS,
        NORMAL_FOCUS,
        TOTAL_FOCUS
    };

    virtual bool isFocusable() { return false; }
    virtual FocusType getFocusType() { return NO_FOCUS; }
    virtual void treatFocusIn() {}
    virtual void treatFocusOut() {}
    virtual ProcessingResult processKeyboardEvent(const KeyPressEvent& keyPressEvent) { return NOT_PROCESSED; }

    virtual bool handleHighPriorityKeyPress(const KeyPressEvent& keyPressEvent) { return false; }
    virtual bool handleLowPriorityKeyPress (const KeyPressEvent& keyPressEvent) { return false; }

    virtual void notifyAboutHotKeyEventForOtherWidget() {}

    virtual void treatLostHotKeyRegistration(const KeyMapping::Id& id) {}
    virtual void treatNewHotKeyRegistration(const KeyMapping::Id& id) {}
    virtual void treatHotKeyEvent(const KeyMapping::Id& id) {}
    
    
    void setNextFocusWidget(GuiWidget* n) { nextFocusWidget = n; n->prevFocusWidget = this; }
    GuiWidget* getNextFocusWidget() { return nextFocusWidget; }
    GuiWidget* getPrevFocusWidget() { return prevFocusWidget; }
    
    void setWinGravity(int winGravity);
    
    virtual void addActionMethods(ActionMethods::Ptr methods);
    
    virtual bool invokeActionMethod(ActionId actionId);
    virtual bool hasActionMethod(ActionId actionId);

    virtual ActionKeyConfig::Ptr getActionKeyConfig();

protected:

    GuiWidget(int x, int y, unsigned int width, unsigned int height, unsigned border_width);
    
    GuiWidget(GuiWidget* parent,
            int x, int y, unsigned int width, unsigned int height, unsigned border_width);

    virtual ~GuiWidget();

    static WidgetId getRootWid();
    static Display* getDisplay();
    static EventDispatcher* getEventDispatcher();
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

protected:
    virtual void requestFocusFor(GuiWidget* w) {
        if (parent != NULL) parent->requestFocusFor(w);
    }
    virtual void requestHotKeyRegistrationFor(const KeyMapping::Id& id, GuiWidget* w) {
        if (parent != NULL) parent->requestHotKeyRegistrationFor(id, w);
    }
    virtual void requestRemovalOfHotKeyRegistrationFor(const KeyMapping::Id& id, GuiWidget* w) {
        if (parent != NULL) parent->requestRemovalOfHotKeyRegistrationFor(id, w);
    }

public:
    WidgetId getWid() const {
        return wid;
    }

    virtual void setBackgroundColor(GuiColor color);
    virtual void setBorderColor(GuiColor color);

protected:
    
    
    void addToXEventMask(long eventMask);
    void removeFromXEventMask(long eventMask);
    
    ProcessingResult propagateEventToParentWidget(const XEvent *event) {
        if (parent.isValid()) {
            return parent->processEvent(event);
        } else {
            return NOT_PROCESSED;
        }
    }
    
    TextStyle* getGuiTextStyle();
    void drawLine(int x, int y, int dx, int dy);
    void drawLine(int x, int y, int dx, int dy, GuiColor color);
    void drawRaisedSurface(int x, int y, int w, int h);
    void drawRaisedSurface(int x, int y, int w, int h, GuiColor color);
    void drawRaisedBox(int x, int y, int w, int h, GuiColor color);
    void drawRaisedBox(int x, int y, int w, int h);
    int getRaisedBoxBorderWidth();
    void drawPressedBox(int x, int y, int w, int h, GuiColor color);
    void drawPressedBox(int x, int y, int w, int h);
    void drawArrow(int x, int y, int w, int h, const Direction::Type direct);
    void drawGuiText(int x, int y, const char* ptr, long length);
    void drawGuiText(int x, int y, const String& ptr) {
        drawGuiText(x, y, ptr.toCString(), ptr.getLength());
    }
    void drawGuiText(int x, int y, const char* ptr) {
        drawGuiText(x, y, ptr, strlen(ptr));
    }
    void drawActiveSunkenFrame(int x, int y, int w, int h);
    void drawInactiveSunkenFrame(int x, int y, int w, int h);
    void drawFrame(int x, int y, int w, int h);
    void undrawFrame(int x, int y, int w, int h);
    void drawDottedFrame(int x, int y, int w, int h);
    int getGuiTextHeight();
    
    
private:
    friend class GuiWidgetAccessForEventProcessors;
    
    bool isTopWindow;
    WidgetId wid;
    long eventMask;
    Position position;
    GC gcid;

    WeakPtr<GuiWidget> parent;

    WeakPtr<GuiWidget> nextFocusWidget;
    WeakPtr<GuiWidget> prevFocusWidget;
    ObjectArray<ActionMethods::Ptr> actionMethods;
};

class GuiWidgetAccessForEventProcessors
{
protected:
    void addToXEventMaskForGuiWidget(GuiWidget* w, long eventMask) {
        w->addToXEventMask(eventMask);
    }
    WidgetId getGuiWidgetWid(GuiWidget* w) {
        return w->getWid();
    }
    GuiWidget::EventRegistration createEventRegistration(GuiWidget* guiWidget, WidgetId wid) {
        return GuiWidget::EventRegistration(guiWidget, wid);
    }
};

} // namespace LucED

#endif // GUI_WIDGET_HPP
