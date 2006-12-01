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
#include "Callback.h"
#include "WeakPtr.h"

using namespace LucED;

class PanelLayoutAdapter : public GuiElement
{
public:
    typedef OwningPtr<PanelLayoutAdapter> Ptr;
    
    static Ptr create(MultiLineEditorWidget* editorWidget, GuiElement* panel) {
        return Ptr(new PanelLayoutAdapter(editorWidget, panel));
    }

    virtual Measures getDesiredMeasures() {
        Measures rslt = panel->getDesiredMeasures();
        int lineHeight = editorWidget->getLineHeight();
        rslt.minHeight  = ROUNDED_UP_DIV(rslt.minHeight,  lineHeight) * lineHeight;
        rslt.bestHeight = ROUNDED_UP_DIV(rslt.bestHeight, lineHeight) * lineHeight;
        if (rslt.maxHeight != INT_MAX) {
            rslt.maxHeight = ROUNDED_UP_DIV(rslt.maxHeight, lineHeight) * lineHeight;
        }
        return rslt;
    }
    
    virtual void setPosition(Position p) {
        panel->setPosition(p);
    }
    
private:
    PanelLayoutAdapter(MultiLineEditorWidget* editorWidget, GuiElement* panel)
        : editorWidget(editorWidget),
          panel(panel)
    {}
    

    WeakPtr<MultiLineEditorWidget> editorWidget;
    WeakPtr<GuiElement> panel;
};



