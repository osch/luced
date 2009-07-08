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

#include "FocusableWidget.hpp"

using namespace LucED;


void FocusableWidget::show()
{
    if (guiWidget.isValid())
    {
        guiWidget->show();
    }
    FocusableElement::show();
}

void FocusableWidget::hide()
{
    if (guiWidget.isValid())
    {
        guiWidget->hide();
    }
    FocusableElement::hide();
}


void FocusableWidget::adopt(RawPtr<GuiElement>   parentElement,
                            RawPtr<GuiWidget>    parentWidget,
                            RawPtr<FocusManager> focusManagerForThis,
                            RawPtr<FocusManager> focusManagerForChilds)
{
    if (!wasAdopted(parentElement, guiWidget))
    {
        guiWidget = GuiWidget::create(parentWidget, 
                                      this, // GuiWidget::EventListener
                                      getPosition(),
                                      borderWidth);
    
        this->width  = guiWidget->getWidth();
        this->height = guiWidget->getHeight();
    
        FocusableElement::adopt(parentElement, guiWidget, focusManagerForThis,
                                                          focusManagerForChilds);
        if (isVisible()) {
            guiWidget->show();
        }
        processGuiWidgetCreatedEvent();
    }
}


void FocusableWidget::setPosition(const Position& p)
{
    if (p != getPosition()) {
        if (guiWidget.isValid()) {
            guiWidget->setPosition(p);
        }
        else {
            FocusableElement::setPosition(p);
            this->width  = p.w - 2 * borderWidth;
            this->height = p.h - 2 * borderWidth;
        }
    }
}

void FocusableWidget::processGuiWidgetNewPositionEvent(const Position& newPosition)
{
    FocusableElement::setPosition(newPosition);

    this->width  = guiWidget->getWidth();
    this->height = guiWidget->getHeight();
}
