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
#include "SaveAsPanel.hpp"
#include "EditorTopWinActions.hpp"
#include "UnknownActionNameException.hpp"
#include "ViewLuaInterface.hpp"
#include "LucedLuaInterface.hpp"
#include "GlobalLuaInterpreter.hpp"
#include "QualifiedName.hpp"

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
                textData->save();
                GlobalConfig::getInstance()->notifyAboutNewFileContent(textData->getFileName());
            }
        } catch (FileException& ex) {
            editorTopWin->setMessageBox(MessageBoxParameter().setTitle("File Error")
                                                             .setMessage(ex.getMessage()));
        } catch (LuaException& ex) {
            ConfigErrorHandler::startWithCatchedException();
        }
        catch (ConfigException& ex) {
            ConfigErrorHandler::startWithCatchedException();
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


class EditorTopWin::UserDefinedActionMethods : public ActionMethods
{
public:
    typedef OwningPtr<UserDefinedActionMethods> Ptr;
    
    static Ptr create(RawPtr<EditorTopWin> thisTopWin) {
        return Ptr(new UserDefinedActionMethods(thisTopWin));
    }
    virtual bool invokeActionMethod(ActionId actionId);

    virtual bool hasActionMethod(ActionId actionId) {
        if (actionId.isBuiltin()) {
            return false;
        }
        return !getLuaActionFunction(actionId).isNil();
    }
private:
    UserDefinedActionMethods(RawPtr<EditorTopWin> thisTopWin)
        : thisTopWin(thisTopWin)
    {}
    LuaVar getLuaActionFunction(ActionId actionId)
    {
        QualifiedName fullActionName(actionId.toString());
        String        moduleName = fullActionName.getQualifier();
        LuaAccess     luaAccess  = GlobalLuaInterpreter::getInstance()->getCurrentLuaAccess();
        LuaVar        rslt(luaAccess);
        
        if (moduleName.getLength() > 0 && moduleName != "builtin")
        {
            LuaVar module(luaAccess);
            try
            {
                module = GlobalLuaInterpreter::getInstance()->requireConfigPackage(moduleName);
            } 
            catch (LuaException& ex) {
            }
            if (!module.isNil())
            {
                LuaVar actionGetter = module["getAction"];
                if (!actionGetter.isNil()) {
                    if (!actionGetter.isFunction()) {
                        throw ConfigException(String() << "'" << moduleName 
                                                       << ".getAction' must be function");
                    }
                    rslt = actionGetter.call(fullActionName.getName());
                }
            }
        }
        return rslt;
    }
    
    RawPtr<EditorTopWin> thisTopWin;
};

EditorTopWin::EditorTopWin(HilitedText::Ptr hilitedText, int width, int height)
    : rootElement(GuiLayoutColumn::create()),
      flagForSetSizeHintAtFirstShow(true),
      hasInvokedPanelFocus(false),
      hasMessageBox(false),
      isMessageBoxModal(false),
      actionMethodContainer(ActionMethodContainer::create()),
      actionKeySequenceHandler(actionMethodContainer)
{
    addToXEventMask(ButtonPressMask);
    
    statusLine = StatusLine::create(this);
    int statusLineIndex = rootElement->addElement(statusLine);
    upperPanelIndex = statusLineIndex + 1;
    
    textEditor = MultiLineEditorWidget::create(this, hilitedText);

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
    
    GlobalConfig::getInstance()->registerConfigChangedCallback(newCallback(this, &EditorTopWin::treatConfigUpdate));

    actionInterface = EditorTopWin::ActionInterface::create(this);

    panelInvoker = PanelInvoker::create(this);

    actionMethodContainer
    ->addActionMethods(EditorTopWinActions::create(
                       TopWinActionsParameter(this, 
                                              textEditor,
                                              newCallback(this, &EditorTopWin::setModalMessageBox),
                                              panelInvoker,
                                              actionInterface)));

    userDefinedActionMethods = UserDefinedActionMethods::create(this);
    textEditor->getKeyActionHandler()->addActionMethods(userDefinedActionMethods);
    
    viewLuaInterface = ViewLuaInterface::create(textEditor);
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

GuiElement::ProcessingResult EditorTopWin::processKeyboardEvent(const KeyPressEvent& keyPressEvent)
{
    try
    {
        KeyId       pressedKey  = keyPressEvent.getKeyId();
        KeyModifier keyModifier = keyPressEvent.getKeyModifier();
    
        ProcessingResult rslt = NOT_PROCESSED;
    
        if (!pressedKey.isModifierKey())
        {
            textEditor->hideMousePointer();
            
            RawPtr<FocusableWidget> focusedWidget;
            
            if (invokedPanel.isValid() && hasInvokedPanelFocus) {
                focusedWidget = invokedPanel;
            } else {
                focusedWidget = textEditor;
            }
            if (actionKeySequenceHandler.handleKeyPress(keyPressEvent, 
                                                        focusedWidget->getKeyActionHandler()))
            {
                rslt = EVENT_PROCESSED;
            }

            if (   actionKeySequenceHandler.isWithinSequence() 
                && rslt != EVENT_PROCESSED)
            {
                actionKeySequenceHandler.reset();
                rslt = EVENT_PROCESSED;
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
    catch (LuaException& ex) {
        ConfigErrorHandler::startWithCatchedException();
        return EVENT_PROCESSED;
    }
    catch (ConfigException& ex) {
        ConfigErrorHandler::startWithCatchedException();
        return EVENT_PROCESSED;
    }
    catch (UnknownActionNameException& ex)
    {
        setMessageBox(MessageBoxParameter().setTitle("Config Error")
                                           .setMessage(ex.getMessage()));
        return NOT_PROCESSED;
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

GuiElement::ProcessingResult EditorTopWin::processEvent(const XEvent* event)
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

void EditorTopWin::reportMouseClickFrom(GuiWidget* w)
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

void EditorTopWin::requestFocusFor(RawPtr<FocusableWidget> w)
{
    reportMouseClickFrom(w);
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
    LucedLuaInterface::getInstance()->setCurrentView(viewLuaInterface);
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



void EditorTopWin::invokePanel(DialogPanel* panel)
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
        ConfigErrorHandler::startWithCatchedException();
    } catch (ConfigException& ex) {
        ConfigErrorHandler::startWithCatchedException();
    }
}

void EditorTopWin::invokeSaveAsPanel(Callback<>::Ptr saveCallback)
{
    if (saveAsPanel.isInvalid()) {
        saveAsPanel = SaveAsPanel::create(this, this->textEditor, 
                                          newCallback(this, &EditorTopWin::setMessageBox),
                                          panelInvoker);
    }
    saveAsPanel->setSaveCallback(saveCallback);
    invokePanel(saveAsPanel);
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


void EditorTopWin::setSize(int width, int height)
{
    flagForSetSizeHintAtFirstShow = false;
    TopWin::setSize(width, height);
}



String EditorTopWin::getFileName() const
{
    return textData->getFileName();
}


bool EditorTopWin::UserDefinedActionMethods::invokeActionMethod(ActionId actionId)
{
    bool rslt = false;
    
    try
    {
        if (actionId.isBuiltin()) {
            return false;
        }

        LuaVar luaActionFunction = getLuaActionFunction(actionId);
                
        if (luaActionFunction.isFunction()) 
        {
            TextData::HistorySection::Ptr historySectionHolder = thisTopWin->textEditor->getTextData()->createHistorySection();

            luaActionFunction.call(thisTopWin->viewLuaInterface);

            rslt = true;
        }
        else if (luaActionFunction.isTable())
        {
            LuaVar shellScript = luaActionFunction["shellScript"];
            if (shellScript.isString())
            {
                String script = shellScript.toString();

                if (script.getLength() > 0)
                {
                    if (thisTopWin->textData->getModifiedFlag() == true) {
                        if (!thisTopWin->textData->isFileNamePseudo()) {
                            thisTopWin->textData->save();
                        }
                    }
                    HeapHashMap<String,String>::Ptr env = HeapHashMap<String,String>::create();
                                                    env->set("FILE", thisTopWin->getFileName());
                    ProgramExecutor::start("/bin/sh",
                                           script,
                                           env,
                                           newCallback(thisTopWin, &EditorTopWin::finishedShellscript));
                    rslt = true;
                }
            }
            else {
                throw ConfigException(String() << "Action '" << actionId.toString() << "' is table but has no field 'shellScript'");
            }
        }
        else {
            throw ConfigException(String() << "Action '" << actionId.toString() << "' must be function or table");
        }
        
        if (rslt)
        {
            thisTopWin->textEditor->showCursor();
            thisTopWin->textEditor->rememberCursorPixX();
        }
    }
    catch (FileException& ex) {
        thisTopWin->setMessageBox(MessageBoxParameter().setTitle("File Error")
                                                       .setMessage(ex.getMessage()));
    }
    catch (LuaException& ex) {
        ConfigErrorHandler::startWithCatchedException(actionId);
    }
    catch (ConfigException& ex) {
        ConfigErrorHandler::startWithCatchedException(actionId);
    }
    return rslt;
}

void EditorTopWin::finishedShellscript(ProgramExecutor::Result rslt)
{
    textData->checkFileInfo();
    if (textData->wasFileModifiedOnDisk()) {
        reloadFile();
    }
    if (rslt.outputLength > 0)
    {
        TextData::Ptr textData = TextData::create();
        textData->setToData(rslt.outputBuffer,
                            rslt.outputLength);
    
        CommandOutputBox::Ptr commandOutputBox = CommandOutputBox::create(this, textData);
        commandOutputBox->show();
    }
}


void EditorTopWin::gotoLineNumber(int lineNumber)
{
    bool wasNegative = (lineNumber < 0);
    
    if (lineNumber < 0) lineNumber = 0;
    
    TextData::TextMark m = textEditor->createNewMarkFromCursor();
    m.moveToLineAndColumn(lineNumber, m.getColumn());
    textEditor->moveCursorToTextMarkAndAdjustVisibility(m);
    textEditor->rememberCursorPixX();
    if (!wasNegative) {
        m.moveToBeginOfLine();    long spos = m.getPos();
        m.moveToNextLineBegin();  long epos = m.getPos();
        
        textEditor->setPrimarySelection(spos, epos);
    
    } else {
        textEditor->releaseSelection();
    }
}


