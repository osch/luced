/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2010 Oliver Schmidt, oliver at luced dot de
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

#include "String.hpp"
#include "CharUtil.hpp"
#include "Utf8Parser.hpp"

using namespace LucED;


String String::toUpperUtf8() const
{
    String rslt;

    Utf8Parser<String> utf8Parser(this);

    const long endPos = getLength();
    long       pos    = 0;
    
    while (pos < endPos)
    {
        long lastPos = pos;
        
        int c = utf8Parser.getWCharAndIncrementPos(&pos);

        c = CharUtil::toUpper(c);

        if (c < 0) {
            rslt.appendSubstring(*this, Pos(lastPos), Len(pos - lastPos));
        } else {
            rslt.appendWCharAsUtf8(c);
        }
    }
    return rslt;
}


String String::toLowerUtf8() const
{
    String rslt;

    Utf8Parser<String> utf8Parser(this);

    const long endPos = getLength();
    long       pos    = 0;
    
    while (pos < endPos)
    {
        long lastPos = pos;
        
        int c = utf8Parser.getWCharAndIncrementPos(&pos);

        c = CharUtil::toLower(c);

        if (c < 0) {
            rslt.appendSubstring(*this, Pos(lastPos), Len(pos - lastPos));
        } else {
            rslt.appendWCharAsUtf8(c);
        }
    }
    return rslt;
}
