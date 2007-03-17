/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2007 Oliver Schmidt, oliver at luced dot de
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

#include <string>

#include "EventDispatcher.h"
#include "SingletonKeeper.h"
#include "GuiRootProperty.h"
#include "EditorClient.h"
#include "CommandlineException.h"
#include "HeapObjectArray.h"

using namespace LucED;

using std::string;

int main(int argc, char **argv)
{
    int rc = 0;
    
    try
    {
        SingletonKeeper::Ptr singletonKeeper = SingletonKeeper::create();
        
        HeapObjectArray<string>::Ptr commandline = HeapObjectArray<string>::create();
        for (int argIndex = 1; argIndex < argc; ++argIndex)
        {
            commandline->append(string(argv[argIndex]));
        }

        EditorClient::getInstance()->startWithCommandline(commandline);
        
        EventDispatcher::getInstance()->doEventLoop();
        
    }
    catch (CommandlineException& ex)
    {
        fprintf(stderr, "CommandlineException: %s\n", ex.getMessage().c_str());
        rc = 1;
    }
#ifdef DEBUG
    HeapObjectChecker::assertAllCleared();
#endif
    return rc;
}
