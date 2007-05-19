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

#include <X11/keysym.h>
#include <X11/Xatom.h>

#include "util.hpp"
#include "EditorTopWin.hpp"
#include "GlobalConfig.hpp"
#include "GuiLayoutWidget.hpp"
#include "GuiLayoutColumn.hpp"
#include "GuiLayoutRow.hpp"
#include "GuiLayoutSpacer.hpp"
#include "Callback.hpp"
#include "WeakPtr.hpp"
#include "File.hpp"
#include "FileException.hpp"
#include "LuaInterpreter.hpp"
#include "LuaException.hpp"

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



EditorTopWin::EditorTopWin(TextStyles::Ptr textStyles, HilitedText::Ptr hilitedText, int width, int height)
    : rootElement(GuiLayoutColumn::create()),
      flagForSetSizeHintAtFirstShow(true)
{
    addToXEventMask(ButtonPressMask);
    
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
    ViewCounterTextDataAccess::incViewCounter(textData);
    
    textData->registerModifiedFlagListener(Callback1<bool>(this, &EditorTopWin::handleChangedModifiedFlag));
    
    textData->registerFileNameListener       (Callback1<const String&>   (statusLine, &StatusLine  ::setFileName));
    textData->registerFileNameListener       (Callback1<const String&>   (this,       &EditorTopWin::handleNewFileName));
    textData->registerLengthListener         (Callback1<long>            (statusLine, &StatusLine  ::setFileLength));
    textEditor->registerLineAndColumnListener(Callback2<long,long>       (statusLine, &StatusLine  ::setLineAndColumn));
    
    scrollBarV->setChangedValueCallback      (Callback1<long>            (textEditor, &TextWidget::setTopLineNumber));
    scrollBarH->setChangedValueCallback      (Callback1<long>            (textEditor, &TextWidget::setLeftPix));

    scrollBarV->setScrollStepCallback        (Callback1<ScrollStep::Type>(textEditor, &TextEditorWidget::handleScrollStepV));
    scrollBarH->setScrollStepCallback        (Callback1<ScrollStep::Type>(textEditor, &TextEditorWidget::handleScrollStepH));

    textEditor->setScrollBarVerticalValueRangeChangedCallback  (Callback3<long,long,long>(scrollBarV, &ScrollBar::setValueRange));
    textEditor->setScrollBarHorizontalValueRangeChangedCallback(Callback3<long,long,long>(scrollBarH, &ScrollBar::setValueRange));
    
    textEditor->setDesiredMeasuresInChars(
            GlobalConfig::getInstance()->getInitialWindowWidth(),
            GlobalConfig::getInstance()->getInitialWindowHeight()
    );


    Measures m = rootElement->getDesiredMeasures();
//    setPosition(Position(getPosition().x, getPosition().y, 
//                         m.bestWidth, m.bestHeight));
    if (width == -1 || height == -1)
    {
        setSizeHints(m.minWidth, m.minHeight, m.incrWidth, m.incrHeight);
        setSize(m.bestWidth, m.bestHeight);
        rootElement->setPosition(Position(0, 0, m.bestWidth, m.bestHeight));
    }
    else
    {
        setSizeHints(width, height, m.incrWidth, m.incrHeight);
        setSize(width, height);
        rootElement->setPosition(Position(0, 0, width, height));
    }
//    setSizeHints(getPosition().x, getPosition().y, 
//                         m.minWidth, m.minHeight, 1, 1);

    flagForSetSizeHintAtFirstShow = true;
    
    textEditor->show();
    scrollBarV->show();
    scrollBarH->show();
    statusLine->show();

    findPanel = FindPanel::create(this, textEditor, 
                                  Callback1<MessageBoxParameter>(this, &EditorTopWin::invokeMessageBox),
                                  Callback1<DialogPanel*>       (this, &EditorTopWin::invokePanel));

    replacePanel = ReplacePanel::create(this, textEditor, findPanel,
                                  Callback1<MessageBoxParameter>(this, &EditorTopWin::invokeMessageBox),
                                  Callback1<DialogPanel*>       (this, &EditorTopWin::invokePanel));

    keyMapping1.set(            ControlMask, XK_l,      Callback0(this,      &EditorTopWin::invokeGotoLinePanel));
    keyMapping1.set(            ControlMask, XK_f,      Callback0(this,      &EditorTopWin::invokeFindPanelForward));
    keyMapping1.set(  ControlMask|ShiftMask, XK_f,      Callback0(this,      &EditorTopWin::invokeFindPanelBackward));
    keyMapping1.set(            ControlMask, XK_r,      Callback0(this,      &EditorTopWin::invokeReplacePanelForward));
    keyMapping1.set(  ControlMask|ShiftMask, XK_r,      Callback0(this,      &EditorTopWin::invokeReplacePanelBackward));
    keyMapping1.set(            ControlMask, XK_w,      Callback0(this,      &EditorTopWin::requestCloseWindow));
    keyMapping1.set(                      0, XK_Escape, Callback0(this,      &EditorTopWin::handleEscapeKey));
    keyMapping1.set(            ControlMask, XK_s,      Callback0(this,      &EditorTopWin::handleSaveKey));
    keyMapping1.set(            ControlMask, XK_n,      Callback0(this,      &EditorTopWin::createEmptyWindow));
    keyMapping1.set(            ControlMask, XK_h,      Callback0(findPanel, &FindPanel::findSelectionForward));
    keyMapping1.set(  ShiftMask|ControlMask, XK_h,      Callback0(findPanel, &FindPanel::findSelectionBackward));

    keyMapping1.set(            ControlMask, XK_t,      Callback0(replacePanel, &ReplacePanel::replaceAgainForward));
    keyMapping1.set(  ControlMask|ShiftMask, XK_t,      Callback0(replacePanel, &ReplacePanel::replaceAgainBackward));

    keyMapping2.set(            ControlMask, XK_g,      Callback0(findPanel, &FindPanel::findAgainForward));
    keyMapping2.set(  ControlMask|ShiftMask, XK_g,      Callback0(findPanel, &FindPanel::findAgainBackward));

    keyMapping2.set(               Mod1Mask, XK_c,      Callback0(this,      &EditorTopWin::createCloneWindow));
    keyMapping2.set(               Mod1Mask, XK_l,      Callback0(this,      &EditorTopWin::executeLuaScript));
}

