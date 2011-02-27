/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2009 Oliver Schmidt, oliver at luced dot de
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

#ifndef EDITOR_TOP_WIN_HPP
#define EDITOR_TOP_WIN_HPP

#include "TopWin.hpp"
#include "EventDispatcher.hpp"
#include "ScrollBar.hpp"
#include "MultiLineEditorWidget.hpp"
#include "TextData.hpp"
#include "TextStyle.hpp"
#include "StatusLine.hpp"
#include "GuiLayoutColumn.hpp"
#include "OwningPtr.hpp"
#include "TopWinList.hpp"
#include "GotoLinePanel.hpp"
#include "MessageBox.hpp"
#include "SaveAsPanel.hpp"
#include "ProgramExecutor.hpp"
#include "RawPtr.hpp"
#include "ScrollableTextGuiCompound.hpp"
#include "ActionKeyConfig.hpp"
#include "ActionKeySequenceHandler.hpp"
#include "ViewLuaInterface.hpp"
#include "FocusableElement.hpp"
#include "ActionMethodContainer.hpp"
                
namespace LucED
{

class ViewLuaInterface;

class EditorTopWin : public  TopWin,
                     public  FocusManager,
                     private ViewCounterTextDataAccess
{
public:
    typedef WeakPtr<EditorTopWin> Ptr;
    
    static EditorTopWin::Ptr create(HilitedText::Ptr hilitedText, int width = -1, int height = -1)
    {
        return transferOwnershipTo(
                new EditorTopWin(hilitedText, width, height),
                TopWinList::getInstance());
    }
    
    ~EditorTopWin();
    
    RawPtr<ViewLuaInterface> getViewLuaInterface() const {
        return viewLuaInterface;
    }
    
    virtual GuiWidget::ProcessingResult processKeyboardEvent(const KeyPressEvent& keyPressEvent);

    virtual void treatFocusIn();
    virtual void treatFocusOut();
    virtual void show();
    virtual void hide();
    
    void invokeSaveAsPanel(Callback<>::Ptr saveCallback);
    void closeSaveAsPanel();
    
    virtual void requestCloseWindow(TopWin::CloseReason reason);

    String getFileName() const;
    
    HilitedText::Ptr getHilitedText() {
        return textEditor->getHilitedText();
    }
    
    void setModalMessageBox(const MessageBoxParameter& messageBoxParameter);
    void closeMessageBox();

    void setMessageBox(const MessageBoxParameter& p);
    
    bool hasUnsavedData() const {
        return textData->getModifiedFlag();
    }

    void requestCloseWindowAndDiscardChanges();
    void save();
    void saveAndClose();
    
    bool checkForFileModifications();
    
    void gotoLineNumber(int lineNumber);

protected:
    virtual void processGuiWidgetCreatedEvent();

protected: // GuiWidget::EventListener interface implementation
    virtual GuiWidget::ProcessingResult processGuiWidgetEvent(const XEvent* event);
    virtual void                        processGuiWidgetNewPositionEvent(const Position& newPosition);

private: // FocusManager methods 
    virtual void requestHotKeyRegistrationFor(const KeyMapping::Id& id, RawPtr<FocusableElement> w);
    virtual void requestRemovalOfHotKeyRegistrationFor(const KeyMapping::Id& id, RawPtr<FocusableElement> w);
    virtual void requestFocusFor(RawPtr<FocusableElement> w);
    virtual void reportMouseClickFrom(RawPtr<FocusableElement> w);

private:
    class PanelInvoker;
    class ActionInterface;
    class UserDefinedActionMethods;
    
    EditorTopWin(HilitedText::Ptr hilitedText, int width, int height);

    void treatConfigUpdate();

    void requestCloseFor(DialogPanel* w);

    void invokePanel(DialogPanel::Ptr panel);
    
    void handleNewFileName(const String& fileName);
    void handleChangedModifiedFlag(bool modifiedFlag);
    void handleChangedReadOnlyFlag(bool readOnlyFlag);
    void handleBeforeMouseClick();
        
    void reloadFile();
    void doNotReloadFile();
    
    void setWindowTitle();
    
    void notifyRequestCloseChildWindow(TopWin* topWin, TopWin::CloseReason reason);
    void internalInvokeNewMessageBox();
    void internalSetMessageBox(const MessageBoxParameter& messageBoxParameter);

    void finishedShellscript(ProgramExecutor::Result result);
    
    MultiLineEditorWidget::Ptr textEditor;
    RawPtr<TextData>         textData;
    StatusLine::Ptr statusLine;
    GuiLayoutColumn::Ptr rootElement;
    GotoLinePanel::Ptr gotoLinePanel;

    int upperPanelIndex;
    int lowerPanelIndex;
    
    WeakPtr<DialogPanel> invokedPanel;
    int                  invokedPanelIndex;
    bool                 hasInvokedPanelFocus;

    MessageBox::Ptr     messageBox;
    MessageBoxParameter messageBoxParameter;
    bool                hasMessageBox;
    bool                isMessageBoxModal;
    
    SaveAsPanel::Ptr    saveAsPanel;
    
    ScrollableTextGuiCompound::Ptr scrollableTextCompound;
    
    KeyModifier             combinationKeyModifier;
    String                  combinationKeys;
    
    OwningPtr<PanelInvoker>             panelInvoker;
    OwningPtr<ActionInterface>          actionInterface;
    OwningPtr<UserDefinedActionMethods> userDefinedActionMethods;
    
    ActionMethodContainer::Ptr          actionMethodContainer;
    ActionKeySequenceHandler            actionKeySequenceHandler;
    
    OwningPtr<ViewLuaInterface> viewLuaInterface;
};

} // namespace LucED

#endif // EDITOR_TOP_WIN_HPP
