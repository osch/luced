/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2009 Oliver Schmidt, oliver at luced dot de
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

#include "LuaSerializer.hpp"
#include "LuaIterator.hpp"
#include "LuaException.hpp"
          
using namespace LucED;



void LuaSerializer::serialize(const LuaVar& c)
{
    if (c.isTable())
    {
        const void* p = c.toVoidPtr();
        if (tables.findFirstIndex(p) >= 0) {
            throw LuaException(c.getLuaAccess(), String() << "object serialization impossible");
        }
        tables.append(p);
        
        buffer << '{';
        for (LuaIterator i(luaAccess); i.in(c);)
        {
            buffer << '[';    serializeValue(i.key());
            buffer << "]=";
            
            serialize(i.value());
            
            buffer << ',';
        }
        buffer << '}';

        tables.removeLast();
    }
    else {
        serializeValue(c);
    }
}

void LuaSerializer::serializeValue(const LuaVar& c)
{
    if (c.isNumber()) {
        buffer << c.toNumber();
    }
    else if (c.isString()) {
        buffer << '"';
        String s = c.toString();
        for (int i = 0, n = s.getLength(); i < n; ++i) {
            char c = s[i];
            if (c == '"') {
                buffer << "\\\"";
            }
            else if (c == '\\') {
                buffer << "\\\\";
            }
            else if (c == '\n') {
                buffer << "\\n";
            }
            else if (c == '\r') {
                buffer << "\\r";
            }
            else {
                buffer << c;
            }
        }
        buffer << '"';
    }
    else {
        throw LuaException(c.getLuaAccess(), String() << "object serialization impossible");
    }
}
