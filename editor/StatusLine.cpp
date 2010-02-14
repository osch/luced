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
#include <limits.h>

#include "String.hpp"

#include "StatusLine.hpp"
#include "util.hpp"

using namespace LucED;

StatusLine::StatusLine()
    : fileLength(0), selectionLength(0),
      lengthPos(0), line(0), column(0), pos(0), lineAndColumnWidth(0),
      hasMessage(false)
{
    RawPtr<TextStyle> guiTextStyle = GuiWidget::getGuiTextStyle();
    
    labelSWidth    = guiTextStyle->getTextWidth("S: ");
    labelPWidth    = guiTextStyle->getTextWidth("P: ");
    labelLWidth    = guiTextStyle->getTextWidth("L: ");
    labelCWidth    = guiTextStyle->getTextWidth("C: ");
    spaceWidth     = guiTextStyle->getTextWidth("  ");
    
    smallWidth  = guiTextStyle->getTextWidth("888");
    middleWidth = guiTextStyle->getTextWidth("888");
    bigWidth    = guiTextStyle->getTextWidth("888888");
}


void StatusLine::processGuiWidgetCreatedEvent()
{
    getGuiWidget()->addToXEventMask(ExposureMask|ButtonPressMask|ButtonReleaseMask|ButtonMotionMask);
    getGuiWidget()->setBackgroundColor(getGuiRoot()->getGuiColor03());
}


GuiElement::Measures StatusLine::internalGetDesiredMeasures()
{
    int statusHeight = GuiWidget::getGuiTextHeight() + 4;
    return Measures(0, statusHeight, 0, statusHeight, INT_MAX, statusHeight);
}

GuiWidget::ProcessingResult StatusLine::processGuiWidgetEvent(const XEvent* event)
{
    switch (event->type)
    {
        case GraphicsExpose:
            if (event->xgraphicsexpose.count > 0) {
                break;
            }
        case Expose: {
            if (event->xexpose.count > 0) {
                break;
            }
            this->drawArea();
            return GuiWidget::EVENT_PROCESSED;
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
    return getGuiWidget()->propagateEventToParentWidget(event);
}

void StatusLine::drawFileName()
{
    String displayText;
    if (hasMessage) {
        displayText = message;
    } else {
        displayText = fileName;
    }
    GuiWidget::GuiClipping c = getGuiWidget()->obtainGuiClipping(
            GuiWidget::getRaisedBoxBorderWidth(), GuiWidget::getRaisedBoxBorderWidth(), 
            getPosition().w - 2 * GuiWidget::getRaisedBoxBorderWidth(), getPosition().h - 2 * GuiWidget::getRaisedBoxBorderWidth());
    
    getGuiWidget()->drawGuiTextUtf8String(4, 2, displayText);

    RawPtr<TextStyle> guiTextStyle = GuiWidget::getGuiTextStyle();

    lengthPos = guiTextStyle->getTextWidth(displayText.toCString(), displayText.getLength())
                + 3 * guiTextStyle->getSpaceWidth();
}


void StatusLine::drawFileLength()
{
    if (!hasMessage)
    {
        GuiWidget::GuiClipping c = getGuiWidget()->obtainGuiClipping(
                GuiWidget::getRaisedBoxBorderWidth(), GuiWidget::getRaisedBoxBorderWidth(), 
                getPosition().w - 2 * GuiWidget::getRaisedBoxBorderWidth(), getPosition().h - 2 * GuiWidget::getRaisedBoxBorderWidth());
    
        char buffer[100];
        sprintf(buffer, "%ld bytes", fileLength);
    
        RawPtr<TextStyle> guiTextStyle = GuiWidget::getGuiTextStyle();
    
        getGuiWidget()->drawRaisedSurface(lengthPos, 2, guiTextStyle->getTextWidth(buffer, strlen(buffer)), GuiWidget::getGuiTextHeight());
        getGuiWidget()->drawGuiTextUtf8String(lengthPos, 2, buffer);
    }
}

int StatusLine::calcWidth(long value)
{
    char buffer[100];
    sprintf(buffer, "%ld", value);

    RawPtr<TextStyle> guiTextStyle = GuiWidget::getGuiTextStyle();

    return guiTextStyle->getTextWidth(buffer);
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
    getGuiWidget()->drawRaisedSurface(getPosition().w - GuiWidget::getRaisedBoxBorderWidth() - w,     2, w, GuiWidget::getGuiTextHeight());

    int p = getPosition().w - GuiWidget::getRaisedBoxBorderWidth() - width;
    
    if (selectionLength != 0) {
        sprintf(buffer, "S: %ld", selectionLength);
        getGuiWidget()->drawGuiTextUtf8String(p, 2, buffer);
        p += labelSWidth + util::maximum(smallWidth, calcWidth(selectionLength)) + spaceWidth;
    }
    {
        sprintf(buffer, "P: %ld", pos);
        getGuiWidget()->drawGuiTextUtf8String(p, 2, buffer);
        p += labelPWidth + util::maximum(bigWidth,    calcWidth(pos))      + 2*spaceWidth;

        sprintf(buffer, "L: %ld", line + 1);
        getGuiWidget()->drawGuiTextUtf8String(p, 2, buffer);
        p += labelLWidth + util::maximum(middleWidth, calcWidth(line + 1)) +  spaceWidth;

        sprintf(buffer, "C: %ld", column);
        getGuiWidget()->drawGuiTextUtf8String(p, 2, buffer);
        p += labelCWidth + util::maximum(smallWidth,  calcWidth(column));
    }
    lineAndColumnWidth = width;
}


void StatusLine::drawArea()
{
    getGuiWidget()->drawRaisedBox(0, 0, getPosition().w, getPosition().h);
    drawFileName();
    drawFileLength();
    drawLineAndColumn();
}

void StatusLine::setFileName(const String& fileName)
{
    this->fileName = fileName;
    if (getGuiWidget().isValid()) {
        drawArea();
    }
}

void StatusLine::setMessage(const String& message)
{
    this->message = message;
    hasMessage = true;
    drawArea();
}

void StatusLine::clearMessage()
{
    if (hasMessage) {
        hasMessage = false;
        drawArea();
    }
}

void StatusLine::setFileLength(long fileLength)
{
    if (fileLength < this->fileLength) {
        this->fileLength = fileLength;
        if (getGuiWidget().isValid()) {
            drawArea();
        }
    }
    else {
        this->fileLength = fileLength;
        if (getGuiWidget().isValid()) {
            drawFileLength();
        }
    }
}

void StatusLine::setCursorPositionData(CursorPositionData d)
{
    if (  this->line            != d.line 
       || this->column          != d.column 
       || this->pos             != d.pos 
       || this->selectionLength != d.selectionLength)
    {
        this->line = d.line;
        this->column = d.column;
        this->pos = d.pos;
        this->selectionLength = d.selectionLength;
        drawLineAndColumn();
    }
}

