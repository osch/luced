/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, oliver at luced dot de
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
#include "String.h"

#include "StatusLine.h"
#include "util.h"

using namespace LucED;

StatusLine::StatusLine(GuiWidget* parent)
    : GuiWidget(parent, 0, 0, 1, 1, 0),
      position(0, 0, 1, 1),
      slotForSetLineAndColumn(this, &StatusLine::setLineAndColumn),
      slotForSetFileName(     this, &StatusLine::setFileName),
      slotForSetFileLength(   this, &StatusLine::setFileLength),
      lengthPos(0), line(0), column(0), lineAndColumnWidth(0)
{
    addToXEventMask(ExposureMask|ButtonPressMask|ButtonReleaseMask|ButtonMotionMask);
    setBackgroundColor(getGuiRoot()->getGuiColor03());
}

void StatusLine::setPosition(Position newPosition)
{
    if (position != newPosition) {
        GuiWidget::setPosition(newPosition);
        this->position = newPosition;
    }
}

GuiElement::Measures StatusLine::getDesiredMeasures()
{
    int statusHeight = getGuiTextHeight() + 4;
    return Measures(0, statusHeight, 0, statusHeight, INT_MAX, statusHeight);
}

GuiElement::ProcessingResult StatusLine::processEvent(const XEvent *event)
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
                this->drawArea();
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

void StatusLine::drawFileName()
{
    GuiClipping c = obtainGuiClipping(
            getRaisedBoxBorderWidth(), getRaisedBoxBorderWidth(), 
            position.w - 2 * getRaisedBoxBorderWidth(), position.h - 2 * getRaisedBoxBorderWidth());
    
    drawGuiText(  4, 2, fileName.toCString(), fileName.getLength());
    lengthPos = getGuiTextStyle()->getTextWidth(fileName.toCString(), fileName.getLength())
            + 3 * getGuiTextStyle()->getSpaceWidth();
}


void StatusLine::drawFileLength()
{
    GuiClipping c = obtainGuiClipping(
            getRaisedBoxBorderWidth(), getRaisedBoxBorderWidth(), 
            position.w - 2 * getRaisedBoxBorderWidth(), position.h - 2 * getRaisedBoxBorderWidth());

    char buffer[30];
    sprintf(buffer, "%ld bytes", fileLength);
    drawRaisedSurface(lengthPos, 2, getGuiTextStyle()->getTextWidth(buffer, strlen(buffer)), getGuiTextHeight());
    drawGuiText(      lengthPos, 2, buffer, strlen(buffer));
}

void StatusLine::drawLineAndColumn()
{
    char buffer[100];
    sprintf(buffer, "  L: %ld  C: %ld  ", line + 1, column);
    int len = strlen(buffer);
    int width = getGuiTextStyle()->getTextWidth(buffer, len);
    int w = util::maximum(width, lineAndColumnWidth);
    drawRaisedSurface(position.w - getRaisedBoxBorderWidth() - w,     2, w, getGuiTextHeight());
    drawGuiText(      position.w - getRaisedBoxBorderWidth() - width, 2, buffer, len);
    lineAndColumnWidth = width;
}


void StatusLine::drawArea()
{
    drawRaisedBox(0, 0, position.w, position.h);
    drawFileName();
    drawFileLength();
    drawLineAndColumn();
}

void StatusLine::setFileName(const String& fileName)
{
    this->fileName = fileName;
    drawArea();
}

void StatusLine::setFileLength(long fileLength)
{
    this->fileLength = fileLength;
    drawFileLength();
}

void StatusLine::setLineAndColumn(long line, long column)
{
    if (this->line != line || this->column != column)
    {
        this->line = line;
        this->column = column;
        drawLineAndColumn();
    }
}

