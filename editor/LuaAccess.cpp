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

#include "LuaAccess.hpp"
#include "LuaStateAccess.hpp"
#include "LuaAccess.hpp"
#include "ByteBuffer.hpp"
#include "File.hpp"
#include "LuaVar.hpp"
#include "Regex.hpp"
#include "SystemException.hpp"
#include "LuaException.hpp"
#include "FileException.hpp"

using namespace LucED;


// prepend fileName with "@" for external file, with "$" for builtin file
void LuaAccess::internLoadBuffer(const char*      scriptBegin, 
                                 long             scriptLength, 
                                 Nullable<String> fileName) const
{
    String      chunkName;
    const char* chunkNamePtr = NULL;
    if (fileName.isValid()) {
        chunkName << fileName.get();
        chunkNamePtr = chunkName.toCString();
    }
    
    int parsingRc = luaL_loadbuffer(L, scriptBegin, scriptLength, chunkNamePtr);
    
    if (parsingRc != 0) {
        if (parsingRc == LUA_ERRSYNTAX) {
            LuaVar errorObject(*this, lua_gettop(L));
            if (fileName.isValid()) {
                throw LuaException(ExceptionLuaInterface::createParsingFileError(errorObject, fileName.get()));
            } else {
                throw LuaException(ExceptionLuaInterface::createParsingScriptError(errorObject, scriptBegin, scriptLength));
            }
        }
        else if (parsingRc == LUA_ERRMEM) {
            throw LuaException(*this, "out of memory");
        }
        else {
            throw SystemException("unknown error while parsing lua script");
        }
    }
}

LuaAccess::Result LuaAccess::executeScript(const char* scriptBegin, long scriptLength) const
{
    ASSERT(isCorrect());

    RawPtr<LuaInterpreter> luaInterpreter = getLuaInterpreter();
    

    internLoadBuffer(scriptBegin, scriptLength);

    String oldPrintBuffer = LuaInterpreter::PrintBufferAccess::getPrintBuffer(luaInterpreter);
                            LuaInterpreter::PrintBufferAccess::setPrintBuffer(luaInterpreter, String());

    int oldTop = lua_gettop(L);

    int error = lua_pcall(L, 0, LUA_MULTRET, luaInterpreter->getErrorHandlerStackIndex());

    String printBuffer = LuaInterpreter::PrintBufferAccess::getPrintBuffer(luaInterpreter);
                         LuaInterpreter::PrintBufferAccess::setPrintBuffer(luaInterpreter, oldPrintBuffer);

    if (error) {
        LuaVar errorObject(*this, lua_gettop(L));
        throw LuaException(errorObject);
    }
    Result rslt;
           rslt.output = printBuffer;
    for (int i = oldTop, n = lua_gettop(L); i <= n; ++i) {
        rslt.objects.appendObjectWithStackIndex(*this, i);
    }

    return rslt;
}


LuaVar LuaAccess::loadString(const char* script, const Nullable<String>& fileName) const
{
    internLoadBuffer(script, strlen(script), fileName);
    
    return LuaVar(*this, lua_gettop(L));
}



LuaVar LuaAccess::loadFile(const String& fileName) const
{
    ByteBuffer buffer;
    try
    {
        File(fileName).loadInto(&buffer);
    }
    catch (FileException& ex) {
        return LuaVar(*this, Null);
    }
    
    internLoadBuffer((const char*)buffer.getPtr(), buffer.getLength(), String() << "@" << fileName);

    return LuaVar(*this, lua_gettop(L));
}

