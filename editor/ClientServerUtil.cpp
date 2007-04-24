/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, oliver at luced dot de
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

#include "ClientServerUtil.h"

using namespace LucED;


GuiRootProperty ClientServerUtil::getDefaultServerRunningProperty()
{
    return GuiRootProperty("LUCED_SERVER_");
}


GuiRootProperty ClientServerUtil::getDefaultServerCommandProperty()
{
    return GuiRootProperty("LUCED_COMMAND_");
}

String ClientServerUtil::quoteCommandline(HeapObjectArray<String>::Ptr commandline)
{
    const int argc = commandline->getLength();
    String rslt;
    
    for (int i = 0; i < argc; ++i)
    {
        const String argument = commandline->get(i);
        
        for (int j = 0; j < argument.getLength(); ++j)
        {
            if (j == 0 && i > 0) {
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


HeapObjectArray<String>::Ptr ClientServerUtil::unquoteCommandline(const String& commandline)
{
    HeapObjectArray<String>::Ptr  rslt = HeapObjectArray<String>::create();
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


