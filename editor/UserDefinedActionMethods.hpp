/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2012 Oliver Schmidt, oliver at luced dot de
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

#ifndef USER_DEFINED_ACTION_METHODS_HPP
#define USER_DEFINED_ACTION_METHODS_HPP

#include "ActionMethods.hpp"
#include "TextEditorWidget.hpp"
#include "ProgramExecutor.hpp"
#include "WeakPtr.hpp"
                    
namespace LucED
{

class UserDefinedActionMethods : public ActionMethods
{
public:
    typedef OwningPtr<UserDefinedActionMethods> Ptr;

    class ShellInvocationHandler : public HeapObject
    {
    public:
        typedef OwningPtr<ShellInvocationHandler> Ptr;
        
        virtual void beforeShellInvocation() = 0;
        virtual void afterShellInvocation(ProgramExecutor::Result rslt) = 0;
        
    protected:
        ShellInvocationHandler()
        {}
    };

    static Ptr create(WeakPtr<TextEditorWidget>   textEditor,
                      ShellInvocationHandler::Ptr shellInvocationHandler = Null)
                      
    {
        return Ptr(new UserDefinedActionMethods(textEditor,
                                                shellInvocationHandler));
                                                
    }

    virtual bool invokeActionMethod(ActionId actionId);

    virtual bool hasActionMethod(ActionId actionId);
    
private:
    UserDefinedActionMethods(WeakPtr<TextEditorWidget>   textEditor,
                             ShellInvocationHandler::Ptr shellInvocationHandler)
        : textEditor(textEditor),
          shellInvocationHandler(shellInvocationHandler)
    {}
    
    LuaVar getLuaActionFunction(ActionId actionId);

    WeakPtr<TextEditorWidget>   textEditor;
    ShellInvocationHandler::Ptr shellInvocationHandler;
};

} // namespace LucED

#endif // USER_DEFINED_ACTION_METHODS_HPP
