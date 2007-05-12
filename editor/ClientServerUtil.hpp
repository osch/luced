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

#ifndef CLIENTSERVERUTIL_H
#define CLIENTSERVERUTIL_H

#include "String.hpp"

#include "GuiRootProperty.hpp"
#include "HeapObjectArray.hpp"

namespace LucED
{



class ClientServerUtil
{
public:
    static GuiRootProperty getDefaultServerRunningProperty();
    
    static GuiRootProperty getDefaultServerCommandProperty();
    
    static String quoteCommandline(HeapObjectArray<String>::Ptr commandline);

    static HeapObjectArray<String>::Ptr unquoteCommandline(const String& commandline);
};

} // namespace LucED

#endif // CLIENTSERVERUTIL_H

