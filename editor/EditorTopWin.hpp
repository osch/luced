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
#include "FindPanel.hpp"
#include "ReplacePanel.hpp"
#include "MessageBox.hpp"
#include "SaveAsPanel.hpp"
#include "ProgramExecutor.hpp"
#include "RawPtr.hpp"
#include "ScrollableTextGuiCompound.hpp"

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
    
    virtual ProcessingResult processEvent(const XEvent *event);
    virtual ProcessingResult processKeyboardEvent(const XEvent *event);

    virtual void treatNewWindowPosition(Position newPosition);
    virtual void treatFocusIn();
    virtual void treatFocusOut();
    virtual void setSize(int width, int height);
    virtual void show();
    
    void invokeGotoLinePanel();
    void invokeSaveAsPanel(Callback<>::Ptr saveCallback);
    void invokeFindPanelBackward();
    void invokeFindPanelForward();

    void invokeReplacePanelBackward();
    void invokeReplacePanelForward();

    virtual void requestCloseWindow();
    
    String getFileName() const;
    
    TextStyles::Ptr getTextStyles() {
        return textEditor->getTextStyles();
    }

    HilitedText::Ptr getHilitedText() {
        return textEditor->getHilitedText();
    }
    
    void setModalMessageBox(const MessageBoxParameter& messageBoxParameter);
    void closeModalMessageBox();

    void invokeMessageBox(MessageBoxParameter p);
    
    bool hasUnsavedData() const {
        return textData->getModifiedFlag();
    }

    void requestCloseWindowAndDiscardChanges();
    void saveAndClose();
    bool isClosing() const {
        return isClosingFlag;
    }
    
    bool checkForFileModifications();

private:
    EditorTopWin(TextStyles::Ptr textStyles, HilitedText::Ptr hilitedText, int width, int height);

    void treatConfigUpdate();

    void requestCloseFor(GuiWidget* w);

    void handleEscapeKey();
    void handleSaveKey();
    void handleSaveAsKey();
    void invokePanel(DialogPanel* panel);
    void createEmptyWindow();
    void createCloneWindow();
    void executeLuaScript();
    
    void requestProgramQuit();
    
    void handleNewFileName(const String& fileName);
    void handleChangedModifiedFlag(bool modifiedFlag);
    void handleChangedReadOnlyFlag(bool readOnlyFlag);
    
    void reloadFile();
    void doNotReloadFile();
    
    void setWindowTitle();
    
    void notifyRequestCloseChildWindow(TopWin *topWin);
    void invokeNewModalMessageBox();

    void executeTestScript();
    void finishedTestScript(ProgramExecutor::Result result);
    
    MultiLineEditorWidget::Ptr textEditor;
    RawPtr<TextData>         textData;
    StatusLine::Ptr statusLine;
    GuiLayoutColumn::Ptr rootElement;
    KeyMapping keyMapping1;
    KeyMapping keyMapping2;
    GotoLinePanel::Ptr gotoLinePanel;
    bool flagForSetSizeHintAtFirstShow;

    FindPanel::Ptr findPanel;
    ReplacePanel::Ptr replacePanel;
    int upperPanelIndex;
    int lowerPanelIndex;
    
    WeakPtr<DialogPanel> invokedPanel;
    int                  invokedPanelIndex;
    MessageBox::Ptr messageBox;

    MessageBox::Ptr     modalMessageBox;
    bool                hasModalMessageBox;
    MessageBoxParameter modalMessageBoxParameter;
    
    SaveAsPanel::Ptr    saveAsPanel;
    
    bool isClosingFlag;

    ScrollableTextGuiCompound::Ptr scrollableTextCompound;
};

} // namespace LucED

#endif // EDITOR_TOP_WIN_HPP
