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

#ifndef TOP_WIN_ACTIONS_HPP
#define TOP_WIN_ACTIONS_HPP

#include "HeapObject.hpp"
#include "OwningPtr.hpp"
#include "RawPtr.hpp"
#include "Callback.hpp"
#include "GuiWidget.hpp"
#include "TextEditorWidget.hpp"
#include "MessageBoxParameter.hpp"
#include "DialogPanel.hpp"
#include "HeapHashMap.hpp"

namespace LucED
{

class TopWinActions : public HeapObject
{
public:
    typedef OwningPtr<TopWinActions> Ptr;

    class Parameter
    {
    public:
    
        Parameter(RawPtr<GuiWidget>                parentWidget, 
                  RawPtr<TextEditorWidget>         editorWidget, 
                  Callback<MessageBoxParameter>::Ptr messageBoxInvoker,
                  Callback<DialogPanel*>::Ptr        panelInvoker,
                  Callback<GuiWidget*>::Ptr          requestCloseCallback)
            : parentWidget(parentWidget),
              editorWidget(editorWidget),
              messageBoxInvoker(messageBoxInvoker),
              panelInvoker(panelInvoker),
              requestCloseCallback(requestCloseCallback)
        {}

    protected:
        RawPtr<GuiWidget>                parentWidget;
        RawPtr<TextEditorWidget>         editorWidget;
        Callback<MessageBoxParameter>::Ptr messageBoxInvoker;
        Callback<DialogPanel*>::Ptr        panelInvoker;
        Callback<GuiWidget*>::Ptr          requestCloseCallback;
    };

    class Handler : public HeapObject
    {
    public:
        typedef OwningPtr<Handler> Ptr;

        virtual bool execute(const String& methodName) = 0;

    protected:
        Handler()
        {}
    };
    
    typedef HeapHashMap< String, TopWinActions::Handler::Ptr > HandlerMap;
    typedef HeapHashMap< String, TopWinActions::Ptr >          ActionsMap;

    virtual Handler::Ptr createNewHandler(const Parameter& parameter) = 0;

protected:
    TopWinActions()
    {}
};

} // namespace LucED

#endif // TOP_WIN_ACTIONS_HPP
