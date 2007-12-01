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

#include <stdio.h>
#include "String.hpp"

#include "StatusLine.hpp"
#include "util.hpp"

using namespace LucED;

StatusLine::StatusLine(GuiWidget* parent)
    : GuiWidget(parent, 0, 0, 1, 1, 0),
      position(0, 0, 1, 1),
      fileLength(0), selectionLength(0),
      lengthPos(0), line(0), column(0), pos(0), lineAndColumnWidth(0)
{
    addToXEventMask(ExposureMask|ButtonPressMask|ButtonReleaseMask|ButtonMotionMask);
    setBackgroundColor(getGuiRoot()->getGuiColor03());
    
    
    labelSWidth    = getGuiTextStyle()->getTextWidth("S: ");
    labelPWidth    = getGuiTextStyle()->getTextWidth("P: ");
    labelLWidth    = getGuiTextStyle()->getTextWidth("L: ");
    labelCWidth    = getGuiTextStyle()->getTextWidth("C: ");
    spaceWidth     = getGuiTextStyle()->getTextWidth("  ");
    
    smallWidth  = getGuiTextStyle()->getTextWidth("888");
    middleWidth = getGuiTextStyle()->getTextWidth("888");
    bigWidth    = getGuiTextStyle()->getTextWidth("888888");
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

    char buffer[100];
    sprintf(buffer, "%ld bytes", fileLength);

    drawRaisedSurface(lengthPos, 2, getGuiTextStyle()->getTextWidth(buffer, strlen(buffer)), getGuiTextHeight());
    drawGuiText(      lengthPos, 2, buffer, strlen(buffer));
}

int StatusLine::calcWidth(long value)
{
    char buffer[100];
    sprintf(buffer, "%ld", value);
    return getGuiTextStyle()->getTextWidth(buffer);
}

void StatusLine::drawLineAndColumn()
{
    char buffer[200];
    
    int width = 0;
    
    if (selectionLength != 0) {
        width += labelSWidth + util::maximum(smallWidth, calcWidth(selectionLength)) + spaceWidth;
        
        //sprintf(buffer,             "P: %-6ld    L: %-5ld  C: %-3ld  ", pos, line + 1, column);
    }
    {
        width += labelPWidth + util::maximum(bigWidth,    calcWidth(pos))      + 2*spaceWidth;
        width += labelLWidth + util::maximum(middleWidth, calcWidth(line + 1)) +   spaceWidth;
        width += labelCWidth + util::maximum(smallWidth,  calcWidth(column))   +   spaceWidth;
    }
        //sprintf(buffer, "S: %-6ld    P: %-6ld    L: %-5ld  C: %-3ld  ", selectionLength, pos, line + 1, column);
    
    int w = util::maximum(width, lineAndColumnWidth);
    drawRaisedSurface(position.w - getRaisedBoxBorderWidth() - w,     2, w, getGuiTextHeight());

    int p = position.w - getRaisedBoxBorderWidth() - width;
    
    if (selectionLength != 0) {
        sprintf(buffer, "S: %ld", selectionLength);
        drawGuiText(p, 2, buffer);
        p += labelSWidth + util::maximum(smallWidth, calcWidth(selectionLength)) + spaceWidth;
    }
    {
        sprintf(buffer, "P: %ld", pos);
        drawGuiText(p, 2, buffer);
        p += labelPWidth + util::maximum(bigWidth,    calcWidth(pos))      + 2*spaceWidth;

        sprintf(buffer, "L: %ld", line + 1);
        drawGuiText(p, 2, buffer);
        p += labelLWidth + util::maximum(middleWidth, calcWidth(line + 1)) +  spaceWidth;

        sprintf(buffer, "C: %ld", column);
        drawGuiText(p, 2, buffer);
        p += labelCWidth + util::maximum(smallWidth,  calcWidth(column));
    }
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
    if (fileLength < this->fileLength) {
        this->fileLength = fileLength;
        drawArea();
    }
    else {
        this->fileLength = fileLength;
        drawFileLength();
    }
}

void StatusLine::setCursorPositionData(CursorPositionData d)
{
    if (this->line != d.line || this->column != d.column || this->selectionLength != d.selectionLength)
    {
        this->line = d.line;
        this->column = d.column;
        this->pos = d.pos;
        this->selectionLength = d.selectionLength;
        drawLineAndColumn();
    }
}

