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

#include "String.hpp"

#include "EventDispatcher.hpp"
#include "SyntaxPatterns.hpp"
#include "LuaException.hpp"
#include "GlobalConfig.hpp"
#include "ConfigException.hpp"
#include "SingletonKeeper.hpp"
#include "EditorServer.hpp"
#include "HeapObjectArray.hpp"
#include "CommandlineException.hpp"
#include "FileException.hpp"

using namespace LucED;


int main(int argc, char **argv)
{
    int rc = 0;
    
    try
    {
        SingletonKeeper::Ptr singletonKeeper = SingletonKeeper::create();
        
        HeapObjectArray<String>::Ptr commandline = HeapObjectArray<String>::create();

        for (int argIndex = 1; argIndex < argc; ++argIndex)
        {
            commandline->append(String(argv[argIndex]));
        }

        try
        {
            GlobalConfig::getInstance()->readConfig();

            EditorServer::getInstance()->startWithCommandline(commandline);
        }
        catch (ConfigException& ex)
        {
            EditorServer::getInstance()->startWithCommandlineAndErrorList(commandline, ex.getErrorList());
        }

        EventDispatcher::getInstance()->doEventLoop();
    }
    catch (CommandlineException& ex)
    {
        fprintf(stderr, "[%s]: CommandlineException: %s\n", argv[0], ex.getMessage().toCString());
        rc = 1;
    }
    catch (BaseException& ex)
    {
        fprintf(stderr, "[%s]: Severe Error: %s\n",    argv[0], ex.getMessage().toCString());
        rc = 16;
    }

#ifdef DEBUG
    HeapObjectChecker::assertAllCleared();
#endif
    return rc;
}
