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

#include "NonFocusableWidget.hpp"

using namespace LucED;


void NonFocusableWidget::show()
{
    if (guiWidget.isValid())
    {
        guiWidget->show();
    }
    GuiElement::show();
}

void NonFocusableWidget::hide()
{
    if (guiWidget.isValid())
    {
        guiWidget->hide();
    }
    GuiElement::hide();
}

void NonFocusableWidget::adopt(RawPtr<GuiElement>   parentElement,
                               RawPtr<GuiWidget>    parentWidget,
                               RawPtr<FocusManager> focusManager)
{
    guiWidget = GuiWidget::create(parentWidget, 
                                  this, // GuiWidget::EventListener
                                  getPosition(), 
                                  borderWidth);
    this->width  = guiWidget->getWidth();
    this->height = guiWidget->getHeight();

    GuiElement::adopt(parentElement, guiWidget, focusManager);

    if (isVisible()) {
        guiWidget->show();
    }
    processGuiWidgetCreatedEvent();
}


void NonFocusableWidget::setPosition(const Position& p)
{
    if (p != getPosition()) {
        if (guiWidget.isValid()) {
            guiWidget->setPosition(p);
        }
        else {
            GuiElement::setPosition(p);
            this->width  = p.w - 2 * borderWidth;
            this->height = p.h - 2 * borderWidth;
        }
    }
}

void NonFocusableWidget::processGuiWidgetNewPositionEvent(const Position& newPosition)
{
    GuiElement::setPosition(newPosition);

    this->width  = guiWidget->getWidth();
    this->height = guiWidget->getHeight();
}


GuiWidget::ProcessingResult NonFocusableWidget::processGuiWidgetEvent(const XEvent* event)
{
    return GuiWidget::NOT_PROCESSED;
}


