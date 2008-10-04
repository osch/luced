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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include "options.hpp"

#ifdef HAS_STACKTRACE
#include <execinfo.h>
#endif

#include "debug.hpp"
#include "util.hpp"
#include "StackTrace.hpp"
#include "ObjectArray.hpp"
#include "MemArray.hpp"
#include "String.hpp"
#include "ProgramName.hpp"

using namespace LucED;

static int childOutFd = -1;
static int childInpFd = -1;


void StackTrace::print()
{
#ifdef HAS_STACKTRACE
    String message;
           message << "\n****** StackTrace:\n";

    if (childOutFd == -1)
    {
        ObjectArray<String> argStrings;
        {
            argStrings.append("addr2line");
    
            argStrings.append("-i");
            argStrings.append("-f");
            argStrings.append("-s");
            argStrings.append("-C");
    
            argStrings.append("-e"); argStrings.append(ProgramName::get());
        }
        MemArray<char*> argPtrs;
        {
            for (int i = 0, n = argStrings.getLength(); i < n; ++i) {
                argPtrs.append(const_cast<char*>(argStrings[i].toCString()));
            }
            argPtrs.append(NULL);
        }

        int outPipe[2];
        int inpPipe[2];
        
        if (::pipe(outPipe) != 0) {
            message << "*** Error: Could not create pipe: " << strerror(errno) << "\n";
        }
        if (::pipe(inpPipe) != 0) {
            message << "*** Error: Could not create pipe: " << strerror(errno) << "\n";
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
            ::dup2(outPipe[1], 2);
    
            ::close(outPipe[1]);
            ::close(inpPipe[0]);
            
            execvp("addr2line", argPtrs.getPtr(0));
        }
        else if (pid < 0)
        {
            message << "*** Error: Could not fork process: " << strerror(errno) << "\n";
        }
    
        // we are parent process
    
        ::close(outPipe[1]);
        ::close(inpPipe[0]);
        childOutFd = outPipe[0];
        childInpFd = inpPipe[1];
    }
    
    String data;
    {
        void*  array[2000];
        int size = ::backtrace(array, 2000);
        
        for (int i = 0; i < size; ++i)
        {
            char buffer[10000];

            sprintf(buffer, "%p\n",array[i]);
            
            write(childInpFd, buffer, strlen(buffer));
            
            int bytesRead = read(childOutFd, buffer, sizeof(buffer));
            data.append(buffer, bytesRead);
        }
    }
    
    ObjectArray<String> functionLines; int maxFunctionLength = 0;
    ObjectArray<String> fileLines;     int maxFileLength     = 0;
    
    for (int i = 0, j = 0, n = 0; i < data.getLength(); ++i) {
        if (data[i] == '\n') {
            String line = data.getSubstringBetween(j, i);
            j = i + 1;
            if (n >= 2)
            {
                if (n % 2 == 0) {
                    functionLines.append(line);
                    util::maximize(&maxFunctionLength, line.getLength());
                } else {
                    fileLines.append(line);
                    util::maximize(&maxFileLength, line.getLength());
                }
            }
            n += 1;
        }
    }
    if (fileLines.getLength() < functionLines.getLength()) {
        fileLines.append("");
    }
    for (int i = 0; i < functionLines.getLength(); ++i) {
        char buffer[4000];
        sprintf(buffer, "   %-*.*s %s\n", maxFileLength, maxFileLength, 
                                          fileLines[i].toCString(), 
                                          functionLines[i].toCString());
        message << buffer;
    }
    message << "******\n";
    printf("%s", message.toCString());
#endif
}

