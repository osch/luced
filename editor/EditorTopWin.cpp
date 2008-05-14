/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2008 Oliver Schmidt, oliver at luced dot de
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
#include "ProgramExecutor.hpp"
#include "CommandOutputBox.hpp"
#include "EditorServer.hpp"
#include "PanelInvoker.hpp"

using namespace LucED;

namespace // anonymous namespace
{

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
    

    RawPtr<MultiLineEditorWidget> editorWidget;
    RawPtr<GuiElement> panel;
};

} // anonymous namespace


class EditorTopWin::PanelInvoker : public LucED::PanelInvoker
{
public:
    typedef OwningPtr<PanelInvoker> Ptr;
    
    static Ptr create(RawPtr<EditorTopWin> editorTopWin)
    {
        return Ptr(new PanelInvoker(editorTopWin));
    }

    virtual void invokePanel(DialogPanel* panel) {
        editorTopWin->invokePanel(panel);
    }
    virtual bool hasInvokedPanel() {
        return editorTopWin->invokedPanel.isValid();
    }
    virtual void closeInvokedPanel() {
        if (editorTopWin->invokedPanel.isValid()) {
            editorTopWin->requestCloseFor(editorTopWin->invokedPanel);
        }
    }
    virtual void closePanel(DialogPanel* panel) {
        if (panel != NULL) {
            editorTopWin->requestCloseFor(panel);
        }
    }

private:
    PanelInvoker(RawPtr<EditorTopWin> editorTopWin)
        : editorTopWin(editorTopWin)
    {}
    RawPtr<EditorTopWin> editorTopWin;
};

EditorTopWin::EditorTopWin(TextStyles::Ptr textStyles, HilitedText::Ptr hilitedText, int width, int height)
    : rootElement(GuiLayoutColumn::create()),
      keyMapping1(KeyMapping::create()),
      keyMapping2(KeyMapping::create()),
      flagForSetSizeHintAtFirstShow(true),
      hasMessageBox(false),
      isMessageBoxModal(false)
{
    addToXEventMask(ButtonPressMask);
    
    statusLine = StatusLine::create(this);
    int statusLineIndex = rootElement->addElement(statusLine);
    upperPanelIndex = statusLineIndex + 1;
    
    textEditor = MultiLineEditorWidget::create(this, textStyles, hilitedText);

    textEditor->setVerticalAdjustmentStrategy(TextWidget::STRICT_TOP_LINE_ANCHOR);


    scrollableTextCompound = ScrollableTextGuiCompound::create(this,
                                                               textEditor,
                                                                 ScrollableTextGuiCompound::Options()
                                                               | ScrollableTextGuiCompound::WITHOUT_OUTER_FRAME);
    
    int r2Index = rootElement->addElement(scrollableTextCompound);
    lowerPanelIndex = r2Index + 1;
    
    textData   = textEditor->getTextData();
    ViewCounterTextDataAccess::incViewCounter(textData);
    
    textData->registerModifiedFlagListener(newCallback(this, &EditorTopWin::handleChangedModifiedFlag));
    
    textData->registerFileNameListener          (newCallback(statusLine, &StatusLine  ::setFileName));
    textData->registerReadOnlyListener          (newCallback(this,       &EditorTopWin::handleChangedReadOnlyFlag));
    textData->registerFileNameListener          (newCallback(this,       &EditorTopWin::handleNewFileName));
    textData->registerLengthListener            (newCallback(statusLine, &StatusLine  ::setFileLength));

    textEditor->registerCursorPositionDataListener(newCallback(statusLine, &StatusLine::setCursorPositionData));
    
    textEditor->setDesiredMeasuresInChars(
            GlobalConfig::getInstance()->getInitialWindowWidth(),
            GlobalConfig::getInstance()->getInitialWindowHeight()
    );


    statusLine->show();
    scrollableTextCompound->show();

    Measures m = rootElement->getDesiredMeasures();

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

    flagForSetSizeHintAtFirstShow = true;
    

//    keyMapping1->set( KeyModifier("Ctrl"),       KeyId("r"),      newCallback(this,      &EditorTopWin::invokeReplacePanelForward));
    keyMapping1->set( KeyModifier("Ctrl"),       KeyId("w"),      newCallback(this,      &EditorTopWin::requestCloseWindowByUser));
    keyMapping1->set( KeyModifier("Ctrl"),       KeyId("s"),      newCallback(this,      &EditorTopWin::handleSaveKey));
    keyMapping1->set( KeyModifier("Ctrl+Shift"), KeyId("s"),      newCallback(this,      &EditorTopWin::handleSaveAsKey));
    keyMapping1->set( KeyModifier("Ctrl"),       KeyId("n"),      newCallback(this,      &EditorTopWin::createEmptyWindow));


    keyMapping2->set( KeyModifier("Alt"),        KeyId("c"),      newCallback(this,      &EditorTopWin::createCloneWindow));
    keyMapping2->set( KeyModifier("Alt"),        KeyId("l"),      newCallback(this,      &EditorTopWin::executeLuaScript));

///////////// TODO
    keyMapping1->set( KeyModifier("Modifier5"),  KeyId("e"),      newCallback(this,      &EditorTopWin::executeTestScript));
/////////////
    
    GlobalConfig::getInstance()->registerConfigChangedCallback(newCallback(this, &EditorTopWin::treatConfigUpdate));


    Callback<GuiWidget*>::Ptr requestClosePanelCallback = newCallback(this, &EditorTopWin::requestCloseFor);

    panelInvoker = PanelInvoker::create(this);

    actionBinder = TopWinActionBinder::create(
                        TopWinActions::Parameter(this, 
                                                 textEditor,
                                                 newCallback(this, &EditorTopWin::setMessageBox),
                                                 panelInvoker));

    rootKeyBinding = TopWinKeyBinding::create(GlobalConfig::getInstance()->getTopWinKeyBindingConfig(),
                                              actionBinder);
    currentKeyBinding = rootKeyBinding;

}

