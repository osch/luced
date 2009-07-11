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

#ifndef GUI_WIDGET_MIXIN_HPP
#define GUI_WIDGET_MIXIN_HPP

#include "GuiWidget.hpp"
#include "Position.hpp"
#include "RawPtr.hpp"

namespace LucED
{

template
<
    class BaseClass
>
class GuiWidgetMixin : public BaseClass,
                       public GuiWidget::EventListener
{
public:
    bool isMapped() const {
        return guiWidget.isValid() && BaseClass::isVisible();
    }
    virtual void show() {
        if (guiWidget.isValid()) {
            guiWidget->show();
        }
        BaseClass::show();
    }
    
    virtual void hide() {
        if (guiWidget.isValid()) {
            guiWidget->hide();
        }
        BaseClass::hide();
    }

    int getWidth() const {
        return width;
    }
    int getHeight() const {
        return height;
    }

    virtual void setPosition(const Position& p)
    {
        if (p != BaseClass::getPosition()) {
            if (guiWidget.isValid()) {
                guiWidget->setPosition(p);
            }
            else {
                BaseClass::setPosition(p);
                this->width  = p.w - 2 * borderWidth;
                this->height = p.h - 2 * borderWidth;
            }
        }
    }

protected:
    GuiWidgetMixin(GuiElement::Visibility  defaultVisibility = GuiElement::VISIBLE, 
                   int                     borderWidth = 0)

        : BaseClass(defaultVisibility, 0, 0, 1 + 2*borderWidth, 1 + 2*borderWidth),
          borderWidth(borderWidth),
          width(1), 
          height(1)
    {} 

    RawPtr<GuiWidget> getGuiWidget() {
        return guiWidget;
    }

    template<class FocusManagerContainer
            >
    void adopt(RawPtr<GuiElement>           parentElement,
               RawPtr<GuiWidget>            parentWidget,
               const FocusManagerContainer& focusManagerContainer)
    {
        if (!BaseClass::wasAdopted(parentElement, guiWidget))
        {
            guiWidget = GuiWidget::create(parentWidget, 
                                          this, // GuiWidget::EventListener
                                          BaseClass::getPosition(),
                                          borderWidth);
        
            this->width  = guiWidget->getWidth();
            this->height = guiWidget->getHeight();
        
            BaseClass::adopt(parentElement, guiWidget, focusManagerContainer);
            if (BaseClass::isVisible()) {
                guiWidget->show();
            }
            processGuiWidgetCreatedEvent();
        }
    }

           ///////////////////////////////////////
protected: // Events for derived classes

    virtual void processGuiWidgetCreatedEvent()
    {}
    

           //////////////////////////////////////////
protected: // GuiWidget::EventListener methods

    virtual GuiWidget::ProcessingResult processGuiWidgetEvent(const XEvent* event)
    {
        return GuiWidget::NOT_PROCESSED;
    }
    
    virtual void processGuiWidgetNewPositionEvent(const Position& newPosition)
    {
        BaseClass::setPosition(newPosition);
    
        this->width  = guiWidget->getWidth();
        this->height = guiWidget->getHeight();
    }

private:
    GuiWidget::Ptr guiWidget;
    int            borderWidth;
    int            width;
    int            height;
};

} // namespace LucED

#endif // GUI_WIDGET_MIXIN_HPP
