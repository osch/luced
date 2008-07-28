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
#include "KeyMapping.hpp"
#include "GotoLinePanel.hpp"
#include "MessageBox.hpp"
#include "SaveAsPanel.hpp"
#include "ProgramExecutor.hpp"
#include "RawPtr.hpp"
#include "ScrollableTextGuiCompound.hpp"
#include "TopWinActionInterface.hpp"
#include "ActionKeyConfig.hpp"
#include "ActionKeySequenceHandler.hpp"
                    
namespace LucED
{


class EditorTopWin : public  TopWin,
                     private ViewCounterTextDataAccess
{
public:
    typedef WeakPtr<EditorTopWin> Ptr;
    
    static EditorTopWin::Ptr create(
            TextStyles::Ptr textStyles, HilitedText::Ptr hilitedText, int width = -1, int height = -1)
    {
        return transferOwnershipTo(
                new EditorTopWin(textStyles, hilitedText, width, height),
                TopWinList::getInstance());
    }
    
    ~EditorTopWin();
    
    virtual ProcessingResult processEvent(const XEvent* event);
    
    virtual ProcessingResult processKeyboardEvent(const KeyPressEvent& keyPressEvent);

    virtual void treatNewWindowPosition(Position newPosition);
    virtual void treatFocusIn();
    virtual void treatFocusOut();
    virtual void setSize(int width, int height);
    virtual void show();
    
    void invokeSaveAsPanel(Callback<>::Ptr saveCallback);

    virtual void requestCloseWindow(TopWin::CloseReason reason);

    String getFileName() const;
    
    TextStyles::Ptr getTextStyles() {
        return textEditor->getTextStyles();
    }

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
    void saveAndClose();
    
    bool checkForFileModifications();

    virtual ActionKeyConfig::Ptr getActionKeyConfig() {
        return actionKeyConfig;
    }

private:
           class ActionInterface;
    friend class ActionInterface;
           class PanelInvoker;
    friend class PanelInvoker;
    
    EditorTopWin(TextStyles::Ptr textStyles, HilitedText::Ptr hilitedText, int width, int height);

    void treatConfigUpdate();

    void requestCloseFor(GuiWidget* w);

    void invokePanel(DialogPanel* panel);
    
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

    void executeTestScript();
    void finishedTestScript(ProgramExecutor::Result result);
    
    MultiLineEditorWidget::Ptr textEditor;
    RawPtr<TextData>         textData;
    StatusLine::Ptr statusLine;
    GuiLayoutColumn::Ptr rootElement;
    KeyMapping::Ptr keyMapping1;
    KeyMapping::Ptr keyMapping2;
    GotoLinePanel::Ptr gotoLinePanel;
    bool flagForSetSizeHintAtFirstShow;

    int upperPanelIndex;
    int lowerPanelIndex;
    
    WeakPtr<DialogPanel> invokedPanel;
    int                  invokedPanelIndex;

    MessageBox::Ptr     messageBox;
    MessageBoxParameter messageBoxParameter;
    bool                hasMessageBox;
    bool                isMessageBoxModal;
    
    SaveAsPanel::Ptr    saveAsPanel;
    
    ScrollableTextGuiCompound::Ptr scrollableTextCompound;
    
    KeyModifier             combinationKeyModifier;
    String                  combinationKeys;
    
    OwningPtr<PanelInvoker>    panelInvoker;
    OwningPtr<ActionInterface> actionInterface;

    ActionKeyConfig::Ptr     actionKeyConfig;    
    ActionKeySequenceHandler actionKeySequenceHandler;
};

} // namespace LucED

#endif // EDITOR_TOP_WIN_HPP
