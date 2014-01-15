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
#include "EncodingException.hpp"
#include "LuaInterpreter.hpp"
#include "LuaException.hpp"
#include "ConfigException.hpp"
#include "WindowCloser.hpp"
#include "System.hpp"
#include "ConfigErrorHandler.hpp"
#include "ProgramExecutor.hpp"
#include "CommandOutputBox.hpp"
#include "PanelInvoker.hpp"
#include "SaveAsPanel.hpp"
#include "EditorTopWinActions.hpp"
#include "UnknownActionNameException.hpp"
#include "LucedLuaInterface.hpp"
#include "GlobalLuaInterpreter.hpp"
#include "QualifiedName.hpp"
#include "EncodingConverter.hpp"
#include "LuaErrorHandler.hpp"
#include "UserDefinedActionMethods.hpp"

using namespace LucED;

namespace // anonymous namespace
{

class PanelLayoutAdapter : public GuiLayoutAdapter
{
public:
    typedef OwningPtr<PanelLayoutAdapter> Ptr;
    
    static Ptr create(RawPtr<MultiLineEditorWidget> editorWidget, RawPtr<GuiElement> panel) {
        return Ptr(new PanelLayoutAdapter(editorWidget, panel));
    }

    virtual GuiMeasures getDesiredMeasures() {
        GuiMeasures rslt = internalGetDesiredMeasures(panel);
        int lineHeight = editorWidget->getLineHeight();
        rslt.minHeight  = ROUNDED_UP_DIV(rslt.minHeight,  lineHeight) * lineHeight;
        rslt.bestHeight = ROUNDED_UP_DIV(rslt.bestHeight, lineHeight) * lineHeight;
        if (rslt.maxHeight != INT_MAX) {
            rslt.maxHeight = ROUNDED_UP_DIV(rslt.maxHeight, lineHeight) * lineHeight;
        }
        return rslt;
    }
    
private:
    PanelLayoutAdapter(RawPtr<MultiLineEditorWidget> editorWidget, RawPtr<GuiElement> panel)
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

    virtual void invokePanel(DialogPanel::Ptr panel) {
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
    virtual void closePanel(RawPtr<DialogPanel> panel) {
        if (panel.isValid()) {
            editorTopWin->requestCloseFor(panel);
        }
    }

private:
    PanelInvoker(RawPtr<EditorTopWin> editorTopWin)
        : editorTopWin(editorTopWin)
    {}
    RawPtr<EditorTopWin> editorTopWin;
};

class EditorTopWin::ActionInterface : public LucED::TopWinActionInterface
{
public:
    typedef OwningPtr<ActionInterface> Ptr;
    
    static Ptr create(RawPtr<EditorTopWin> editorTopWin)
    {
        return Ptr(new ActionInterface(editorTopWin));
    }
    
    virtual void requestCloseWindowByUser()
    {
        editorTopWin->requestCloseWindow(TopWin::CLOSED_BY_USER);
    }
    
    virtual void handleSaveKey()
    {
        try {
            RawPtr<TextData> textData = editorTopWin->textData;
            
            if (textData->isFileNamePseudo()) {
                editorTopWin->invokeSaveAsPanel(newCallback(this, &ActionInterface::handleSaveKey));
            }
            else {
                editorTopWin->save();
            }
        } catch (...) {
            editorTopWin->handleCatchedException();
        }
    }
    
    virtual void handleSaveAsKey()
    {
        editorTopWin->invokeSaveAsPanel(newCallback(this, &ActionInterface::handleSaveKey));
    }
    
    virtual void createEmptyWindow()
    {
        LanguageMode::Ptr languageMode = GlobalConfig::getInstance()->getDefaultLanguageMode();
     
        String untitledFileName = File(String() << editorTopWin->textData->getFileName()).getDirName() << "/Untitled";
        TextData::Ptr     emptyTextData     = TextData::create();
                          emptyTextData->setPseudoFileName(untitledFileName);
     
        HilitedText::Ptr  hilitedText  = HilitedText::create(emptyTextData, languageMode);
     
        EditorTopWin::Ptr win          = EditorTopWin::create(hilitedText);
                          win->show();
    }
    
