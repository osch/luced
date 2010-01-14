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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include "config.h"

#if HAVE_EXECINFO_H && HAVE_SYS_WAIT_H && !defined(HAS_STACKTRACE)
#  define HAS_STACKTRACE
#endif

#ifdef HAS_STACKTRACE
#include <execinfo.h>
#include <sys/wait.h>
#endif


#include <vector>

#include "debug.hpp"
#include "util.hpp"
#include "StackTrace.hpp"
#include "ProgramName.hpp"

using namespace LucED;

static bool  isAddr2LineRunning = false;
static bool  canStartAddr2Line  = true;
static int   childOutFd = -1;
static int   childInpFd = -1;
static pid_t childPid  = -1;


#ifdef HAS_STACKTRACE
static bool isChildRunning()
{
    if (isAddr2LineRunning)
    {
        int stat_loc;
        if (waitpid(childPid, &stat_loc, WNOHANG) == childPid) {
            if (WIFEXITED(stat_loc)) {
                isAddr2LineRunning = false;
            }
        }
    }
    return isAddr2LineRunning;
}
#endif

std::string StackTrace::getCurrent()
{
#ifdef HAS_STACKTRACE

    if (!canStartAddr2Line) {
        return "";    
    }

    int tryCounter = 1;
    
startAddr2Line:

    std::string message;
                message.append("\n****** StackTrace:\n");

    if (!isChildRunning())
    {
        std::vector<std::string> argStrings;
        {
            argStrings.push_back("addr2line");

        if (tryCounter == 1) {
            argStrings.push_back("-x"); // <-- not supported under older addr2line
        }
            argStrings.push_back("-f");
            argStrings.push_back("-s");
            argStrings.push_back("-C");
    
            argStrings.push_back("-e"); argStrings.push_back(ProgramName::get());
        }
        std::vector<char*> argPtrs;
        {
            for (int i = 0, n = argStrings.size(); i < n; ++i) {
                argPtrs.push_back(const_cast<char*>(argStrings[i].c_str()));
            }
            argPtrs.push_back(NULL);
        }

        int outPipe[2];
        int inpPipe[2];
        
        if (::pipe(outPipe) != 0) {
            message.append("*** Error: Could not create pipe: ").append(strerror(errno)).append("\n");
        }
        if (::pipe(inpPipe) != 0) {
            message.append("*** Error: Could not create pipe: ").append(strerror(errno)).append("\n");
        }
        
        pid_t pid = ::fork();
        
        if (pid == 0) 
        {
            // we are child process: the new program
            
            sigset_t blockedSignals;
            sigemptyset(&blockedSignals);
            ::sigprocmask(SIG_SETMASK, &blockedSignals, NULL);
    
            ::close(outPipe[0]);
            ::close(inpPipe[1]);
            
            ::dup2(inpPipe[0], 0);
            ::dup2(outPipe[1], 1);
//            ::dup2(outPipe[1], 2);
    
            ::close(outPipe[1]);
            ::close(inpPipe[0]);
            int rc = execvp("addr2line", &argPtrs[0]);
            if (rc != 0) {
                exit(errno);
            }
        }
        else if (pid < 0)
        {
            message.append("*** Error: Could not fork process: ").append(strerror(errno)).append("\n");
        }
    
        // we are parent process
    
        ::close(outPipe[1]);
        ::close(inpPipe[0]);
        childOutFd         = outPipe[0];
        childInpFd         = inpPipe[1];
        isAddr2LineRunning = true;
        childPid           = pid;
    }
    
    std::string data;
    {
        void*  array[2000];
        int size = ::backtrace(array, 2000);
        
        for (int i = 0; i < size && isChildRunning(); ++i)
        {
            char buffer[10000];

            sprintf(buffer, "%p\n",array[i]);
            
            int writeResult = -1;
            int bytesRead   = -1;
            
            if (isChildRunning()) {
                writeResult = write(childInpFd, buffer, strlen(buffer));
            }            
            if (isChildRunning()) {
                bytesRead = read(childOutFd, buffer, sizeof(buffer));
            }
            if (isChildRunning() && writeResult > 0 && bytesRead > 0)
            {
                data.append(buffer, bytesRead);
            }
        }
    }
    if (!isChildRunning()) {
        if (tryCounter == 1) {
            tryCounter = 2;
            goto startAddr2Line;
        } else {
            canStartAddr2Line = false;
            return "";
        }
    }
    
    std::vector<std::string> functionLines; int maxFunctionLength = 0;
    std::vector<std::string> fileLines;     int maxFileLength     = 0;
    
    for (int i = 0, j = 0, n = 0; i < data.length(); ++i) {
        if (data[i] == '\n') {
            std::string line = data.substr(j, i - j);
            j = i + 1;
            if (n >= 2)
            {
                if (n % 2 == 0) {
                    functionLines.push_back(line);
                    util::maximize(&maxFunctionLength, line.length());
                } else {
                    fileLines.push_back(line);
                    util::maximize(&maxFileLength, line.length());
                }
            }
            n += 1;
        }
    }
    if (fileLines.size() < functionLines.size()) {
        fileLines.push_back("");
    }
    for (int i = 0; i < functionLines.size(); ++i) {
        char buffer[4000];
        sprintf(buffer, "   %-*.*s %s\n", maxFileLength, maxFileLength, 
                                          fileLines[i].c_str(), 
                                          functionLines[i].c_str());
        message.append(buffer);
    }
    message.append("******\n");
    return message;

#endif
}


void StackTrace::print(FILE* fprintfOutput)
{
#ifdef HAS_STACKTRACE
    fprintf(fprintfOutput, "%s", getCurrent().c_str());
#endif
}