EditorTopWin::~EditorTopWin()
{
    ViewCounterTextDataAccess::decViewCounter(textData);
    closeMessageBox();
}

void EditorTopWin::treatConfigUpdate()
{
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

GuiElement::ProcessingResult EditorTopWin::processKeyboardEvent(const XEvent* event)
{
    try
    {
        KeyId       pressedKey  = KeyId(XLookupKeysym((XKeyEvent*)&event->xkey, 0));
        KeyModifier keyModifier = KeyModifier(event->xkey.state);
    
        ProcessingResult rslt = NOT_PROCESSED;
    
        Callback<>::Ptr m = keyMapping1->find(keyModifier, pressedKey);
    
        if (m.isValid())
        {
            if (event->type == KeyPress && !pressedKey.isModifierKey()) {
                textEditor->hideMousePointer();
            }
    
            if (currentKeyBinding != rootKeyBinding) {
                currentKeyBinding = rootKeyBinding;
                if (invokedPanel.isValid()) {
                    invokedPanel->treatFocusIn();
                } else {
                    textEditor->treatFocusIn();
                }
                statusLine->clearMessage();
            }
            m->call();
            rslt = EVENT_PROCESSED;
        }
        else if (event->type == KeyPress && !pressedKey.isModifierKey())
        {
            textEditor->hideMousePointer();

            TopWinKeyBinding::FoundValue foundBinding;
            
            if (currentKeyBinding != rootKeyBinding) {
                foundBinding = currentKeyBinding->find(combinationKeyModifier, pressedKey);
            } else {
                foundBinding = currentKeyBinding->find(keyModifier,            pressedKey);
            }
            if (foundBinding.isValid())
            {
                if (foundBinding.get().isCallable())
                {
                    foundBinding.get().call();
                    currentKeyBinding = rootKeyBinding;
                    rslt = EVENT_PROCESSED;
                }
                else if (foundBinding.get().hasNext())
                {
                    if (currentKeyBinding == rootKeyBinding) {
                        combinationKeyModifier = keyModifier;
                        combinationKeys = pressedKey.toString().toUpper();
                    } else {
                        combinationKeys << "," << pressedKey.toString().toUpper();
                    }
                    currentKeyBinding = foundBinding.get().getNext();
                    rslt = EVENT_PROCESSED;
                }
            }
            if (   currentKeyBinding != rootKeyBinding 
                && rslt != EVENT_PROCESSED)
            {
                currentKeyBinding = rootKeyBinding;
                rslt = EVENT_PROCESSED;
            }
            if (currentKeyBinding != rootKeyBinding) {
                if (invokedPanel.isValid()) {
                    invokedPanel->treatFocusOut();
                } else {
                    textEditor->treatFocusOut();
                }
                statusLine->setMessage(String() << "Key combination: " 
                                                << (  combinationKeyModifier.toString().getLength() > 0
                                                    ? (combinationKeyModifier.toString() << "+")
                                                    : "")
                                                << combinationKeys
                                                << ", ...");
            } else {
                if (invokedPanel.isValid()) {
                    invokedPanel->treatFocusIn();
                } else {
                    textEditor->treatFocusIn();
                }
                statusLine->clearMessage();
            }
        }
        
        if (rslt == NOT_PROCESSED && invokedPanel.isValid()) {
            rslt = invokedPanel->processKeyboardEvent(event);
        }
        
        if (rslt == NOT_PROCESSED)
        {
           m = keyMapping2->find(keyModifier, pressedKey);
    
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
    catch (UnknownActionNameException& ex)
    {
        setMessageBox(MessageBoxParameter().setTitle("Config Error")
                                           .setMessage(ex.getMessage()));
        return NOT_PROCESSED;
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
    if (hasMessageBox) {
        if (messageBox.isInvalid()) {
            internalInvokeNewMessageBox();
        }
        if (isMessageBoxModal) {
            textEditor->disableCursorChanges();
        }
        messageBox->raise();
        messageBox->requestFocus();
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
    try
    {
        textData->checkFileInfo();
    
        if (   textData->wasFileModifiedOnDisk() 
            && (  !textData->hasModifiedOnDiskFlagBeenIgnored()
                || textData->wasFileModifiedOnDiskSinceLastIgnore()))
        {
            setMessageBox(MessageBoxParameter().setTitle("File Modified")
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
    catch (FileException& ex) {
        setMessageBox(MessageBoxParameter().setTitle("File Error")
                                           .setMessage(ex.getMessage()));
        return false;
    }
}


void EditorTopWin::reloadFile()
{
    textData->reloadFile();
    textEditor->releaseSelection();
}

void EditorTopWin::doNotReloadFile()
{
    textData->setIgnoreModifiedOnDiskFlag(true);
}

void EditorTopWin::treatFocusOut()
{
    if (currentKeyBinding != rootKeyBinding) {
        currentKeyBinding = rootKeyBinding;
        if (invokedPanel.isValid()) {
            invokedPanel->treatFocusIn();
        } else {
            textEditor->treatFocusIn();
        }
        statusLine->clearMessage();
    }
    if (invokedPanel.isValid()) {
        invokedPanel->treatFocusOut();
    } else {
        textEditor->treatFocusOut();
    }
}

void EditorTopWin::setModalMessageBox(const MessageBoxParameter& p)
{
    isMessageBoxModal = true;
    internalSetMessageBox(p);
}

void EditorTopWin::setMessageBox(const MessageBoxParameter& p)
{
    isMessageBoxModal = false;
    internalSetMessageBox(p);
}

void EditorTopWin::internalSetMessageBox(const MessageBoxParameter& p)
{
    hasMessageBox = true;
    this->messageBoxParameter = p;
    
    if (messageBox.isValid())
    {
        MessageBox::Ptr oldBox = messageBox;
                                 messageBox.invalidate();
        oldBox->hide();
        oldBox->requestCloseWindow(TopWin::CLOSED_SILENTLY);
    }
    if (isVisible())
    {
        if (isMessageBoxModal) {
            textEditor->disableCursorChanges();
        }
        internalInvokeNewMessageBox();
        messageBox->raise();
        messageBox->requestFocus();
    }
}

void EditorTopWin::internalInvokeNewMessageBox()
{
    messageBox = MessageBox::create(this, messageBoxParameter);
    messageBox->registerRequestForCloseNotifyCallback(newCallback(this, &EditorTopWin::notifyRequestCloseChildWindow));
    messageBox->show();
}

void EditorTopWin::closeMessageBox()
{
    if (hasMessageBox) {
        hasMessageBox = false;
        if (isMessageBoxModal) {
            textEditor->enableCursorChanges();
        }
        if (messageBox.isValid()) {
            messageBox->hide();
            messageBox->requestCloseWindow(TopWin::CLOSED_SILENTLY);
        }
    }
}

void EditorTopWin::notifyRequestCloseChildWindow(TopWin* topWin, TopWin::CloseReason reason)
{
    if (!isClosing() && hasMessageBox && topWin == messageBox)
    {
        hasMessageBox = false;
        
        if (isMessageBoxModal) {
            isMessageBoxModal = false;
            textEditor->enableCursorChanges();
        }
        
        if (reason == TopWin::CLOSED_BY_USER)
        {
            this->requestFocus();
            this->raise();
        }
    }
}

void EditorTopWin::invokeSaveAsPanel(Callback<>::Ptr saveCallback)
{
    if (saveAsPanel.isInvalid()) {
        saveAsPanel = SaveAsPanel::create(this, textEditor, 
                                                newCallback(this, &EditorTopWin::setMessageBox),
                                                panelInvoker);
    }
    saveAsPanel->setSaveCallback(saveCallback);
    invokePanel(saveAsPanel);
}


void EditorTopWin::invokePanel(DialogPanel* panel)
{
    if (invokedPanel != panel) {
        if (invokedPanel.isValid()) {
            requestCloseFor(invokedPanel);
        }
        ASSERT(invokedPanel.isInvalid());
        if (GlobalConfig::getInstance()->isEditorPanelOnTop()) {
            textEditor->setVerticalAdjustmentStrategy(TextWidget::BOTTOM_LINE_ANCHOR);
            this->invokedPanelIndex = upperPanelIndex;
        } else {
            textEditor->setVerticalAdjustmentStrategy(TextWidget::STRICT_TOP_LINE_ANCHOR);
            this->invokedPanelIndex = lowerPanelIndex;
        }
        rootElement->insertElementAtPosition(PanelLayoutAdapter::create(textEditor, panel), 
                                             this->invokedPanelIndex);
        Position p = getPosition();
        rootElement->setPosition(Position(0, 0, p.w, p.h));
        panel->show();
        panel->treatFocusIn();
        textEditor->treatFocusOut();
        invokedPanel = panel;
        textEditor->setVerticalAdjustmentStrategy(TextWidget::STRICT_TOP_LINE_ANCHOR);
    }
}

void EditorTopWin::requestCloseFor(GuiWidget* w)
{
    if (w == invokedPanel) {
        if (messageBox.isValid()) {
            messageBox->requestCloseWindow(TopWin::CLOSED_SILENTLY);
        }
        rootElement->removeElementAtPosition(this->invokedPanelIndex);
        w->hide();
        w->treatFocusOut();
        textEditor->treatFocusIn();
        invokedPanel.invalidate();
        Position p = getPosition();
        rootElement->setPosition(Position(0, 0, p.w, p.h));
        textEditor->setVerticalAdjustmentStrategy(TextWidget::STRICT_TOP_LINE_ANCHOR);
    }
}

void EditorTopWin::setWindowTitle()
{
    File file(textData->getFileName());
    
    String title;
    
    if (EditorServer::getInstance()->getInstanceName().getLength() > 0) {
        title << EditorServer::getInstance()->getInstanceName() << ": ";
    }
    
    title << file.getBaseName();

    if (textData->getModifiedFlag() == true
     && textData->isReadOnly())
    {
        title << " (read only, modified)";
    } else if (textData->isReadOnly()) {
        title << " (read only)";
    } else if (textData->getModifiedFlag() == true) {
        title << " (modified)";
    }
    title << " - " << file.getDirName() << "/ ["
          << System::getInstance()->getUserName() << "@" 
          << System::getInstance()->getHostName() << "]";

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
        if (textData->isFileNamePseudo()) {
            invokeSaveAsPanel(newCallback(this, &EditorTopWin::handleSaveKey));
        }
        else {
            textData->save();
            GlobalConfig::getInstance()->notifyAboutNewFileContent(textData->getFileName());
        }
    } catch (FileException& ex) {
        setMessageBox(MessageBoxParameter().setTitle("File Error")
                                           .setMessage(ex.getMessage()));
    } catch (LuaException& ex) {
        setMessageBox(MessageBoxParameter().setTitle("Lua Error")
                                           .setMessage(ex.getMessage()));
    } catch (ConfigException& ex)
    {
        if (ex.getErrorList().isValid() && ex.getErrorList()->getLength() > 0) {
            ConfigErrorHandler::start(ex.getErrorList());
        } else {
            setMessageBox(MessageBoxParameter().setTitle("Config Error")
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
        if (textData->isFileNamePseudo()) {
            invokeSaveAsPanel(newCallback(this, &EditorTopWin::saveAndClose));
        }
        else {
            textData->save();
            GlobalConfig::getInstance()->notifyAboutNewFileContent(textData->getFileName());
            requestCloseWindow(TopWin::CLOSED_SILENTLY);
        }
    } catch (FileException& ex) {
        setMessageBox(MessageBoxParameter().setTitle("File Error")
                                           .setMessage(ex.getMessage()));
    } catch (LuaException& ex) {
        setMessageBox(MessageBoxParameter().setTitle("Lua Error")
                                           .setMessage(ex.getMessage()));
    } catch (ConfigException& ex) {
        setMessageBox(MessageBoxParameter().setTitle("Config Error")
                                           .setMessage(ex.getMessage()));
    }
}

void EditorTopWin::requestCloseWindowByUser()
{
    requestCloseWindow(TopWin::CLOSED_BY_USER);
}

void EditorTopWin::requestCloseWindow(TopWin::CloseReason reason)
{
    File file(textData->getFileName());

    if (ViewCounterTextDataAccess::getViewCounter(textData) == 1
     && textData->getModifiedFlag() == true)
    {
        setMessageBox(MessageBoxParameter().setTitle("Save File")
                                           .setMessage(String() << "Save file '" << file.getBaseName() 
                                                                << "' before closing?")
                                           .setDefaultButton(    "S]ave",    newCallback(this, &EditorTopWin::saveAndClose))
                                           .setAlternativeButton("D]iscard", newCallback(this, &EditorTopWin::requestCloseWindowAndDiscardChanges)));
        
    }
    else
    {
        TopWin::requestCloseWindow(reason);
    }
}

void EditorTopWin::requestCloseWindowAndDiscardChanges()
{
    TopWin::requestCloseWindow(TopWin::CLOSED_SILENTLY);
}

void EditorTopWin::createEmptyWindow()
{
    TextStyles::Ptr   textStyles   = GlobalConfig::getInstance()->getTextStyles();
    LanguageMode::Ptr languageMode = GlobalConfig::getInstance()->getDefaultLanguageMode();

    String untitledFileName = File(String() << textData->getFileName()).getDirName() << "/Untitled";
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
            
            LuaInterpreter::Result scriptResult = LuaInterpreter::getInstance()->executeScript((const char*) textData->getAmount(selBegin, selLength),
                                                                                               selLength);
            String output = scriptResult.output;
            
            TextData::HistorySection::Ptr historySectionHolder = textData->createHistorySection();
            
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
                byte c = textData->getChar(spos - 1);
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
                LuaInterpreter::Result scriptResult = LuaInterpreter::getInstance()->executeExpression((const char*) textData->getAmount(spos, cursorPos - spos),
                                                                                                       cursorPos - spos);
                String output = scriptResult.output;
                for (int i = 0, n = scriptResult.objects.getLength(); i < n; ++i) {
                    output << scriptResult.objects[i].toString();
                }
                if (output.getLength() > 0) 
                {
                    TextData::HistorySection::Ptr historySectionHolder = textData->createHistorySection();
                    
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
        setMessageBox(MessageBoxParameter().setTitle("Lua Error")
                                           .setMessage(ex.getMessage()));
    }
}

String EditorTopWin::getFileName() const
{
    return textData->getFileName();
}

void EditorTopWin::executeTestScript()
{
    printf("-------------------------\n");
    ProgramExecutor::start("/bin/sh",
                           "echo 1234567890; ls|head; exit 12",
                           newCallback(this, &EditorTopWin::finishedTestScript));
}

void EditorTopWin::finishedTestScript(ProgramExecutor::Result rslt)
{
    if (rslt.outputLength > 0)
    {
        TextData::Ptr textData = TextData::create();
        textData->setToData(rslt.outputBuffer,
                            rslt.outputLength);
    
        CommandOutputBox::Ptr commandOutputBox = CommandOutputBox::create(this, textData);
        commandOutputBox->show();
    }
}


