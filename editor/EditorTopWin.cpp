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
#include "ConfigException.hpp"
#include "WindowCloser.hpp"
#include "System.hpp"
#include "ConfigErrorHandler.hpp"

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
      flagForSetSizeHintAtFirstShow(true),
      hasModalMessageBox(false),
      isClosingFlag(false)
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
    
    textData->registerModifiedFlagListener(newCallback(this, &EditorTopWin::handleChangedModifiedFlag));
    
    textData->registerFileNameListener       (newCallback(statusLine, &StatusLine  ::setFileName));
    textData->registerReadOnlyListener       (newCallback(this,       &EditorTopWin::handleChangedReadOnlyFlag));
    textData->registerFileNameListener       (newCallback(this,       &EditorTopWin::handleNewFileName));
    textData->registerLengthListener         (newCallback(statusLine, &StatusLine  ::setFileLength));
    textEditor->registerLineAndColumnListener(newCallback(statusLine, &StatusLine  ::setLineAndColumn));
    
    scrollBarV->setChangedValueCallback      (newCallback(textEditor, &TextWidget::setTopLineNumber));
    scrollBarH->setChangedValueCallback      (newCallback(textEditor, &TextWidget::setLeftPix));

    scrollBarV->setScrollStepCallback        (newCallback(textEditor, &TextEditorWidget::handleScrollStepV));
    scrollBarH->setScrollStepCallback        (newCallback(textEditor, &TextEditorWidget::handleScrollStepH));

    textEditor->setScrollBarVerticalValueRangeChangedCallback  (newCallback(scrollBarV, &ScrollBar::setValueRange));
    textEditor->setScrollBarHorizontalValueRangeChangedCallback(newCallback(scrollBarH, &ScrollBar::setValueRange));
    
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
        setSizeHints(m.minWidth, m.minHeight, m.incrWidth, m.incrHeight);
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
                                  newCallback(this, &EditorTopWin::invokeMessageBox),
                                  newCallback(this, &EditorTopWin::invokePanel));

    replacePanel = ReplacePanel::create(this, textEditor, findPanel,
                                  newCallback(this, &EditorTopWin::invokeMessageBox),
                                  newCallback(this, &EditorTopWin::invokePanel));

    keyMapping1.set( KeyModifier("Ctrl"),       KeyId("q"),      newCallback(this,      &EditorTopWin::requestProgramQuit));
    keyMapping1.set( KeyModifier("Ctrl"),       KeyId("l"),      newCallback(this,      &EditorTopWin::invokeGotoLinePanel));
    keyMapping1.set( KeyModifier("Ctrl"),       KeyId("f"),      newCallback(this,      &EditorTopWin::invokeFindPanelForward));
    keyMapping1.set( KeyModifier("Ctrl+Shift"), KeyId("f"),      newCallback(this,      &EditorTopWin::invokeFindPanelBackward));
    keyMapping1.set( KeyModifier("Ctrl"),       KeyId("r"),      newCallback(this,      &EditorTopWin::invokeReplacePanelForward));
    keyMapping1.set( KeyModifier("Ctrl+Shift"), KeyId("r"),      newCallback(this,      &EditorTopWin::invokeReplacePanelBackward));
    keyMapping1.set( KeyModifier("Ctrl"),       KeyId("w"),      newCallback(this,      &EditorTopWin::requestCloseWindow));
    keyMapping1.set( KeyModifier(),             KeyId("Escape"), newCallback(this,      &EditorTopWin::handleEscapeKey));
    keyMapping1.set( KeyModifier("Ctrl"),       KeyId("s"),      newCallback(this,      &EditorTopWin::handleSaveKey));
    keyMapping1.set( KeyModifier("Ctrl+Shift"), KeyId("s"),      newCallback(this,      &EditorTopWin::handleSaveAsKey));
    keyMapping1.set( KeyModifier("Ctrl"),       KeyId("n"),      newCallback(this,      &EditorTopWin::createEmptyWindow));
    keyMapping1.set( KeyModifier("Ctrl"),       KeyId("h"),      newCallback(findPanel, &FindPanel::findSelectionForward));
    keyMapping1.set( KeyModifier("Ctrl+Shift"), KeyId("h"),      newCallback(findPanel, &FindPanel::findSelectionBackward));

    keyMapping1.set( KeyModifier("Ctrl"),       KeyId("t"),      newCallback(replacePanel, &ReplacePanel::replaceAgainForward));
    keyMapping1.set( KeyModifier("Ctrl+Shift"), KeyId("t"),      newCallback(replacePanel, &ReplacePanel::replaceAgainBackward));

    keyMapping2.set( KeyModifier("Ctrl"),       KeyId("g"),      newCallback(findPanel, &FindPanel::findAgainForward));
    keyMapping2.set( KeyModifier("Ctrl+Shift"), KeyId("g"),      newCallback(findPanel, &FindPanel::findAgainBackward));

    keyMapping2.set( KeyModifier("Alt"),        KeyId("c"),      newCallback(this,      &EditorTopWin::createCloneWindow));
    keyMapping2.set( KeyModifier("Alt"),        KeyId("l"),      newCallback(this,      &EditorTopWin::executeLuaScript));

    
    GlobalConfig::getInstance()->registerConfigChangedCallback(newCallback(this, &EditorTopWin::treatConfigUpdate));
}