EditorTopWin::~EditorTopWin()
{
    ViewCounterTextDataAccess::decViewCounter(textEditor->getTextData());
}

void EditorTopWin::show()
{
    if (rootElement.isValid())
    {
// TODO: What was the following code good for?!? ( -- it causes problems for initial window sizes)
//    
//        if (flagForSetSizeHintAtFirstShow) {
////            setPosition(Position(getPosition().x, getPosition().y, 
////                                 m.bestWidth, m.bestHeight));
//            Measures m = rootElement->getDesiredMeasures();
//            setSizeHints(m.minWidth, m.minHeight, m.incrWidth, m.incrHeight);
//            setSize(m.bestWidth, m.bestHeight);
//            flagForSetSizeHintAtFirstShow = false;
//        }
////        setSizeHints(getPosition().x, getPosition().y, 
////                             m.minWidth, m.minHeight, 1, 1);
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
    ProcessingResult rslt = NOT_PROCESSED;

    Callback0 m = keyMapping1.find(event->xkey.state, XLookupKeysym((XKeyEvent*)&event->xkey, 0));

    if (m.isValid())
    {
        if (event->type == KeyPress) {
            textEditor->hideMousePointer();
        }

        m.call();
        rslt = EVENT_PROCESSED;
    } 

    if (rslt == NOT_PROCESSED && invokedPanel.isValid()) {
        rslt = invokedPanel->processKeyboardEvent(event);
    }
    
    if (rslt == NOT_PROCESSED)
    {
       m = keyMapping2.find(event->xkey.state, XLookupKeysym((XKeyEvent*)&event->xkey, 0));

       if (m.isValid())
       {
           if (event->type == KeyPress) {
               textEditor->hideMousePointer();
           }

           m.call();
           rslt = EVENT_PROCESSED;
       } 
       else
       {
            rslt = textEditor->processKeyboardEvent(event);
            if (rslt == EVENT_PROCESSED && invokedPanel.isValid()) {
                invokedPanel->notifyAboutHotKeyEventForOtherWidget();
            }
       }
    }
    return rslt;
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


void EditorTopWin::invokeFindPanelForward()
{
    ASSERT(findPanel.isValid());
    findPanel->setDefaultDirection(Direction::DOWN);
    invokePanel(findPanel);
}

void EditorTopWin::invokeFindPanelBackward()
{
    ASSERT(findPanel.isValid());
    findPanel->setDefaultDirection(Direction::UP);
    invokePanel(findPanel);
}

void EditorTopWin::invokeReplacePanelForward()
{
    ASSERT(replacePanel.isValid());
    replacePanel->setDefaultDirection(Direction::DOWN);
    invokePanel(replacePanel);
}

void EditorTopWin::invokeReplacePanelBackward()
{
    ASSERT(replacePanel.isValid());
    replacePanel->setDefaultDirection(Direction::UP);
    invokePanel(replacePanel);
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
        if (messageBox.isValid()) {
            requestCloseChildWindow(messageBox);
        }
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

void EditorTopWin::handleNewFileName(const String& fileName)
{
    File file(fileName);
    
    if (textEditor->getTextData()->getModifiedFlag() == false) {
        setTitle(String() << file.getBaseName() << " - " << file.getDirName());
    } else {
        setTitle(String() << file.getBaseName() << " (modified) - " << file.getDirName());
    }
}

void EditorTopWin::handleChangedModifiedFlag(bool modifiedFlag)
{
    File file(textEditor->getTextData()->getFileName());
    
    if (modifiedFlag == false) {
        setTitle(String() << file.getBaseName() << " - " << file.getDirName());
    } else {
        setTitle(String() << file.getBaseName() << " (modified) - " << file.getDirName());
    }
}

void EditorTopWin::handleSaveKey()
{
    try {
        textEditor->getTextData()->save();
    } catch (FileException& ex) {
        invokeMessageBox(MessageBoxParameter().setTitle("Error")
                                              .setMessage(ex.getMessage()));
    }
}

void EditorTopWin::saveAndClose()
{
    try {
        textEditor->getTextData()->save();
        requestCloseWindow();
    } catch (FileException& ex) {
        invokeMessageBox(MessageBoxParameter().setTitle("Error")
                                              .setMessage(ex.getMessage()));
    }
}

void EditorTopWin::requestCloseWindow()
{
    File file(textEditor->getTextData()->getFileName());

    if (ViewCounterTextDataAccess::getViewCounter(textEditor->getTextData()) == 1
     && textEditor->getTextData()->getModifiedFlag() == true)
    {
        invokeMessageBox(MessageBoxParameter().setTitle("Save File")
                                              .setMessage(String() << "Save file '" << file.getBaseName() 
                                                                   << "' before closing?")
                                              .setDefaultButton(    "S]ave",    Callback0(this, &EditorTopWin::saveAndClose))
                                              .setAlternativeButton("D]iscard", Callback0(this, &EditorTopWin::requestCloseWindowAndDiscardChanges)));
        
    }
    else
    {
        TopWin::requestCloseWindow();
    }
}

void EditorTopWin::requestCloseWindowAndDiscardChanges()
{
    TopWin::requestCloseWindow();
}

void EditorTopWin::createEmptyWindow()
{
    TextStyles::Ptr   textStyles   = GlobalConfig::getInstance()->getTextStyles();
    LanguageMode::Ptr languageMode = GlobalConfig::getInstance()->getDefaultLanguageMode();

    String untitledFileName = File(String() << textEditor->getTextData()->getFileName()).getDirName() << "/Untitled";
    TextData::Ptr     emptyTextData     = TextData::create();
                      emptyTextData->setFileName(untitledFileName);

    HilitedText::Ptr  hilitedText  = HilitedText::create(emptyTextData, languageMode);

    EditorTopWin::Ptr win          = EditorTopWin::create(textStyles, hilitedText);
                      win->show();
}

void EditorTopWin::setSize(int width, int height)
{
    flagForSetSizeHintAtFirstShow = false;
    TopWin::setSize(width, height);
}

void EditorTopWin::createCloneWindow()
{
    Position myPosition = this->getPosition();

    EditorTopWin::Ptr newWin = EditorTopWin::create(textEditor->getTextStyles(), 
                                                    textEditor->getHilitedText(),
                                                    myPosition.w, 
                                                    myPosition.h);
    newWin->textEditor->moveCursorToTextMark(this->textEditor->createNewMarkFromCursor());
    newWin->textEditor->setTopLineNumber(    this->textEditor->getTopLineNumber());
    newWin->textEditor->setLeftPix(          this->textEditor->getLeftPix());
    newWin->show();
}

void EditorTopWin::executeLuaScript()
{
    if (textEditor->areCursorChangesDisabled())
    {
        return;
    }
    try
    {
        if (textEditor->hasSelectionOwnership())
        {
            long selBegin  = textEditor->getBackliteBuffer()->getBeginSelectionPos();
            long selLength = textEditor->getBackliteBuffer()->getEndSelectionPos() - selBegin;
            
            LuaInterpreter::Result scriptResult = LuaInterpreter::getInstance()->executeScript((const char*) textEditor->getTextData()->getAmount(selBegin, selLength),
                                                                                               selLength);
            String output = scriptResult.output;
            textEditor->getTextData()->setHistorySeparator();
            textEditor->hideCursor();
            textEditor->moveCursorToTextPosition(selBegin + selLength);
            if (output.getLength() > 0) {
                textEditor->insertAtCursor((const byte*) output.toCString(), output.getLength());
                textEditor->getBackliteBuffer()->activateSelection(selBegin + selLength);
                textEditor->getBackliteBuffer()->extendSelectionTo(selBegin + selLength + output.getLength());

                textEditor->moveCursorToTextPosition(selBegin + selLength + output.getLength());
                textEditor->assureCursorVisible();
                textEditor->moveCursorToTextPosition(selBegin + selLength);
            } else {
                textEditor->releaseSelectionOwnership();
            }
            textEditor->getTextData()->setHistorySeparator();
            textEditor->assureCursorVisible();
            textEditor->rememberCursorPixX();
            textEditor->showCursor();
        }
        else
        {
            long cursorPos = textEditor->getCursorTextPosition();
            long spos = cursorPos;
            int parenCounter = 0;
            
            while (spos > 0)
            {
                byte c = textEditor->getTextData()->getChar(spos - 1);
                if (parenCounter > 0)
                {
                    if (c == '(') {
                        --parenCounter;
                    } else if (c == ')') {
                        ++parenCounter;
                    }
                    --spos;
                }
                else if (textEditor->isWordCharacter(c) || c == '.') {
                    --spos;
                }
                else if (c == ')') {
                    ++parenCounter;
                    --spos;
                } else {
                    break;
                }
            }
            if (spos < cursorPos)
            {
                LuaInterpreter::Result scriptResult = LuaInterpreter::getInstance()->executeExpression((const char*) textEditor->getTextData()->getAmount(spos, cursorPos - spos),
                                                                                                       cursorPos - spos);
                String output = scriptResult.output;
                for (int i = 0, n = scriptResult.objects.getLength(); i < n; ++i) {
                    output << scriptResult.objects[i].toString();
                }
                if (output.getLength() > 0) 
                {
                    textEditor->hideCursor();
                    textEditor->moveCursorToTextPosition(spos);
                    textEditor->removeAtCursor(cursorPos - spos);
                    textEditor->insertAtCursor((const byte*) output.toCString(), output.getLength());
                    textEditor->moveCursorToTextPosition(spos + output.getLength());
                    textEditor->getTextData()->setHistorySeparator();
                    textEditor->assureCursorVisible();
                    textEditor->rememberCursorPixX();
                    textEditor->showCursor();
                }
            }
        }
    }
    catch (LuaException& ex)
    {
        invokeMessageBox(MessageBoxParameter().setTitle("Lua Error")
                                              .setMessage(ex.getMessage()));
    }
}

