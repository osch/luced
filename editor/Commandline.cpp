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

#include "Commandline.hpp"

using namespace LucED;

Commandline::Ptr Commandline::createFromQuotedString(const String& commandline)
{
    Commandline::Ptr rslt(new Commandline());
    
    const int len = commandline.getLength();
    
    int i = 0;
    
    while (i < len)
    {
        while (i < len && commandline[i] == ' ') {
            ++i;
        }
        if (i < len && commandline[i] == '"') {
            ++i;

            String s;
            
            while (i < len && commandline[i] != '"')
            {
                if (commandline[i] == '\\' && i + 1 < len) {
                    s << commandline[i + 1];
                    ++i;
                } else {
                    s << commandline[i];
                }
                ++i;
            }
            if (i < len && commandline[i] == '"') {
                ++i;
            }
            rslt->append(s);
        }
        else {
            String s;
            
            while (i < len && commandline[i] != ' ')
            {
                if (commandline[i] == '\\' && i + 1 < len) {
                    s << commandline[i + 1];
                    ++i;
                } else {
                    s << commandline[i];
                }
                ++i;
            }
            if (i < len && commandline[i] == ' ') {
                ++i;
            }
            rslt->append(s);
        }
    }
    return rslt;
}


String Commandline::toQuotedString() const
{
    const int argc = this->getLength();
    String rslt;
    int firstIndex = 0;
    
    for (int i = firstIndex; i < argc; ++i)
    {
        const String argument = this->get(i);
        
        if (i > firstIndex) {
            rslt << ' ';
        }
        rslt << '"';

        for (int j = 0; j < argument.getLength(); ++j)
        {
            switch (argument[j]) {
                case '"': {
                    rslt << "\\\"";  //    "   ->   \"
                    break;
                }
                case '\\': {
                    rslt << "\\\\";  //    \   ->   \\
                    break;
                }
                default: {
                    rslt << argument[j];
                    break;
                }
            }
        }
        rslt << '"';
    }
    return rslt;
}

