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

#ifndef EDITOR_TOP_WIN_ACTIONS_HPP
#define EDITOR_TOP_WIN_ACTIONS_HPP

#include "FindPanel.hpp"
#include "ReplacePanel.hpp"
#include "GotoLinePanel.hpp"
#include "ExecutePanel.hpp"
#include "WindowCloser.hpp"
#include "ActionMethodBinding.hpp"
#include "TopWinActionsParameter.hpp"

namespace LucED
{

class EditorTopWinActions : public  ActionMethodBinding<EditorTopWinActions>,
                            private TopWinActionsParameter
{
public:
    typedef OwningPtr<EditorTopWinActions> Ptr;
    
    static Ptr create(const TopWinActionsParameter& parameter) {
        return Ptr(new EditorTopWinActions(parameter));
    }

    void invokeGotoLinePanel()
    {
        if (gotoLinePanel.isInvalid()) {
            gotoLinePanel = GotoLinePanel::create(editorWidget,
                                                  newCallback(this, &EditorTopWinActions::closeGotoLinePanel));
        }
        panelInvoker->invokePanel(gotoLinePanel);
    }
    
    void invokeExecutePanel()
    {
        if (executePanel.isInvalid()) {
            executePanel = ExecutePanel::create(editorWidget,
                                                newCallback(this, &EditorTopWinActions::closeExecutePanel));
        }
        panelInvoker->invokePanel(executePanel);
    }
    
    void invokeFindPanelForward()
    {
        ASSERT(findPanel.isValid());
        findPanel->setDefaultDirection(Direction::DOWN);
        panelInvoker->invokePanel(findPanel);
    }
    
    void invokeFindPanelBackward()
    {
        ASSERT(findPanel.isValid());
        findPanel->setDefaultDirection(Direction::UP);
        panelInvoker->invokePanel(findPanel);
    }
    
    void invokeReplacePanelForward()
    {
        ASSERT(replacePanel.isValid());
        replacePanel->setDefaultDirection(Direction::DOWN);
        panelInvoker->invokePanel(replacePanel);
    }
    
    void invokeReplacePanelBackward()
    {
        ASSERT(replacePanel.isValid());
        replacePanel->setDefaultDirection(Direction::UP);
        panelInvoker->invokePanel(replacePanel);
    }
    
    void findSelectionForward()
    {
        panelInvoker->closeInvokedPanel();
        findPanel->findSelectionForward();
    }
    
    void findSelectionBackward()
    {
        panelInvoker->closeInvokedPanel();
        findPanel->findSelectionBackward();
    }
    
    void replaceAgainForward()
    {
        replacePanel->replaceAgainForward();
    }
    
    void replaceAgainBackward()
    {
        replacePanel->replaceAgainBackward();
    }
    
    void findAgainForward()
    {
        if (replacePanel->isMapped()) {
            replacePanel->findAgainForward();
        } else {
            findPanel->findAgainForward();
        }
    }
    
    void findAgainBackward()
    {
        if (replacePanel->isMapped()) {
            replacePanel->findAgainBackward();
        } else {
            findPanel->findAgainBackward();
        }
    }

    void createCloneWindow()
    {
        topWinActionInterface->createCloneWindow();
    }
    
    void createEmptyWindow()
    {
        topWinActionInterface->createEmptyWindow();
    }
    
    void requestProgramTermination()
    {
        WindowCloser::start();
    }

    void closePanel()
    {
        if (panelInvoker->hasInvokedPanel()) {
            panelInvoker->closeInvokedPanel();
        }
    }
    
    void requestCloseWindow()
    {
        topWinActionInterface->requestCloseWindowByUser();
    }

    void handleSaveKey()
    {
        topWinActionInterface->handleSaveKey();
    }
    
    void handleSaveAsKey()
    {
        topWinActionInterface->handleSaveAsKey();
    }
    
    void executeLuaScript();
    
    void resetLuaModules();
    
    void mainViewUndo();
    void mainViewRedo();
        
private:
 
    EditorTopWinActions(const TopWinActionsParameter& parameter)
 
        : ActionMethodBinding<EditorTopWinActions>(this),
          TopWinActionsParameter(parameter)
    {
        findPanel = FindPanel::create(editorWidget, 
                                      messageBoxInvoker,
                                      newCallback(this, &EditorTopWinActions::invokeFindPanel),
                                      newCallback(this, &EditorTopWinActions::closeFindPanel));

        replacePanel = ReplacePanel::create(editorWidget, findPanel,
                                            messageBoxInvoker,
                                            newCallback(this, &EditorTopWinActions::invokeReplacePanel),
                                            newCallback(this, &EditorTopWinActions::closeReplacePanel));
    }
    
    void invokeFindPanel() {
        panelInvoker->invokePanel(findPanel);
    }
    void closeFindPanel() {
        panelInvoker->getCloseCallback()->call(findPanel);
    }
    void invokeReplacePanel() {
        panelInvoker->invokePanel(replacePanel);
    }
    void closeReplacePanel() {
        panelInvoker->getCloseCallback()->call(replacePanel);
    }
    void closeGotoLinePanel() {
        panelInvoker->getCloseCallback()->call(gotoLinePanel);
    }
    void closeExecutePanel() {
        panelInvoker->getCloseCallback()->call(executePanel);
    }

    FindPanel::Ptr                           findPanel;
    ReplacePanel::Ptr                        replacePanel;
    GotoLinePanel::Ptr                       gotoLinePanel;
    ExecutePanel::Ptr                        executePanel;
};


} // namespace LucED

#endif // EDITOR_TOP_WIN_ACTIONS_HPP
