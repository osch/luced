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
#include "GuiLayoutTable.h"
#include "GuiLayoutWidget.h"
#include "GuiLayoutColumn.h"
#include "GuiLayoutRow.h"

using namespace LucED;

EditorTopWin::EditorTopWin(TextData::Ptr textData, TextStyles::Ptr textStyles, HilitingBuffer::Ptr hilitingBuffer,
        int x, int y, unsigned int width, unsigned int height)
    : TopWin(x, y, width, height, 0),
      layout(GuiLayoutColumn::create())
{
    addToXEventMask(ButtonPressMask);
    
    statusLine = StatusLine::create(this);
    layout->addElement(statusLine);
    
    GuiLayoutTable::Ptr tableLayout = GuiLayoutTable::create(2, 2);
    layout->addElement(tableLayout);
    
    textEditor = TextEditorWidget::create(this, textData, textStyles, hilitingBuffer);

    GuiLayoutColumn::Ptr c2 = GuiLayoutColumn::create();
    GuiLayoutRow::Ptr    r2 = GuiLayoutRow::create();
    
    tableLayout->setElement(0, 0, c2);
    c2->addElement(r2);
    c2->addElement(GuiLayoutWidget::create(this, 1, 1, 1, 1, -1, 1));

    r2->addElement(textEditor);
    r2->addElement(GuiLayoutWidget::create(this, 1, 1, 1, 1, 1, -1));

    scrollBarV = ScrollBar::create(this, Orientation::VERTICAL);
    tableLayout->setElement(0, 1, scrollBarV);
    
    scrollBarH = ScrollBar::create(this, Orientation::HORIZONTAL);
    tableLayout->setElement(1, 0, scrollBarH);
    
    layout->setPosition(Position(0, 0, width, height));
    
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
    layout->setPosition(Position(0, 0, newPosition.w, newPosition.h));
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