EditorTopWin::~EditorTopWin()
{
    ViewCounterTextDataAccess::decViewCounter(textEditor->getTextData());
    closeModalMessageBox();
}

void EditorTopWin::treatConfigUpdate()
{
    TextData* textData = textEditor->getTextData();

    LanguageMode::Ptr newLanguageMode;

    if (textData->isFileNamePseudo())
    {
        newLanguageMode = GlobalConfig::getInstance()->getDefaultLanguageMode();
    }
    else {
        newLanguageMode = GlobalConfig::getInstance()->getLanguageModeForFileName(textData->getFileName());
    }
    textEditor->getHilitedText()->setLanguageMode(newLanguageMode);
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
    KeyId       pressedKey  = KeyId(XLookupKeysym((XKeyEvent*)&event->xkey, 0));
    KeyModifier keyModifier = KeyModifier(event->xkey.state);

    ProcessingResult rslt = NOT_PROCESSED;

    Callback<>::Ptr m = keyMapping1.find(keyModifier, pressedKey);

    if (m.isValid())
    {
        if (event->type == KeyPress && !pressedKey.isModifierKey()) {
            textEditor->hideMousePointer();
        }

        m->call();
        rslt = EVENT_PROCESSED;
    } 

    if (rslt == NOT_PROCESSED && invokedPanel.isValid()) {
        rslt = invokedPanel->processKeyboardEvent(event);
    }
    
    if (rslt == NOT_PROCESSED)
    {
       m = keyMapping2.find(keyModifier, pressedKey);

       if (m.isValid())
       {
           if (event->type == KeyPress) {
               textEditor->hideMousePointer();
           }

           m->call();
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
    if (hasModalMessageBox) {
        if (modalMessageBox.isInvalid()) {
            modalMessageBox = MessageBox::create(this, modalMessageBoxParameter);
            modalMessageBox->show();
        }
        textEditor->disableCursorChanges();
        modalMessageBox->raise();
    } else {
        if (invokedPanel.isValid()) {
            invokedPanel->treatFocusIn();
        } else {
            textEditor->treatFocusIn();
        }
    }
    checkForFileModifications();
}

bool EditorTopWin::checkForFileModifications()
{
    TextData* textData = textEditor->getTextData();
    textData->checkFileInfo();

    if (   textData->wasFileModifiedOnDisk() 
        && (  !textData->hasModifiedOnDiskFlagBeenIgnored()
            || textData->wasFileModifiedOnDiskSinceLastIgnore()))
    {
        invokeMessageBox(MessageBoxParameter().setTitle("File Modified")
                                              .setMessage(String() << "File '" 
                                                                   << textData->getFileName()
                                                                   << "' was modified on disk.")
                                              .setDefaultButton(    "R]eload", newCallback(this, &EditorTopWin::reloadFile))
                                              .setCancelButton (    "C]ancel", newCallback(this, &EditorTopWin::doNotReloadFile))
                                              );
        return true;
    }
    else {
        return false;
    }
}


void EditorTopWin::reloadFile()
{
    TextData* textData = textEditor->getTextData();
    textData->reloadFile();
}

void EditorTopWin::doNotReloadFile()
{
    textEditor->getTextData()->setIgnoreModifiedOnDiskFlag(true);
}

void EditorTopWin::treatFocusOut()
{
    if (invokedPanel.isValid()) {
        invokedPanel->treatFocusOut();
    } else {
        textEditor->treatFocusOut();
    }
}

void EditorTopWin::setModalMessageBox(const MessageBoxParameter& messageBoxParameter)
{
    this->hasModalMessageBox = true;
    this->modalMessageBoxParameter = messageBoxParameter;
    if (modalMessageBox.isValid()) {
        modalMessageBox->hide();
        modalMessageBox->requestCloseWindow();
        modalMessageBox = MessageBox::create(this, modalMessageBoxParameter);
        modalMessageBox->show();
    }
}

void EditorTopWin::closeModalMessageBox()
{
    if (hasModalMessageBox) {
        hasModalMessageBox = false;
        textEditor->enableCursorChanges();
        if (modalMessageBox.isValid()) {
            modalMessageBox->hide();
            modalMessageBox->requestCloseWindow();
        }
    }
}

void EditorTopWin::requestCloseChildWindow(TopWin *topWin)
{
    if (hasModalMessageBox && topWin == modalMessageBox)
    {
        hasModalMessageBox = false;
        textEditor->enableCursorChanges();
    }
    TopWinOwner::requestCloseChildWindow(topWin);
}

void EditorTopWin::invokeGotoLinePanel()
{
    if (gotoLinePanel.isInvalid()) {
        gotoLinePanel = GotoLinePanel::create(this, textEditor);
    }
    invokePanel(gotoLinePanel);
}

void EditorTopWin::invokeSaveAsPanel(Callback<>::Ptr saveCallback)
{
    if (saveAsPanel.isInvalid()) {
        saveAsPanel = SaveAsPanel::create(this, textEditor, 
                                                newCallback(this, &EditorTopWin::invokeMessageBox));
    }
    saveAsPanel->setSaveCallback(saveCallback);
    invokePanel(saveAsPanel);
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

void EditorTopWin::setWindowTitle()
{
    File file(textEditor->getTextData()->getFileName());
    
    String title = file.getBaseName();
    if (textEditor->getTextData()->getModifiedFlag() == true
     && textEditor->getTextData()->isReadOnly())
    {
        title << " (read only, modified)";
    } else if (textEditor->getTextData()->isReadOnly()) {
        title << " (read only)";
    } else if (textEditor->getTextData()->getModifiedFlag() == true) {
        title << " (modified)";
    }
    title << " - ";
    title << file.getDirName() << "/ [" << System::getInstance()->getHostName() << "]";
    setTitle(title);
}

void EditorTopWin::handleNewFileName(const String& fileName)
{
    setWindowTitle();
}

void EditorTopWin::handleChangedReadOnlyFlag(bool readOnlyFlag)
{
    setWindowTitle();
}

void EditorTopWin::handleChangedModifiedFlag(bool modifiedFlag)
{
    setWindowTitle();
}

void EditorTopWin::handleSaveKey()
{
    try {
        if (textEditor->getTextData()->isFileNamePseudo()) {
            invokeSaveAsPanel(newCallback(this, &EditorTopWin::handleSaveKey));
        }
        else {
            textEditor->getTextData()->save();
            GlobalConfig::getInstance()->notifyAboutNewFileContent(textEditor->getTextData()->getFileName());
        }
    } catch (FileException& ex) {
        invokeMessageBox(MessageBoxParameter().setTitle("File Error")
                                              .setMessage(ex.getMessage()));
    } catch (LuaException& ex) {
        invokeMessageBox(MessageBoxParameter().setTitle("Lua Error")
                                              .setMessage(ex.getMessage()));
    } catch (ConfigException& ex)
    {
        if (ex.getErrorList().isValid() && ex.getErrorList()->getLength() > 0) {
            ConfigErrorHandler::start(ex.getErrorList());
        } else {
            invokeMessageBox(MessageBoxParameter().setTitle("Config Error")
                                                  .setMessage(ex.getMessage()));
        }
    }
}

void EditorTopWin::handleSaveAsKey()
{
    invokeSaveAsPanel(newCallback(this, &EditorTopWin::handleSaveKey));
}

void EditorTopWin::saveAndClose()
{
    try {
        if (textEditor->getTextData()->isFileNamePseudo()) {
            invokeSaveAsPanel(newCallback(this, &EditorTopWin::saveAndClose));
        }
        else {
            textEditor->getTextData()->save();
            GlobalConfig::getInstance()->notifyAboutNewFileContent(textEditor->getTextData()->getFileName());
            requestCloseWindow();
        }
    } catch (FileException& ex) {
        invokeMessageBox(MessageBoxParameter().setTitle("File Error")
                                              .setMessage(ex.getMessage()));
    } catch (LuaException& ex) {
        invokeMessageBox(MessageBoxParameter().setTitle("Lua Error")
                                              .setMessage(ex.getMessage()));
    } catch (ConfigException& ex) {
        invokeMessageBox(MessageBoxParameter().setTitle("Config Error")
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
                                              .setDefaultButton(    "S]ave",    newCallback(this, &EditorTopWin::saveAndClose))
                                              .setAlternativeButton("D]iscard", newCallback(this, &EditorTopWin::requestCloseWindowAndDiscardChanges)));
        
    }
    else
    {
        TopWin::requestCloseWindow();
        isClosingFlag = true;
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
                      emptyTextData->setPseudoFileName(untitledFileName);

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
        if (textEditor->hasSelection())
        {
            long selBegin  = textEditor->getBeginSelectionPos();
            long selLength = textEditor->getEndSelectionPos() - selBegin;
            
            LuaInterpreter::Result scriptResult = LuaInterpreter::getInstance()->executeScript((const char*) textEditor->getTextData()->getAmount(selBegin, selLength),
                                                                                               selLength);
            String output = scriptResult.output;
            
            EditingHistory::SectionHolder::Ptr historySectionHolder = textEditor->getTextData()->createHistorySection();
            
            textEditor->hideCursor();
            textEditor->moveCursorToTextPosition(selBegin + selLength);
            if (output.getLength() > 0) {
                textEditor->insertAtCursor((const byte*) output.toCString(), output.getLength());
                textEditor->setPrimarySelection(selBegin + selLength, 
                                                selBegin + selLength + output.getLength());

                textEditor->moveCursorToTextPosition(selBegin + selLength + output.getLength());
                textEditor->assureCursorVisible();
                textEditor->moveCursorToTextPosition(selBegin + selLength);
            } else {
                textEditor->releaseSelection();
            }
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
                    EditingHistory::SectionHolder::Ptr historySectionHolder = textEditor->getTextData()->createHistorySection();
                    
                    textEditor->hideCursor();
                    textEditor->moveCursorToTextPosition(spos);
                    textEditor->removeAtCursor(cursorPos - spos);
                    textEditor->insertAtCursor((const byte*) output.toCString(), output.getLength());
                    textEditor->moveCursorToTextPosition(spos + output.getLength());
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

String EditorTopWin::getFileName() const
{
    return textEditor->getTextData()->getFileName();
}

void EditorTopWin::requestProgramQuit()
{
    WindowCloser::start();
}

