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

#include "debug.hpp"
#include "LuaStackTrace.hpp"

using namespace LucED;

// prepend fileName with "@" for external file, with "$" for builtin file
LuaStackTrace::Entry::Ptr LuaStackTrace::Entry::createFileEntry(const String& fileName, int lineNumber)
{
    bool builtin;
    if (fileName.startsWith("$")) {
        builtin = true;
    } else {
        ASSERT(fileName.startsWith("@"));
        builtin = false;
    }
    return Ptr(new Entry(fileName.getTail(1), builtin, lineNumber));
}


void LuaStackTrace::fillStackTrace(LuaAccess luaAccess,
                                   int       callLevel)
{
nextEntry:
    lua_Debug debugInfo;   memset(&debugInfo, 0, sizeof(debugInfo));
    
    int rc = lua_getstack(luaAccess.L, callLevel, &debugInfo);
    if (rc == 1)
    {
        lua_getinfo (luaAccess.L, "Sln", &debugInfo);

        if (   strcmp(debugInfo.what, "C") != 0
            && debugInfo.source != NULL)
        {
            Entry::Ptr e;
            if (debugInfo.source[0] == '@' || debugInfo.source[0] == '$') {
                e = Entry::createFileEntry(debugInfo.source,
                                           debugInfo.currentline - 1);
            } else if (debugInfo.source[0] != '=') {
                e = Entry::createScriptBytesEntry((const byte*)debugInfo.source,
                                                               debugInfo.sourceLen,
                                                  debugInfo.currentline - 1);
            }
            if (e.isValid()) {
                appendEntry(e);
            }
        }
        callLevel += 1;
        goto nextEntry;
    }
}

String LuaStackTrace::Entry::toString() const
{
    String rslt;
    if (hasFileName()) {
        if (isBuiltinFile()) {
            rslt << "builtin  file '" << getFileName() << "'";
        } else {
            rslt << "file '" << getFileName() << "'";
        }
    } else {
        rslt << "temporary script bytes";
    }
    if (lineNumber >= 0) {
        rslt << ", line " << (lineNumber + 1);
    }
    return rslt;
}

String LuaStackTrace::toString() const
{
    String rslt;
    for (int i = 0, j = getEntryCount(); i < j; ++i) {
        rslt << getEntry(i)->toString() << "\n";
    }
    return rslt;
}


LuaStackTrace::Entry::Ptr LuaStackTrace::findFirstExternalFileEntry() const 
{
    for (int i = 0, j = getEntryCount(); i < j; ++i) {
        if (    entries[i]->hasFileName()
            && !entries[i]->isBuiltinFile())
        {
            return entries[i];
        }
    }
    return Null;
}

LuaStackTrace::Entry::Ptr LuaStackTrace::findFirstScriptBytesEntry() const
{
    for (int i = 0, j = getEntryCount(); i < j; ++i) {
        if (entries[i]->hasScriptBytes())
        {
            return entries[i];
        }
    }
    return Null;
}    

LuaStackTrace::Entry::Ptr LuaStackTrace::findFirstBuiltinFileEntry() const
{
    for (int i = 0, j = getEntryCount(); i < j; ++i) {
        if (   entries[i]->hasFileName()
            && entries[i]->isBuiltinFile())
        {
            return entries[i];
        }
    }
    return Null;
}

LuaStackTrace::Entry::Ptr LuaStackTrace::findFirstFileEntry() const
{
    for (int i = 0, j = getEntryCount(); i < j; ++i) {
        if (entries[i]->hasFileName())
        {
            return entries[i];
        }
    }
    return Null;
}
