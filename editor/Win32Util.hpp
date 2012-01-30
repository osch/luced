/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2012 Oliver Schmidt, oliver at luced dot de
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

#ifndef WIN32_UTIL_HPP
#define WIN32_UTIL_HPP

#include "config.h"

#if HAVE_WINDOWS_H
#   include <windows.h>
#endif
#if HAVE_SYS_CYGWIN_H
#   include <sys/cygwin.h>
#endif

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>


#if !defined(LUCED_USE_CYGWIN)
#  if defined(_WIN32) && HAVE_WINDOWS_H && HAVE_SYS_CYGWIN_H
#    define LUCED_USE_CYGWIN 1
#  else
#    define LUCED_USE_CYGWIN 0
#  endif
#endif

#if !defined(LUCED_USE_WIN32)
#  if defined(_WIN32) && HAVE_WINDOWS_H
#    define LUCED_USE_WIN32 1
#  else
#    define LUCED_USE_WIN32 0
#  endif
#endif

#include "String.hpp"
#include "CharArray.hpp"
#include "SystemException.hpp"
#include "File.hpp"
#include "HeapHashMap.hpp"

namespace LucED
{

#if LUCED_USE_WIN32
class Win32Util
{
public:
    typedef HeapHashMap<String,String> EnvMap;

#if LUCED_USE_CYGWIN
    static void copyCygwinEnvToWin32() 
    {
        cygwin_internal(CW_SYNC_WINENV); // synchronize the Win32 environment with the Cygwin environment
    }
#endif

    static EnvMap::Ptr getWindowsEnvironmentMap()
    {
        EnvMap::Ptr rslt = EnvMap::create();
        
        char* environmentStrings = GetEnvironmentStrings();
        if (environmentStrings != NULL)
        {
            const char* entry = environmentStrings;
            while (*entry != '\0')
            {
                int i = 0;
                while (entry[i] != '\0' && entry[i] != '=') { ++i; }
                if (entry[i] == '=') {
                    String key = String(entry, i);
                    ++i;
                    int j = i;
                    while (entry[j] != '\0') { ++j; }
                    String value = String(entry + i, j - i);
                    rslt->set(key, value);
                    entry += j + 1;
                } else {
                    entry += i + 1;
                }
            }
            FreeEnvironmentStrings(environmentStrings);
        }
        return rslt;
    }
    static String buildEnvironmentStringsFromMap(EnvMap::Ptr map)
    {
        String rslt;
        EnvMap::Iterator iter = map->getIterator();
        for (; !iter.isAtEnd(); iter.gotoNext()) {
            rslt << iter.getKey() << '=' << iter.getValue() << '\0';
        }
        rslt << '\0' << '\0';
        return rslt;
    }
    static String getThisProgramFileName()
    {
        MemArray<char> buffer(2000);
        long len = 0;
        while (true)
        {
            len = GetModuleFileName(NULL, buffer.getPtr(), buffer.getLength());
            if (len == buffer.getLength()) {
                buffer.increaseTo(len + 1000);
                continue;
            } else {
                break;
            }
        }
        if (len > 0) {
            return String(buffer.getPtr(), len);
        } else {
            throw SystemException(String() << "Error in call to Win32 GetModuleFileName: " << getLastWin32ErrorMessage());
        }
    }
    static bool doesWindowsFileExist(const String& win32FileName)
    {
        return GetFileAttributes(win32FileName.toCString()) != INVALID_FILE_ATTRIBUTES;
    }
#if LUCED_USE_CYGWIN
    static String toWindowsFileName(const String& cygwinFileName)
    {
        MemArray<char> buffer(2000);

        while (true)
        {
            ssize_t rc = cygwin_conv_path(CCP_POSIX_TO_WIN_A|CCP_ABSOLUTE,
                                          cygwinFileName.toCString(),
                                          buffer.getPtr(), 
                                          buffer.getLength());

            if (rc != 0 && errno == ENOSPC) {
                buffer.increaseTo(buffer.getLength() + 1000);
                continue;
            } else if (rc != 0) {
                throw SystemException(String() << "error in call to cygwin_conv_path for '" << cygwinFileName << "': " << strerror(errno));
            } else {
                return String(buffer.getPtr());
            }
        }
    }
    static String getWindowsProgramFileName(const String& cygwinProgramFileName)
    {
        String winFile = toWindowsFileName(File(cygwinProgramFileName).getAbsoluteNameWithResolvedLinks());
        
        
        if (!doesWindowsFileExist(winFile)) {
            winFile << ".exe";
            if (!doesWindowsFileExist(winFile)) {
                throw SystemException(String() << "Cannot find executable '" << cygwinProgramFileName << "'");
            }
        }
        return winFile;
    }
#endif
    static const char* getThisProgramCommandline()
    {
        return GetCommandLine();
    }
    static void createPipeForCreatedProcess(HANDLE* readHandle, HANDLE* writeHandle)
    {
        SECURITY_ATTRIBUTES sa; 
         
        memset(&sa, 0, sizeof(sa));
        sa.nLength = sizeof(sa); 
        sa.bInheritHandle = TRUE;  // Set the bInheritHandle flag so pipe handles are inherited. 
        sa.lpSecurityDescriptor = NULL;
        
        if (!CreatePipe(readHandle, writeHandle, &sa, 0)) 
        {
            throw SystemException(String() << "Error in call to Win32 CreatePipe: " << getLastWin32ErrorMessage());
        }
    }
    static void clearInheritFlag(HANDLE h)
    {
        if (!SetHandleInformation(h, HANDLE_FLAG_INHERIT, 0))
        {
            throw SystemException(String() << "Error in call to Win32 SetHandleInformation: " << getLastWin32ErrorMessage());
        }
    }

