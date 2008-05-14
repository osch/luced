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

typedef TopWinActions::Binding Binding;


class BuiltinTopWinActions::BindingImpl : public  Binding, 
                                          private TopWinActions::Parameter
{
public:
    
    void invokeGotoLinePanel()
    {
        if (gotoLinePanel.isInvalid()) {
            gotoLinePanel = GotoLinePanel::create(parentWidget, editorWidget, panelInvoker);
        }
        panelInvoker->invokePanel(gotoLinePanel);
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
        if (replacePanel->isVisible()) {
            replacePanel->findAgainForward();
        } else {
            findPanel->findAgainForward();
        }
    }
    
    void findAgainBackward()
    {
        if (replacePanel->isVisible()) {
            replacePanel->findAgainBackward();
        } else {
            findPanel->findAgainBackward();
        }
    }

    void createCloneWindow()
    {
    
    }
    
    void requestProgramTermination()
    {
        WindowCloser::start();
    }

    void handleEscapeKey()
    {
        if (panelInvoker->hasInvokedPanel()) {
            panelInvoker->closeInvokedPanel();
        }
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
    
    virtual Callback<>::Ptr getCallback(const String& methodName)
    {
        Callback<>::Ptr rslt;
        
        CallbackMap::Value foundValue = callbackMap.get(methodName);
        if (foundValue.isValid()) {
            rslt = foundValue.get();
        } else {
            MethodMap::Value foundMethod = methodMap->get(methodName);
            if (foundMethod.isValid()) {
                rslt = newCallback(this, foundMethod.get());
                callbackMap.set(methodName, rslt);
            }
        }
        return rslt;
    }

private:
    friend class BuiltinTopWinActions;
    
    typedef OwningPtr<BindingImpl> Ptr;
    
    BindingImpl(MethodMap::Ptr methodMap, 
                const TopWinActions::Parameter& parameter)
        : TopWinActions::Parameter(parameter),
          methodMap(methodMap)
    {
        findPanel = FindPanel::create(parentWidget, editorWidget, 
                                      messageBoxInvoker,
                                      panelInvoker);

        replacePanel = ReplacePanel::create(parentWidget, editorWidget, findPanel,
                                            messageBoxInvoker,
                                            panelInvoker);
    }

    MethodMap::Ptr                           methodMap;

    typedef HashMap<String, Callback<>::Ptr> CallbackMap;
    CallbackMap                              callbackMap;

    
    FindPanel::Ptr                           findPanel;
    ReplacePanel::Ptr                        replacePanel;
    GotoLinePanel::Ptr                       gotoLinePanel;
};


Binding::Ptr BuiltinTopWinActions::createNewBinding(const TopWinActions::Parameter& parameter)
{
    BindingImpl::Ptr rslt(new BindingImpl(methodMap, parameter));
    return rslt;
}


BuiltinTopWinActions::BuiltinTopWinActions()
{
    methodMap = MethodMap::create();
    
    methodMap->set("invokeGotoLinePanel",        &BindingImpl::invokeGotoLinePanel);

    methodMap->set("invokeFindPanelForward",     &BindingImpl::invokeFindPanelForward);
    methodMap->set("invokeFindPanelBackward",    &BindingImpl::invokeFindPanelBackward);

    methodMap->set("invokeReplacePanelForward",  &BindingImpl::invokeReplacePanelForward);
    methodMap->set("invokeReplacePanelBackward", &BindingImpl::invokeReplacePanelBackward);

    methodMap->set("findSelectionForward",       &BindingImpl::findSelectionForward);
    methodMap->set("findSelectionBackward",      &BindingImpl::findSelectionBackward);

    methodMap->set("replaceAgainForward",        &BindingImpl::replaceAgainForward);
    methodMap->set("replaceAgainBackward",       &BindingImpl::replaceAgainBackward);

    methodMap->set("findAgainForward",           &BindingImpl::findAgainForward);
    methodMap->set("findAgainBackward",          &BindingImpl::findAgainBackward);
    
    methodMap->set("requestProgramTermination",  &BindingImpl::requestProgramTermination);

    methodMap->set("handleEscapeKey",            &BindingImpl::handleEscapeKey);
}

