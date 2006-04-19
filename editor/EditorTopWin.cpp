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

#include "EditorTopWin.h"
#include "GlobalConfig.h"

using namespace LucED;

EditorTopWin::EditorTopWin(TextData::Ptr textData, TextStyles::Ptr textStyles, HilitingBuffer::Ptr hilitingBuffer,
        int x, int y, unsigned int width, unsigned int height)
    : TopWin(x, y, width, height, 0)
{
    int scrollBarWidth = GlobalConfig::getInstance()->getScrollBarWidth();
    int statusHeight = getGuiTextHeight() + 4;
    
    addToXEventMask(ButtonPressMask);
    
    
    statusLine = StatusLine::create(this, 0, 0, width, statusHeight);

    textEditor = TextEditorWidget::create(this, textData, textStyles, hilitingBuffer,
            0, statusHeight, width - scrollBarWidth, height - scrollBarWidth - statusHeight);

    scrollBarV = ScrollBar::create(this, width - scrollBarWidth, statusHeight, scrollBarWidth, height - scrollBarWidth - statusHeight + 1);
    scrollBarH = ScrollBar::create(this, 0, height - scrollBarWidth, width - scrollBarWidth + 1, scrollBarWidth);
    
    textData->registerFileNameListener(statusLine->slotForSetFileName);
    textData->registerLengthListener(statusLine->slotForSetFileLength);
    textEditor->registerLineAndColumnListener(statusLine->slotForSetLineAndColumn);
    
    scrollBarV->setChangedValueCallback(textEditor->slotForVerticalScrollBarChangedValue);
    scrollBarH->setChangedValueCallback(textEditor->slotForHorizontalScrollBarChangedValue);

    scrollBarV->setScrollStepCallback(textEditor->slotForScrollStepV);
    scrollBarH->setScrollStepCallback(textEditor->slotForScrollStepH);

    textEditor->setScrollBarVerticalValueRangeChangedCallback(  scrollBarV->slotForSetValueRange);
    textEditor->setScrollBarHorizontalValueRangeChangedCallback(scrollBarH->slotForSetValueRange);
    
    textEditor->show();
    scrollBarV->show();
    scrollBarH->show();
    statusLine->show();
}

void EditorTopWin::treatNewWindowPosition(Position newPosition)
{
    int scrollBarWidth = GlobalConfig::getInstance()->getScrollBarWidth();
    int statusHeight = getGuiTextHeight() + 4;
    
    statusLine->setPosition(Position(
            0, 0, newPosition.w, statusHeight));

    textEditor->setPosition(Position(
            0, statusHeight, newPosition.w - scrollBarWidth, newPosition.h - scrollBarWidth - statusHeight));

    scrollBarV->setPosition(Position(
            newPosition.w - scrollBarWidth, statusHeight, scrollBarWidth, newPosition.h - scrollBarWidth - statusHeight + 1));
            
    scrollBarH->setPosition(Position(
            0, newPosition.h - scrollBarWidth, newPosition.w - scrollBarWidth + 1, scrollBarWidth));
    
}

bool EditorTopWin::processKeyboardEvent(const XEvent *event)
{
    return textEditor->processKeyboardEvent(event);
}


bool EditorTopWin::processEvent(const XEvent *event)
{
    if (TopWin::processEvent(event)) {
        return true;
    } else {
        switch (event->type)
        {
            case ButtonPress: {
                if (event->xbutton.button == Button4
                 || event->xbutton.button == Button5)
                {
                    textEditor->processEvent(event);
                    return true;
                }
            }
        }
        return false;
    }
}


void EditorTopWin::treatFocusIn()
{
    textEditor->treatFocusIn();
}


void EditorTopWin::treatFocusOut()
{
    textEditor->treatFocusOut();
}




