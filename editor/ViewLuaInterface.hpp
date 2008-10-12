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

#ifndef VIEW_LUA_INTERFACE_HPP
#define VIEW_LUA_INTERFACE_HPP

#include "HeapObject.hpp"
#include "OwningPtr.hpp"
#include "RawPtr.hpp"
#include "TextEditorWidget.hpp"
#include "LuaCFunctionResult.hpp"
#include "LuaCFunctionArguments.hpp"

namespace LucED
{

class ViewLuaInterface : public HeapObject
{
public:
    typedef OwningPtr<ViewLuaInterface> Ptr;
    
    static Ptr create(RawPtr<TextEditorWidget> e) {
        return Ptr(new ViewLuaInterface(e));
    }


    LuaCFunctionResult getFileName(const LuaCFunctionArguments& args)
    {
        LuaAccess luaAccess = args.getLuaAccess();
        
        return LuaCFunctionResult(luaAccess) << e->getTextData()->getFileName();
    }
    

    LuaCFunctionResult getCursorPosition(const LuaCFunctionArguments& args)
    {
        LuaAccess luaAccess = args.getLuaAccess();
        
        return LuaCFunctionResult(luaAccess) << e->getCursorTextPosition();
    }
    

    LuaCFunctionResult getCursorLine(const LuaCFunctionArguments& args)
    {
        LuaAccess luaAccess = args.getLuaAccess();
        
        return LuaCFunctionResult(luaAccess) << e->getCursorLineNumber();
    }
    

    LuaCFunctionResult getCursorColumn(const LuaCFunctionArguments& args)
    {
        LuaAccess luaAccess = args.getLuaAccess();
        
        return LuaCFunctionResult(luaAccess) << e->getCursorColumn();
    }
    
    LuaCFunctionResult insertAtCursor(const LuaCFunctionArguments& args)
    {
        LuaAccess luaAccess = args.getLuaAccess();
        
        TextData::TextMark m = e->createNewMarkFromCursor();
        
        for (int i = 1; i < args.getLength(); ++i)
        {
            if (args[i].isString()) {
                long insertedLength = e->getTextData()->insertAtMark(m, (const byte*)
                                                                        args[i].getStringPtr(),
                                                                        args[i].getStringLength());
                m.moveForwardToPos(m.getPos() + insertedLength);
            }
            else if (args[i].isTable()) {
            
            }
        } 
        
        e->moveCursorToTextMark(m);
        
        return LuaCFunctionResult(luaAccess);
    }
    
    LuaCFunctionResult getCharAtCursor(const LuaCFunctionArguments& args)
    {
        LuaAccess luaAccess = args.getLuaAccess();
        
        long pos = e->getCursorTextPosition();
        LuaVar rslt(luaAccess);
        
        if (pos < e->getTextData()->getLength())
        {
            rslt.assign(luaAccess.toLua((const char*)(e->getTextData()->getAmount(pos, 1)),
                                        1));
        }
        else {
            rslt.assign("");
        }
        return LuaCFunctionResult(luaAccess) << rslt;
    }
    
    LuaCFunctionResult getCharsAtCursor(const LuaCFunctionArguments& args)
    {
        LuaAccess luaAccess = args.getLuaAccess();
        
        long pos    = e->getCursorTextPosition();
        long length = e->getTextData()->getLength();
        long amount = 1;
        
        if (args.getLength() >= 2 && args[1].isNumber()) {
            amount = args[1].toLong();
        }
        if (pos + amount > length) {
            amount = length - pos;
        }
        
        LuaVar rslt(luaAccess);
        
        if (amount > 0)
        {
            rslt.assign(luaAccess.toLua((const char*)(e->getTextData()->getAmount(pos, amount)),
                                        amount));
        }
        else {
            rslt.assign("");
        }
        return LuaCFunctionResult(luaAccess) << rslt;
    }
    

private:
    ViewLuaInterface(RawPtr<TextEditorWidget> e)
        : e(e)
    {}

    RawPtr<TextEditorWidget> e;
};

} // namespace LucED

#endif // VIEW_LUA_INTERFACE_HPP