    static HANDLE createProcess(const String& win32ProgramFileName,
                                const String& commandLine,
                                EnvMap::Ptr   environmentMap = Null,
                                HANDLE*       inputHandle    = NULL,
                                HANDLE*       outputHandle   = NULL,
                                HANDLE*       stderrHandle   = NULL)
    {
        STARTUPINFO         su;
        memset(&su, 0, sizeof(su));
        su.cb = sizeof(su);

        bool useHandles = (    inputHandle != NULL 
                           && outputHandle != NULL 
                           && stderrHandle != NULL);

        HANDLE stdinRead,  stdinWrite;
        HANDLE stdoutRead, stdoutWrite;
        HANDLE stderrRead, stderrWrite;
        
        if (useHandles)
        {
            createPipeForCreatedProcess(&stdinRead,  &stdinWrite);
            createPipeForCreatedProcess(&stdoutRead, &stdoutWrite);
            createPipeForCreatedProcess(&stderrRead, &stderrWrite);
            
            clearInheritFlag(stdinWrite);
            clearInheritFlag(stdoutRead);
            clearInheritFlag(stderrRead);

            *inputHandle  = stdinWrite;
            *outputHandle = stdoutRead;
            *stderrHandle = stderrRead;

            su.hStdInput  = stdinRead;
            su.hStdOutput = stdoutWrite;
            su.hStdError  = stderrWrite;
            su.dwFlags |= STARTF_USESTDHANDLES;
        }

        PROCESS_INFORMATION pi;
        memset(&pi, 0, sizeof(pi));
        
        CharArray commandLineBuffer;
                  commandLineBuffer.appendString(commandLine);
                  commandLineBuffer.append('\0');
        
        {
            char*      environmentStringsPtr = NULL;
            CharArray  environmentStrings;
            if (environmentMap.isValid()) {
                environmentStrings.appendString(buildEnvironmentStringsFromMap(environmentMap));
                environmentStringsPtr = environmentStrings.getPtr();
            }
            bool wasOK = CreateProcess(win32ProgramFileName.toCString(),   // lpApplicationName
                                       commandLineBuffer.getPtr(),    // lpCommandLine
                                       0,         // lpProcessAttributes,
                                       0,         // lpThreadAttributes,
                                       true,      // bInheritHandles,
                                       0, // dwCreationFlags,
                                       environmentStringsPtr,      // lpEnvironment,
                                       NULL,      // lpCurrentDirectory,
                                       &su,         // in:  lpStartupInfo,
                                       &pi);        // out: lpProcessInformation
    
            if (!wasOK)
            {
                throw SystemException(String() << "Error in call to Win32 CreateProcess for '" << win32ProgramFileName << "'");
            }
        }
        if (useHandles) {
            CloseHandle(stdinRead);
            CloseHandle(stdoutWrite);
            CloseHandle(stderrWrite);
        }
        CloseHandle(pi.hThread);        
        
        return pi.hProcess;
    }
    static String getLastWin32ErrorMessage()
    {
        String rslt;
        
        DWORD lastErrorCode = GetLastError();
        
        LPTSTR errorText = NULL;
        
        FormatMessage(  FORMAT_MESSAGE_FROM_SYSTEM
                      | FORMAT_MESSAGE_ALLOCATE_BUFFER
                      | FORMAT_MESSAGE_IGNORE_INSERTS,  
                      NULL,
                      HRESULT_FROM_WIN32(lastErrorCode),
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                      (LPTSTR)&errorText,
                      0,
                      NULL);
        
        if (errorText != NULL)
        {
            for (const char* p = errorText; *p != '\0'; ++p) {
                if (*p == '\r' || *p == '\n') {
                    rslt << ' ';
                    do { ++p; } while (*p == '\r' || *p == '\n');
                } else {
                    rslt << *p;
                }
            } 
            LocalFree(errorText);
        }
        return rslt;
    }
private:
    Win32Util();
    Win32Util(const Win32Util&);
};
#endif

} // namespace LucED

#endif // WIN32_UTIL_HPP

