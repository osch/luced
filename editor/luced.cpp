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

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <locale.h>
#include <sys/types.h>

#include "String.hpp"
#include "EventDispatcher.hpp"
#include "SingletonKeeper.hpp"
#include "GuiRootProperty.hpp"
#include "EditorClient.hpp"
#include "EditorServer.hpp"
#include "Commandline.hpp"
#include "CommandlineException.hpp"
#include "ProgramName.hpp"
#include "DefaultConfig.hpp"
             
using namespace LucED;

int main(int argc, char** argv)
{
    setlocale(LC_CTYPE, "");

    int rc = 0;
    
    try
    {
        ProgramName::set(argv[0]);
        
        bool isServerStartupNeeded = false;
        {
            SingletonKeeper::Ptr singletonKeeper = SingletonKeeper::create();
            Commandline::Ptr     commandline     = Commandline::create(argc, argv);
            EditorClient::Ptr    editorClient    = EditorClient::getInstance();
          
            DefaultConfig::createMissingConfigFiles();
              
            editorClient->startWithCommandline(commandline);

            EventDispatcher::getInstance()->doEventLoop();
            
            isServerStartupNeeded = editorClient->isServerStartupNeeded();
        }
        #ifdef DEBUG
            HeapObjectChecker::assertAllCleared();
        #endif
        
        if (isServerStartupNeeded) // start new server
        {
            pid_t pid = fork();
            
            if (pid == 0) // we are child process: the new server
            {
                setlocale(LC_CTYPE, "");
                
                SingletonKeeper::Ptr singletonKeeper = SingletonKeeper::create();
                Commandline::Ptr     commandline     = Commandline::create(argc, argv);
                EditorServer::Ptr    editorServer    = EditorServer::getInstance();

                try
                {
                    editorServer->startWithCommandline(commandline);
                }
                catch (ConfigException& ex)
                {
                    editorServer->startWithCommandlineAndErrorList(commandline, ex.getErrorList());
                }

                EventDispatcher::getInstance()->doEventLoop();
            }
            else if (pid < 0)
            {
                fprintf(stderr, "[%s]: Could not fork process: %s\n", argv[0], strerror(errno));
                rc = 32;
            }
            #ifdef DEBUG
                HeapObjectChecker::assertAllCleared();
            #endif
        }
    }
    catch (CommandlineException& ex)
    {
        fprintf(stderr, "[%s]: Commandline Error: %s\n", argv[0], ex.getMessage().toCString());
        rc = 1;
    }
    catch (BaseException& ex)
    {
        fprintf(stderr, "[%s]: %s: %s\n", argv[0], ex.what(), ex.getMessage().toCString());
        rc = 16;
    }

    #ifdef DEBUG
        HeapObjectChecker::assertAllCleared();
    #endif

    return rc;
}
