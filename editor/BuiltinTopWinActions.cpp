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

#include "BuiltinTopWinActions.hpp"
#include "FindPanel.hpp"
#include "ReplacePanel.hpp"
#include "GotoLinePanel.hpp"
#include "WindowCloser.hpp"

using namespace LucED;

typedef TopWinActions::Handler Handler;


class BuiltinTopWinActions::HandlerImpl : public  Handler, 
                                          private TopWinActions::Parameter
{
public:
    
    void invokeGotoLinePanel()
    {
        if (gotoLinePanel.isInvalid()) {
            gotoLinePanel = GotoLinePanel::create(parentWidget, editorWidget, requestCloseCallback);
        }
        panelInvoker->call(gotoLinePanel);
    }
    
    void invokeFindPanelForward()
    {
        ASSERT(findPanel.isValid());
        findPanel->setDefaultDirection(Direction::DOWN);
        panelInvoker->call(findPanel);
    }
    
    void invokeFindPanelBackward()
    {
        ASSERT(findPanel.isValid());
        findPanel->setDefaultDirection(Direction::UP);
        panelInvoker->call(findPanel);
    }
    
    void invokeReplacePanelForward()
    {
        ASSERT(replacePanel.isValid());
        replacePanel->setDefaultDirection(Direction::DOWN);
        panelInvoker->call(replacePanel);
    }
    
    void invokeReplacePanelBackward()
    {
        ASSERT(replacePanel.isValid());
        replacePanel->setDefaultDirection(Direction::UP);
        panelInvoker->call(replacePanel);
    }
    
    void findSelectionForward()
    {
        findPanel->findSelectionForward();
    }
    
    void findSelectionBackward()
    {
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
        findPanel->findAgainForward();
    }
    
    void findAgainBackward()
    {
        findPanel->findAgainBackward();
    }

    void createCloneWindow()
    {
    
    }
    
    void requestProgramQuit()
    {
        WindowCloser::start();
    }
    
    virtual bool execute(const String& methodName)
    {
        MethodMap::Value foundMethod = methodMap->get(methodName);
        if (foundMethod.isValid())
        {
            MethodPtr methodPtr = foundMethod.get();
            
            (this->*methodPtr)(); // invoke Method
            return true;
        }
        else {
            return false;
        }
    }

private:
    friend class BuiltinTopWinActions;
    
    typedef OwningPtr<HandlerImpl> Ptr;
    
    HandlerImpl(MethodMap::Ptr methodMap, 
                const TopWinActions::Parameter& parameter)
        : TopWinActions::Parameter(parameter),
          methodMap(methodMap)
    {
        findPanel = FindPanel::create(parentWidget, editorWidget, 
                                      messageBoxInvoker,
                                      panelInvoker,
                                      requestCloseCallback);

        replacePanel = ReplacePanel::create(parentWidget, editorWidget, findPanel,
                                            messageBoxInvoker,
                                            panelInvoker,
                                            requestCloseCallback);
    }

    MethodMap::Ptr                     methodMap;

    FindPanel::Ptr                     findPanel;
    ReplacePanel::Ptr                  replacePanel;
    GotoLinePanel::Ptr                 gotoLinePanel;
};


Handler::Ptr BuiltinTopWinActions::createNewHandler(const TopWinActions::Parameter& parameter)
{
    HandlerImpl::Ptr rslt(new HandlerImpl(methodMap, parameter));
    return rslt;
}


BuiltinTopWinActions::BuiltinTopWinActions()
{
    methodMap = MethodMap::create();
    
    methodMap->set("invokeGotoLinePanel",        &HandlerImpl::invokeGotoLinePanel);

    methodMap->set("invokeFindPanelForward",     &HandlerImpl::invokeFindPanelForward);
    methodMap->set("invokeFindPanelBackward",    &HandlerImpl::invokeFindPanelBackward);

    methodMap->set("invokeReplacePanelForward",  &HandlerImpl::invokeReplacePanelForward);
    methodMap->set("invokeReplacePanelBackward", &HandlerImpl::invokeReplacePanelBackward);

    methodMap->set("findSelectionForward",       &HandlerImpl::findSelectionForward);
    methodMap->set("findSelectionBackward",      &HandlerImpl::findSelectionBackward);

    methodMap->set("replaceAgainForward",        &HandlerImpl::replaceAgainForward);
    methodMap->set("replaceAgainBackward",       &HandlerImpl::replaceAgainBackward);

    methodMap->set("findAgainForward",           &HandlerImpl::findAgainForward);
    methodMap->set("findAgainBackward",          &HandlerImpl::findAgainBackward);
    
    methodMap->set("requestProgramQuit",         &HandlerImpl::requestProgramQuit);
}