    virtual void createCloneWindow()
    {
        Position myPosition = editorTopWin->getPosition();
      
        EditorTopWin::Ptr newWin = EditorTopWin::create(editorTopWin->textEditor->getHilitedText(),
                                                        myPosition.w, 
                                                        myPosition.h);
        newWin->textEditor->moveCursorToTextMark(editorTopWin->textEditor->createNewMarkFromCursor());
        newWin->textEditor->setTopLineNumber(    editorTopWin->textEditor->getTopLineNumber());
        newWin->textEditor->setLeftPix(          editorTopWin->textEditor->getLeftPix());
        newWin->show();
    }
    
private:
    ActionInterface(RawPtr<EditorTopWin> editorTopWin)
        : editorTopWin(editorTopWin)
    {}
    RawPtr<EditorTopWin> editorTopWin;
};

class EditorTopWin::ShellInvocationHandler : public UserDefinedActionMethods::ShellInvocationHandler
{
public:
    static Ptr create(WeakPtr<EditorTopWin> e) {
        return Ptr(new ShellInvocationHandler(e));
    }

    void beforeShellInvocation(bool isFilter) {
        this->isFilter = isFilter;
        if (e.isValid()) {
            if (isFilter) {
                e->textEditor->disableCursorChanges();
            }
            else if (e->textData->getModifiedFlag() == true) {
                if (!e->textData->isFileNamePseudo()) {
                    e->save();
                }
            }
        }
    }
    void afterShellInvocation(ProgramExecutor::Result rslt) {
        if (e.isValid()) {
            if (isFilter) {
                e->textEditor->enableCursorChanges();
            }
            e->finishedShellscript(rslt, isFilter);
        }
        isFilter = false;
    }

private:
    ShellInvocationHandler(WeakPtr<EditorTopWin> e)
        : e(e),
          isFilter(false)
    {}
    WeakPtr<EditorTopWin> e;
    bool                  isFilter;
};


EditorTopWin::EditorTopWin(HilitedText::Ptr hilitedText, int width, int height)
    : rootElement(GuiLayoutColumn::create()),
      hasInvokedPanelFocus(false),
      hasMessageBox(false),
      isMessageBoxModal(false),
      actionMethodContainer(ActionMethodContainer::create()),
      actionKeySequenceHandler(actionMethodContainer)
{
    TopWin::setFocusManager(this);
    TopWin::setRootElement(rootElement);
    
    statusLine = StatusLine::create();
    int statusLineIndex = rootElement->addElement(statusLine);
    upperPanelIndex = statusLineIndex + 1;
    
    textEditor = MultiLineEditorWidget::create(hilitedText);

    textEditor->setVerticalAdjustmentStrategy(TextWidget::STRICT_TOP_LINE_ANCHOR);


    scrollableTextCompound = ScrollableTextGuiCompound::create(textEditor,
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
            GlobalConfig::getConfigData()->getGeneralConfig()->getInitialWindowWidth(),
            GlobalConfig::getConfigData()->getGeneralConfig()->getInitialWindowHeight()
    );



    GuiElement::Measures m = rootElement->getDesiredMeasures();

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

    GlobalConfig::getInstance()->registerConfigChangedCallback(newCallback(this, &EditorTopWin::treatConfigUpdate));

    actionInterface = EditorTopWin::ActionInterface::create(this);

    panelInvoker = PanelInvoker::create(this);

    actionMethodContainer
    ->addActionMethods(EditorTopWinActions::create(
                       TopWinActionsParameter(textEditor,
                                              newCallback(this, &EditorTopWin::setModalMessageBox),
                                              panelInvoker,
                                              actionInterface,
                                              newCallback(this, &EditorTopWin::handleCatchedException))));

    UserDefinedActionMethods::Ptr userActions = UserDefinedActionMethods::create(textEditor,
                                                                                 ShellInvocationHandler::create(this));
    textEditor->getKeyActionHandler()->addActionMethods(userActions);
}


void EditorTopWin::processGuiWidgetCreatedEvent()
{
    getGuiWidget()->addToXEventMask(ButtonPressMask);

    TopWin::processGuiWidgetCreatedEvent();
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
        GlobalConfig::LanguageModeAndEncoding result = GlobalConfig::getInstance()
                                                       ->getLanguageModeAndEncodingForFileNameAndContent
                                                         (
                                                             textData->getFileName(), 
                                                             textData->getByteBuffer()
                                                         );
        if (result.encoding.getLength() > 0) {
            textData->setEncoding(result.encoding);
        }
        newLanguageMode = result.languageMode;
    }
    textEditor->getHilitedText()->setLanguageMode(newLanguageMode);
}


void EditorTopWin::show()
{
    if (rootElement.isValid())
    {
    }
    TopWin::show();
}


void EditorTopWin::hide()
{
    TopWin::hide();
}


void EditorTopWin::processGuiWidgetNewPositionEvent(const Position& newPosition)
{
    TopWin::processGuiWidgetNewPositionEvent(newPosition);
    rootElement->setPosition(Position(0, 0, newPosition.w, newPosition.h));
}

void EditorTopWin::handleCatchedException()
{
    try
    {
        throw;
    }
    catch (FileException& ex) {
        setMessageBox(MessageBoxParameter().setTitle("File Error")
                                           .setMessage(ex.getMessage()));
    }
    catch (EncodingException& ex) {
        setMessageBox(MessageBoxParameter().setTitle("Encoding Error")
                                           .setMessage(ex.getMessage()));
    }
    catch (LuaException& ex) {
        LuaErrorHandler::start(ex.getExceptionLuaInterface(),
                               newCallback(this, &EditorTopWin::setMessageBox),
                               newCallback(this, &EditorTopWin::handleCatchedException));
    }
    catch (ConfigException& ex) {
        ConfigErrorHandler::startWithCatchedException();
    }
    catch (UnknownActionNameException& ex)
    {
        setMessageBox(MessageBoxParameter().setTitle("Config Error")
                                           .setMessage(ex.getMessage()));
    }
    catch (BaseException& ex) {
        setMessageBox(MessageBoxParameter().setTitle("Internal Error")
                                           .setMessage(ex.getMessage()));
    }
}


GuiWidget::ProcessingResult EditorTopWin::processKeyboardEvent(const KeyPressEvent& keyPressEvent)
{
    try
    {
        KeyId       pressedKey  = keyPressEvent.getKeyId();
        KeyModifier keyModifier = keyPressEvent.getKeyModifier();
    
        GuiWidget::ProcessingResult rslt = GuiWidget::NOT_PROCESSED;
    
        if (!pressedKey.isModifierKey())
        {
            textEditor->hideMousePointer();
            
            RawPtr<FocusableElement> focusedWidget;
            
            if (invokedPanel.isValid() && hasInvokedPanelFocus) {
                focusedWidget = invokedPanel;
            } else {
                focusedWidget = textEditor;
            }
            if (actionKeySequenceHandler.handleKeyPress(keyPressEvent, 
                                                        focusedWidget->getKeyActionHandler()))
            {
                rslt = GuiWidget::EVENT_PROCESSED;
            }

            if (   actionKeySequenceHandler.isWithinSequence() 
                && rslt != GuiWidget::EVENT_PROCESSED)
            {
                actionKeySequenceHandler.reset();
                rslt = GuiWidget::EVENT_PROCESSED;
            }
            if (actionKeySequenceHandler.hasJustEnteredSequence())
            {
                if (invokedPanel.isValid() && hasInvokedPanelFocus) {
                    invokedPanel->treatFocusOut();
                } else {
                    textEditor->treatFocusOut();
                }
                EventDispatcher::getInstance()->registerBeforeMouseClickListener(newCallback(this, &EditorTopWin::handleBeforeMouseClick));
                statusLine->setMessage(String() << "Key combination: " 
                                                << actionKeySequenceHandler.getKeySequenceAsString()
                                                << ", ...");
            }
            else if (actionKeySequenceHandler.hasJustQuitSequence()) 
            {
                if (invokedPanel.isValid() && hasInvokedPanelFocus) {
                    invokedPanel->treatFocusIn();
                } else {
                    textEditor->treatFocusIn();
                }
                statusLine->clearMessage();
            }
        }
        return rslt;
    }
    catch (...) {
        handleCatchedException();
        return GuiWidget::EVENT_PROCESSED;
    }
}

void EditorTopWin::handleBeforeMouseClick()
{
    if (actionKeySequenceHandler.isWithinSequence())
    {
        actionKeySequenceHandler.reset();
        statusLine->clearMessage();
        if (hasFocus()) {
            if (invokedPanel.isValid()) {
                invokedPanel->treatFocusIn();
            } else {
                textEditor->treatFocusIn();
            }
        }
    }
    EventDispatcher::getInstance()->deregisterBeforeMouseClickListenerFor(this);
}

GuiWidget::ProcessingResult EditorTopWin::processGuiWidgetEvent(const XEvent* event)
{
    if (TopWin::processGuiWidgetEvent(event) == GuiWidget::EVENT_PROCESSED) {
        return GuiWidget::EVENT_PROCESSED;
    } else {
        switch (event->type)
        {
            case ButtonPress: {
                if (event->xbutton.button == Button4
                 || event->xbutton.button == Button5)
                {
                    textEditor->processGuiWidgetEvent(event);
                    return GuiWidget::EVENT_PROCESSED;
                }
            }
        }
        return GuiWidget::NOT_PROCESSED;
    }
}

void EditorTopWin::reportMouseClickFrom(RawPtr<FocusableElement> w)
{
    requestFocusFor(w);
}

void EditorTopWin::requestFocusFor(RawPtr<FocusableElement> w)
{
    if (!hasMessageBox || !isMessageBoxModal)
    {
        if (w == textEditor) {
            textEditor->treatFocusIn();
            if (invokedPanel.isValid()) {
                invokedPanel->treatFocusOut();
            }
            hasInvokedPanelFocus = false;
        }
        else if (w == invokedPanel) {
            invokedPanel->treatFocusIn();
            textEditor->treatFocusOut();
            hasInvokedPanelFocus = true;
        }
    }
}

void EditorTopWin::requestHotKeyRegistrationFor(const KeyMapping::Id& id, RawPtr<FocusableElement> w)
{
    ASSERT(false);
}

void EditorTopWin::requestRemovalOfHotKeyRegistrationFor(const KeyMapping::Id& id, RawPtr<FocusableElement> w)
{
    ASSERT(false);
}


void EditorTopWin::treatFocusIn()
{
    if (hasMessageBox) {
        if (messageBox.isInvalid()) {
            internalInvokeNewMessageBox();
        }
        if (isMessageBoxModal) {
            //textEditor->disableCursorChanges();
            messageBox->raise();
            messageBox->requestFocus();
        } else {
            textEditor->treatFocusIn();
        }
    } else {
        if (invokedPanel.isValid() && hasInvokedPanelFocus) {
            invokedPanel->treatFocusIn();
        } else {
            textEditor->treatFocusIn();
        }
        checkForFileModifications();
    }
    LucedLuaInterface::getInstance()->setCurrentView(getViewLuaInterface());
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
    catch (...) {
        handleCatchedException();
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
    if (actionKeySequenceHandler.isWithinSequence())
    {
        actionKeySequenceHandler.reset();
        if (invokedPanel.isValid() && hasInvokedPanelFocus) {
            invokedPanel->treatFocusIn();
        } else {
            textEditor->treatFocusIn();
        }
        statusLine->clearMessage();
    }

    if (invokedPanel.isValid() && hasInvokedPanelFocus) {
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
//            textEditor->disableCursorChanges();
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

void EditorTopWin::invokePanel(DialogPanel::Ptr panel)
{
    if (panel == invokedPanel) {
        panel->treatFocusIn();
        hasInvokedPanelFocus = true;
        textEditor->treatFocusOut();
    } else {
        if (invokedPanel.isValid()) {
            requestCloseFor(invokedPanel);
        }
        ASSERT(invokedPanel.isInvalid());
        if (GlobalConfig::getConfigData()->getGeneralConfig()->getEditorPanelOnTop()) {
            textEditor->setVerticalAdjustmentStrategy(TextWidget::BOTTOM_LINE_ANCHOR);
            this->invokedPanelIndex = upperPanelIndex;
        } else {
            textEditor->setVerticalAdjustmentStrategy(TextWidget::STRICT_TOP_LINE_ANCHOR);
            this->invokedPanelIndex = lowerPanelIndex;
        }
        panel->setLayoutAdapter(PanelLayoutAdapter::create(textEditor, panel));
        rootElement->insertElementAtPosition(panel, 
                                             this->invokedPanelIndex);
        Position p = getPosition();
        panel->show();
        rootElement->setPosition(Position(0, 0, p.w, p.h));
        panel->treatFocusIn();
        hasInvokedPanelFocus = true;
        textEditor->treatFocusOut();
        invokedPanel = panel;
        textEditor->setVerticalAdjustmentStrategy(TextWidget::STRICT_TOP_LINE_ANCHOR);
    }
}

void EditorTopWin::requestCloseFor(DialogPanel* w)
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
    String instanceName = GuiRoot::getInstance()->getInstanceName();
    if (instanceName.getLength() > 0) {
        title << instanceName << ": ";
    }
    
    title << EncodingConverter::convertLocaleToUtf8StringIgnoreErrors(file.getBaseName());

    if (textData->getModifiedFlag() == true
     && textData->isReadOnly())
    {
        title << " (read only, modified)";
    } else if (textData->isReadOnly()) {
        title << " (read only)";
    } else if (textData->getModifiedFlag() == true) {
        title << " (modified)";
    }
    title << " - " 
          << EncodingConverter::convertLocaleToUtf8StringIgnoreErrors(file.getDirName())
          << "/ ["
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

void EditorTopWin::save()
{
    {
        GlobalConfig::LanguageModeAndEncoding result = GlobalConfig::getInstance()
                                                       ->getLanguageModeAndEncodingForFileNameAndContent
                                                         (
                                                             textData->getFileName(), 
                                                             textData->getByteBuffer()
                                                         );
        if (result.encoding.getLength() > 0 && EncodingConverter::canConvertFromTo("UTF-8", result.encoding)) {
            textData->setEncoding(result.encoding);
        }
        if (result.languageMode != textEditor->getHilitedText()->getLanguageMode()) {
            textEditor->getHilitedText()->setLanguageMode(result.languageMode);
        }
    }
    textData->save();
    GlobalConfig::getInstance()->notifyAboutNewFileContent(textData->getFileName());
}


void EditorTopWin::saveAndClose()
{
    try {
        if (textData->isFileNamePseudo()) {
            invokeSaveAsPanel(newCallback(this, &EditorTopWin::saveAndClose));
        }
        else {
            this->save();
            requestCloseWindow(TopWin::CLOSED_SILENTLY);
        }
    } catch (...) {
        handleCatchedException();
    }
}

void EditorTopWin::invokeSaveAsPanel(Callback<>::Ptr saveCallback)
{
    if (saveAsPanel.isInvalid()) {
        saveAsPanel = SaveAsPanel::create(this->textEditor, 
                                          newCallback(this, &EditorTopWin::setMessageBox),
                                          newCallback(this, &EditorTopWin::closeSaveAsPanel));
    }
    saveAsPanel->setSaveCallback(saveCallback);
    invokePanel(saveAsPanel);
}

void EditorTopWin::closeSaveAsPanel()
{
    panelInvoker->closePanel(saveAsPanel);
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



String EditorTopWin::getFileName() const
{
    return textData->getFileName();
}


void EditorTopWin::finishedShellscript(ProgramExecutor::Result rslt, bool wasFilter)
{
    if (wasFilter && rslt.returnCode == 0) {
        String outputData = rslt.outputBuffer->toString();
        try {
            EncodingConverter converter(System::getInstance()->getDefaultEncoding(), "UTF-8");
 
            if (converter.isConvertingBetweenDifferentCodesets()) {
                outputData = converter.convertStringToString(outputData);
            }
        }
        catch (EncodingException& ex) {
            // ignore, take outputData as it is
        }
        textEditor->replaceSelection(outputData);
    }
    else {
        textData->checkFileInfo();
        if (textData->wasFileModifiedOnDisk()) {
            reloadFile();
        }
        if (rslt.outputBuffer->getLength() > 0)
        {
            TextData::Ptr textData = TextData::create();
    
            textData->takeOverBuffer(System::getInstance()->getDefaultEncoding(), 
                                     rslt.outputBuffer);
        
            CommandOutputBox::Ptr commandOutputBox = CommandOutputBox::create(this, textData);
            commandOutputBox->show();
        }
    }
}


