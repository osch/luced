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

#include "config.h"

#if HAVE_WINDOWS_H
#   include <windows.h>
#endif
#if HAVE_SYS_CYGWIN_H
#   include <sys/cygwin.h>
#endif

#include "String.hpp"
#include "ByteBuffer.hpp"
#include "EventDispatcher.hpp"
#include "SingletonKeeper.hpp"
#include "GuiRootProperty.hpp"
#include "EditorClient.hpp"
#include "EditorServer.hpp"
#include "Commandline.hpp"
#include "CommandlineException.hpp"
#include "ProgramName.hpp"
#include "DefaultConfig.hpp"
#include "ConfigException.hpp"


#if !defined(LUCED_USE_CYGWINFORKWORKAROUND)
#  if defined(_WIN32) && HAVE_WINDOWS_H && HAVE_SYS_CYGWIN_H && !DISABLE_CYGWINFORKWORKAROUND
#    define LUCED_USE_CYGWINFORKWORKAROUND 1
#  else
#    define LUCED_USE_CYGWINFORKWORKAROUND 0
#  endif
#endif

             
using namespace LucED;


static bool startupClient(int argc, char** argv)
{
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
    return isServerStartupNeeded;
}


static void startupServer(int argc, char** argv)
{
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



int main(int argc, char** argv)
{
    setlocale(LC_CTYPE, "");

    int rc = 0;
    
    try
    {
        ProgramName::set(argv[0]);
        
#if LUCED_USE_CYGWINFORKWORKAROUND
        {
            // use Windows function "CreateProcess" for server startup
            
            cygwin_internal(CW_SYNC_WINENV); // synchronize the Win32 environment with the Cygwin environment
            
            String programName = argv[0];
            
            if (!programName.startsWith("forked_"))
            {
                // we are client
                
                bool isServerStartupNeeded = startupClient(argc, argv);
                
                if (isServerStartupNeeded)
                {
                    ByteBuffer commandline;
                               commandline.appendCStr("forked_");
                               commandline.appendCStr(GetCommandLine());
                               commandline.append((byte)0);
                    
                    char winPath[800];
                    
                    GetModuleFileName(NULL,              // HMODULE hModule,
                                      winPath,          // LPTSTR lpFilename,
                                      sizeof(winPath)); // DWORD nSize
                    
                    STARTUPINFO         su;
                    PROCESS_INFORMATION pi;
                    
                    memset(&su, 0, sizeof(su));
                    memset(&pi, 0, sizeof(pi));
                    
                    bool wasOK = CreateProcess(winPath,   // lpApplicationName
                                               (char*)commandline.getPtr(0),    // lpCommandLine
                                               0,         // lpProcessAttributes,
                                               0,         // lpThreadAttributes,
                                               true,      // bInheritHandles,
                                               0, // dwCreationFlags,
                                               NULL,      // lpEnvironment,
                                               NULL,      // lpCurrentDirectory,
                                               &su,         // lpStartupInfo,
                                               &pi);        // lpProcessInformation
                    if (!wasOK)
                    {
                        fprintf(stderr, "[%s]: Could not create process: error %d\n", argv[0], (int)GetLastError());
                        rc = 32;
                    }
                }
            }
            else
            {
                // we are server
    
                startupServer(argc, argv);
            }
        }
#else
        {
            // use posix fork for server startup
    
            bool isServerStartupNeeded = startupClient(argc, argv);
    
            if (isServerStartupNeeded) // start new server
            {
                pid_t pid = fork();
                
                if (pid == 0) // we are child process: the new server
                {
                    setlocale(LC_CTYPE, "");
                    
                    startupServer(argc, argv);
                }
                else if (pid < 0)
                {
                    fprintf(stderr, "[%s]: Could not fork process: %s\n", argv[0], strerror(errno));
                    rc = 32;
                }
            }
        }
#endif
    }
    catch (CommandlineException& ex)
    {
        fprintf(stderr, "[%s]: Commandline Error: %s\n", argv[0], ex.getMessage().toCString());
        rc = 1;
    }
    catch (ConfigException& ex)
    {
        fprintf(stderr, "[%s]: %s: %s\n", argv[0], ex.what(), ex.getMessage().toCString());
        rc = 16;
        if (ex.getErrorList().isValid())
        {
            for (int i = 0; i < ex.getErrorList()->getLength(); ++i) {
                ConfigException::Error e = ex.getErrorList()->get(i);
                fprintf(stderr, "[%s]: %s:%d:%s\n", argv[0], e.getConfigFileName().toCString(),
                                                             e.getLineNumber(),
                                                             e.getMessage().toCString());
            }
        }
    }
    catch (BaseException& ex)
    {
        fprintf(stderr, "[%s]: %s\n", argv[0], ex.toString().toCString());
        rc = 16;
    }

    #ifdef DEBUG
        HeapObjectChecker::assertAllCleared();
    #endif

    return rc;
}