EditorTopWin::EditorTopWin(TextStyles::Ptr textStyles, HilitedText::Ptr hilitedText)
    : rootElement(GuiLayoutColumn::create()),
      wasNeverShown(true)
{
    addToXEventMask(ButtonPressMask);
    keyMapping.set(            ControlMask, XK_l,      Callback0(this, &EditorTopWin::invokeGotoLinePanel));
    keyMapping.set(            ControlMask, XK_f,      Callback0(this, &EditorTopWin::invokeFindPanel));
    keyMapping.set(            ControlMask, XK_w,      Callback0(this, &EditorTopWin::requestCloseWindow));
    keyMapping.set(                      0, XK_Escape, Callback0(this, &EditorTopWin::handleEscapeKey));
    
    statusLine = StatusLine::create(this);
    int statusLineIndex = rootElement->addElement(statusLine);
    upperPanelIndex = statusLineIndex + 1;
    
//    GuiLayoutTable::Ptr tableLayout = GuiLayoutTable::create(2, 2);
//    rootElement->addElement(tableLayout);
    
    textEditor = MultiLineEditorWidget::create(this, textStyles, hilitedText);
    
//    GuiLayoutColumn::Ptr c1 = GuiLayoutColumn::create();
    GuiLayoutColumn::Ptr c2 = GuiLayoutColumn::create();
    GuiLayoutRow::Ptr    r1 = GuiLayoutRow::create();
    GuiLayoutRow::Ptr    r2 = GuiLayoutRow::create();
    rootElement->addElement(r1);
    
//    tableLayout->setElement(0, 0, c2);


    scrollBarV = ScrollBar::create(this, Orientation::VERTICAL);
//    tableLayout->setElement(0, 1, scrollBarV);
    
    c2->addElement(textEditor);
    c2->addElement(GuiLayoutWidget::create(this, 1, 1, 1, 1, INT_MAX, 1));
    r1->addElement(c2);
    r1->addElement(GuiLayoutWidget::create(this, 1, 1, 1, 1, 1, INT_MAX));
    r1->addElement(scrollBarV);
    
    scrollBarH = ScrollBar::create(this, Orientation::HORIZONTAL);
//    tableLayout->setElement(1, 0, scrollBarH);
    
//    c2->addElement(r2);
    int r2Index = rootElement->addElement(r2);
    lowerPanelIndex = r2Index + 1;
    
    r2->addElement(scrollBarH);
//    c1->addElement(GuiLayoutWidget::create(this, 1, 1, 1, 1, INT_MAX, 1));
//    c1->addElement(scrollBarH);
    int w = GlobalConfig::getInstance()->getScrollBarWidth();
    r2->addElement(GuiLayoutSpacer::create(w, w, w, w, w, w));
    
//    rootElement->setPosition(Position(0, 0, width, height));
    
    TextData::Ptr textData = hilitedText->getTextData();
    
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


    Measures m = rootElement->getDesiredMeasures();
//    setPosition(Position(getPosition().x, getPosition().y, 
//                         m.bestWidth, m.bestHeight));

    setSizeHints(m.minWidth, m.minHeight, m.incrWidth, m.incrHeight);
    setSize(m.bestWidth, m.bestHeight);
    rootElement->setPosition(Position(0, 0, m.bestWidth, m.bestHeight));
//    setSizeHints(getPosition().x, getPosition().y, 
//                         m.minWidth, m.minHeight, 1, 1);

    textEditor->show();
    scrollBarV->show();
    scrollBarH->show();
    statusLine->show();
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

GuiElement::ProcessingResult EditorTopWin::processKeyboardEvent(const XEvent *event)
{
    Callback0 m = keyMapping.find(event->xkey.state, XLookupKeysym((XKeyEvent*)&event->xkey, 0));

    if (m.isValid())
    {
        m.call();
        return EVENT_PROCESSED;
    } 
    else
    {
        ProcessingResult rslt = NOT_PROCESSED;
        
        if (invokedPanel.isValid()) {
            rslt = invokedPanel->processKeyboardEvent(event);
        }
        if (rslt == NOT_PROCESSED) {
            rslt = textEditor->processKeyboardEvent(event);
            if (rslt == EVENT_PROCESSED && invokedPanel.isValid()) {
                invokedPanel->notifyAboutHotKeyEventForOtherWidget();
            }
        }
        return rslt;
    }
}


GuiElement::ProcessingResult EditorTopWin::processEvent(const XEvent *event)
{
    if (TopWin::processEvent(event) == EVENT_PROCESSED) {
        return EVENT_PROCESSED;
    } else {
        switch (event->type)
        {
            case ButtonPress: {
                if (event->xbutton.button == Button4
                 || event->xbutton.button == Button5)
                {
                    textEditor->processEvent(event);
                    return EVENT_PROCESSED;
                }
            }
        }
        return NOT_PROCESSED;
    }
}


void EditorTopWin::treatFocusIn()
{
    if (invokedPanel.isValid()) {
        invokedPanel->treatFocusIn();
    } else {
        textEditor->treatFocusIn();
    }
}


void EditorTopWin::treatFocusOut()
{
    if (invokedPanel.isValid()) {
        invokedPanel->treatFocusOut();
    } else {
        textEditor->treatFocusOut();
    }
}

void EditorTopWin::requestCloseChildWindow(TopWin *topWin)
{
    TopWinOwner::requestCloseChildWindow(topWin);
}

void EditorTopWin::invokeGotoLinePanel()
{
    if (gotoLinePanel.isInvalid()) {
        gotoLinePanel = GotoLinePanel::create(this, textEditor);
    }
    invokePanel(gotoLinePanel);
}


void EditorTopWin::invokeFindPanel()
{
    if (findPanel.isInvalid()) {
        findPanel = FindPanel::create(this, textEditor, 
                                      Callback1<MessageBoxParameter>(this, &EditorTopWin::invokeMessageBox));
    }
    invokePanel(findPanel);
}

void EditorTopWin::invokePanel(DialogPanel* panel)
{
    if (invokedPanel != panel) {
        if (invokedPanel.isValid()) {
            requestCloseFor(invokedPanel);
        }
        ASSERT(invokedPanel.isInvalid());
        int panelIndex;
        if (GlobalConfig::getInstance()->isEditorPanelOnTop()) {
            textEditor->setResizeAdjustment(VerticalAdjustment::BOTTOM);
            panelIndex = upperPanelIndex;
        } else {
            textEditor->setResizeAdjustment(VerticalAdjustment::TOP);
            panelIndex = lowerPanelIndex;
        }
        rootElement->insertElementAtPosition(PanelLayoutAdapter::create(textEditor, panel), panelIndex);
        Position p = getPosition();
        rootElement->setPosition(Position(0, 0, p.w, p.h));
        panel->show();
        panel->treatFocusIn();
        textEditor->treatFocusOut();
        invokedPanel = panel;
        textEditor->setResizeAdjustment(VerticalAdjustment::TOP);
    }
}

void EditorTopWin::requestCloseFor(GuiWidget* w)
{
    if (w == invokedPanel) {
        int panelIndex;
        if (GlobalConfig::getInstance()->isEditorPanelOnTop()) {
            textEditor->setResizeAdjustment(VerticalAdjustment::BOTTOM);
            panelIndex = upperPanelIndex;
        } else {
            textEditor->setResizeAdjustment(VerticalAdjustment::TOP);
            panelIndex = lowerPanelIndex;
        }
        rootElement->removeElementAtPosition(panelIndex);
        w->hide();
        w->treatFocusOut();
        textEditor->treatFocusIn();
        invokedPanel.invalidate();
        Position p = getPosition();
        rootElement->setPosition(Position(0, 0, p.w, p.h));
        textEditor->setResizeAdjustment(VerticalAdjustment::TOP);
    }
}

void EditorTopWin::handleEscapeKey()
{
    if (invokedPanel.isValid()) {
        requestCloseFor(invokedPanel);
    }
}

void EditorTopWin::invokeMessageBox(MessageBoxParameter p)
{
    if (messageBox.isValid()) {
        requestCloseChildWindow(messageBox);
    }
    messageBox = MessageBox::create(this, p);
    messageBox->requestFocus();
}

