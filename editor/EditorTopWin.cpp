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

#include <X11/keysym.h>
#include <X11/Xatom.h>

#include "util.h"
#include "EditorTopWin.h"
#include "GlobalConfig.h"
#include "GuiLayoutWidget.h"
#include "GuiLayoutColumn.h"
#include "GuiLayoutRow.h"
#include "GuiLayoutSpacer.h"

using namespace LucED;

EditorTopWin::EditorTopWin(TextData::Ptr textData, TextStyles::Ptr textStyles, HilitingBuffer::Ptr hilitingBuffer)
    : rootElement(GuiLayoutColumn::create()),
      keyMapping(this),
      wasNeverShown(true)
{
    addToXEventMask(ButtonPressMask);
    keyMapping.add(            ControlMask, XK_f,      &EditorTopWin::invokeFindDialog);
    
    statusLine = StatusLine::create(this);
    rootElement->addElement(statusLine);
    
//    GuiLayoutTable::Ptr tableLayout = GuiLayoutTable::create(2, 2);
//    rootElement->addElement(tableLayout);
    
    textEditor = TextEditorWidget::create(this, textData, textStyles, hilitingBuffer);
    
//    GuiLayoutColumn::Ptr c1 = GuiLayoutColumn::create();
    GuiLayoutColumn::Ptr c2 = GuiLayoutColumn::create();
    GuiLayoutRow::Ptr    r1 = GuiLayoutRow::create();
    GuiLayoutRow::Ptr    r2 = GuiLayoutRow::create();
    rootElement->addElement(r1);
    
//    tableLayout->setElement(0, 0, c2);


    scrollBarV = ScrollBar::create(this, Orientation::VERTICAL);
//    tableLayout->setElement(0, 1, scrollBarV);
    
    c2->addElement(textEditor);
    c2->addElement(GuiLayoutWidget::create(this, 1, 1, 1, 1, -1, 1));
    r1->addElement(c2);
    r1->addElement(GuiLayoutWidget::create(this, 1, 1, 1, 1, 1, -1));
    r1->addElement(scrollBarV);
    
    scrollBarH = ScrollBar::create(this, Orientation::HORIZONTAL);
//    tableLayout->setElement(1, 0, scrollBarH);
    
//    c2->addElement(r2);
    rootElement->addElement(r2);
    r2->addElement(scrollBarH);
//    c1->addElement(GuiLayoutWidget::create(this, 1, 1, 1, 1, -1, 1));
//    c1->addElement(scrollBarH);
    int w = GlobalConfig::getInstance()->getScrollBarWidth();
    r2->addElement(GuiLayoutSpacer::create(w, w, w, w, w, w));
    
//    rootElement->setPosition(Position(0, 0, width, height));
    
    textData->registerFileNameListener(statusLine->slotForSetFileName);
    textData->registerLengthListener(statusLine->slotForSetFileLength);
    textEditor->registerLineAndColumnListener(statusLine->slotForSetLineAndColumn);
    
    scrollBarV->setChangedValueCallback(textEditor->slotForVerticalScrollBarChangedValue);
    scrollBarH->setChangedValueCallback(textEditor->slotForHorizontalScrollBarChangedValue);

    scrollBarV->setScrollStepCallback(textEditor->slotForScrollStepV);
    scrollBarH->setScrollStepCallback(textEditor->slotForScrollStepH);

    textEditor->setScrollBarVerticalValueRangeChangedCallback(  scrollBarV->slotForSetValueRange);
    textEditor->setScrollBarHorizontalValueRangeChangedCallback(scrollBarH->slotForSetValueRange);
    
    textEditor->setDesiredMeasuresInChars(
            GlobalConfig::getInstance()->getInitialWindowWidth(),
            GlobalConfig::getInstance()->getInitialWindowHeight()
    );

    textEditor->show();
    scrollBarV->show();
    scrollBarH->show();
    statusLine->show();

    Measures m = rootElement->getDesiredMeasures();
//    setPosition(Position(getPosition().x, getPosition().y, 
//                         m.bestWidth, m.bestHeight));

    setSizeHints(m.minWidth, m.minHeight, m.incrWidth, m.incrHeight);
//    setSizeHints(getPosition().x, getPosition().y, 
//                         m.minWidth, m.minHeight, 1, 1);

}

void EditorTopWin::show()
{
    if (rootElement.isValid())
    {
        if (wasNeverShown) {
//            setPosition(Position(getPosition().x, getPosition().y, 
//                                 m.bestWidth, m.bestHeight));
            Measures m = rootElement->getDesiredMeasures();
            setSizeHints(m.minWidth, m.minHeight, m.incrWidth, m.incrHeight);
            setSize(m.bestWidth, m.bestHeight);
            wasNeverShown = false;
        }
//        setSizeHints(getPosition().x, getPosition().y, 
//                             m.minWidth, m.minHeight, 1, 1);
    }
    GuiWidget::show();
}

void EditorTopWin::treatNewWindowPosition(Position newPosition)
{
    TopWin::treatNewWindowPosition(newPosition);
    rootElement->setPosition(Position(0, 0, newPosition.w, newPosition.h));
}

bool EditorTopWin::processKeyboardEvent(const XEvent *event)
{
    KeyMapping<EditorTopWin>::MethodPtr m = 
            keyMapping.find(event->xkey.state, XLookupKeysym((XKeyEvent*)&event->xkey, 0));

    if (m != NULL)
    {
        (this->*m)();
        return true;
    } 
    else
    {
        return textEditor->processKeyboardEvent(event);
    }
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

void EditorTopWin::requestCloseChildWindow(TopWin *topWin)
{
    if (findDialog.getRawPtr() == topWin) {
        findDialog->hide();
    } else {
        TopWinOwner::requestCloseChildWindow(topWin);
    }
}

void EditorTopWin::invokeFindDialog()
{
    if (!findDialog.isValid()) {
        findDialog = FindDialog::create(this, 250, 250, 100, 100);
    }
    findDialog->requestFocus();
}


