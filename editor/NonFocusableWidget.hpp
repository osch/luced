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

#ifndef NON_FOCUSABLE_WIDGET_HPP
#define NON_FOCUSABLE_WIDGET_HPP

#include "GuiWidget.hpp"
#include "Position.hpp"
#include "RawPtr.hpp"

namespace LucED
{

class NonFocusableWidget : public GuiElement,
                           public GuiWidget::EventListener
{
public:
    virtual void show();
    virtual void hide();

    bool isMapped() const {
        return guiWidget.isValid() && isVisible();
    }

    virtual void setPosition(const Position& p);

    virtual void adopt(RawPtr<GuiElement>   parentElement,
                       RawPtr<GuiWidget>    parentWidget,
                       RawPtr<FocusManager> focusManager);

    int getWidth() const {
        return width;
    }
    int getHeight() const {
        return height;
    }
protected:
    NonFocusableWidget(Visibility                       defaultVisibility = VISIBLE, 
                       int                              borderWidth = 0)
        : GuiElement(defaultVisibility, 0, 0, 1 + borderWidth, 1 + borderWidth),
          borderWidth(borderWidth),
          width(1), height(1)
    {}

    RawPtr<GuiWidget> getGuiWidget() {
        return guiWidget;
    }

    virtual void processGuiWidgetCreatedEvent() = 0;

protected: // GuiWidget::EventListener methods
    virtual GuiWidget::ProcessingResult processGuiWidgetEvent(const XEvent* event);
    virtual void                        processGuiWidgetNewPositionEvent(const Position& newPosition);

private:
    int                              borderWidth;
    GuiWidget::Ptr                   guiWidget;
    int                              width;
    int                              height;
};

} // namespace LucED

#endif // NON_FOCUSABLE_WIDGET_HPP
