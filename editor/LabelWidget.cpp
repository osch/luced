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

#include <stdio.h>
#include <string.h>

#include "LabelWidget.h"
#include "util.h"

using namespace LucED;

LabelWidget::LabelWidget(GuiWidget* parent, const string& leftText, const string& rightText)
    : GuiWidget(parent, 0, 0, 1, 1, 0),
      position(0, 0, 1, 1),
      leftText(leftText),
      rightText(rightText)
{
    addToXEventMask(ExposureMask|ButtonPressMask|ButtonReleaseMask|ButtonMotionMask);
    setBackgroundColor(getGuiRoot()->getGuiColor03());
}

void LabelWidget::setPosition(Position newPosition)
{
    if (position != newPosition) {
        GuiWidget::setPosition(newPosition);
        this->position = newPosition;
    }
}

GuiElement::Measures LabelWidget::getDesiredMeasures()
{
    int height = getGuiTextHeight() + 1;
    int width  = getGuiTextStyle()->getTextWidth(leftText.c_str(), leftText.length());
    return Measures(width, height, width, height, -1, height);
}

GuiElement::ProcessingResult LabelWidget::processEvent(const XEvent *event)
{
    if (GuiWidget::processEvent(event) == EVENT_PROCESSED) {
        return EVENT_PROCESSED;
    } else {
        
        switch (event->type) {
            
            case GraphicsExpose:
                if (event->xgraphicsexpose.count > 0) {
                    break;
                }
            case Expose: {
                if (event->xexpose.count > 0) {
                    break;
                }
                this->draw();
                return EVENT_PROCESSED;
            }

            case ButtonPress: {
                break;
            }

            case ButtonRelease: {
                break;
            }

            case MotionNotify: {
                break;
            }
        }
        return propagateEventToParentWidget(event);
    }
}

void LabelWidget::draw()
{
    drawRaisedSurface(0, 0, position.w, position.h);
    drawGuiText( 0, 0, leftText.c_str(), leftText.length());
}

