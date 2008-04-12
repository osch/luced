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

#ifndef BUILTIN_TOP_WIN_ACTIONS_HPP
#define BUILTIN_TOP_WIN_ACTIONS_HPP

#include "TopWinActions.hpp"
#include "HeapHashMap.hpp"

namespace LucED
{

class BuiltinTopWinActions : public TopWinActions
{
public:
    typedef OwningPtr<BuiltinTopWinActions> Ptr;
    typedef TopWinActions::Handler          Handler;
    
    static Ptr create() {
        return Ptr(new BuiltinTopWinActions());
    }
    
    virtual Handler::Ptr createNewHandler(const TopWinActions::Parameter& parameter);

private:
    BuiltinTopWinActions();

    class HandlerImpl;
    typedef void (HandlerImpl::*MethodPtr)();

    typedef HeapHashMap< String, MethodPtr > MethodMap;
    
    MethodMap::Ptr methodMap;
};

} // namespace LucED

#endif // BUILTIN_TOP_WIN_ACTIONS_HPP
