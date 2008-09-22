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
    String s;
    
    for (int i = 0; i < commandline.getLength(); ++i)
    {
        if (commandline[i] == ' ' && s.getLength() > 0) {
            rslt->append(s);
            s = "";
        }
        else {
            if (commandline[i] == '\\' && i + 1 < commandline.getLength()) {
                s << commandline[i + 1];
                i += 1;
            } else {
                s << commandline[i];
            }
        }
    }
    if (s.getLength() > 0)
    {
        rslt->append(s);
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
        
        for (int j = 0; j < argument.getLength(); ++j)
        {
            if (j == 0 && i > firstIndex) {
                rslt << ' ';
            }
            switch (argument[j]) {
                case ' ': {
                    rslt << "\\ ";
                    break;
                }
                case '\\': {
                    rslt << "\\\\";
                    break;
                }
                default: {
                    rslt << argument[j];
                    break;
                }
            }
        }
    }
    return rslt;
}

