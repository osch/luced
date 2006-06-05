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

#ifndef GUIWIDGET_H
#define GUIWIDGET_H

#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xutil.h>

#include "types.h"
#include "GuiElement.h"
#include "GuiColor.h"
#include "GuiRoot.h"
#include "TextStyle.h"
#include "OwningPtr.h"


namespace LucED {

class EventDispatcher;
class GuiRoot;

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
        EventRegistration(GuiWidget* guiWidget, Window wid) : guiWidget(guiWidget), wid(wid) {}
        GuiWidget* guiWidget;
        Window wid;
    };
    
    virtual bool processEvent(const XEvent *event);
    virtual void setPosition(Position newPosition);
    virtual void treatNewWindowPosition(Position newPosition) {}
    const Position& getPosition() { return position; }
    
    virtual void show();
    virtual void hide();
    
    
protected:

    GuiWidget(int x, int y, unsigned int width, unsigned int height, unsigned border_width);
    
    GuiWidget(GuiWidget* parent,
            int x, int y, unsigned int width, unsigned int height, unsigned border_width);

    virtual ~GuiWidget();

    static Window getRootWid();
    static Display* getDisplay();
    static EventDispatcher* getEventDispatcher();
    static GuiRoot* getGuiRoot() { return GuiRoot::getInstance(); }
    
    class GuiClipping
    {
    public:
        ~GuiClipping();
    private:
        friend class GuiWidget;
        GuiClipping(GuiWidget* guiWidget) : guiWidget(guiWidget) {}
        GuiWidget* guiWidget;
    };
    friend class GuiClipping;
    
    GuiClipping obtainGuiClipping(int x, int y, int w, int h);

public:
    Window getWid() const {
        return wid;
    }
protected:
    
    void setBackgroundColor(GuiColor color);
    void setBorderColor(GuiColor color);
    
    void addToXEventMask(long eventMask);
    void removeFromXEventMask(long eventMask);
    
    bool propagateEventToParentWidget(const XEvent *event) {
        if (parent != NULL) {
            return parent->processEvent(event);
        } else {
            return false;
        }
    }
    
    TextStyle* getGuiTextStyle();
    void drawRaisedSurface(int x, int y, int w, int h);
    void drawRaisedBox(int x, int y, int w, int h, GuiColor color);
    void drawRaisedBox(int x, int y, int w, int h);
    int getRaisedBoxBorderWidth();
    void drawPressedBox(int x, int y, int w, int h, GuiColor color);
    void drawPressedBox(int x, int y, int w, int h);
    void drawArrow(int x, int y, int w, int h, const Direction::Type direct);
    void drawGuiText(int x, int y, const char* ptr, long length);
    void drawGuiText(int x, int y, const string& ptr) {
        drawGuiText(x, y, ptr.c_str(), ptr.length());
    }
    int getGuiTextHeight();
    
private:
    friend class GuiWidgetAccessForEventProcessors;
    
    bool isTopWindow;
    Window wid;
    long eventMask;
    GuiWidget *parent;
    Position position;
};

class GuiWidgetAccessForEventProcessors
{
protected:
    void addToXEventMaskForGuiWidget(GuiWidget* w, long eventMask) {
        w->addToXEventMask(eventMask);
    }
    Window getGuiWidgetWid(GuiWidget* w) {
        return w->getWid();
    }
    GuiWidget::EventRegistration createEventRegistration(GuiWidget* guiWidget, Window wid) {
        return GuiWidget::EventRegistration(guiWidget, wid);
    }
};

} // namespace LucED


#endif // GUIWIDGET_H
