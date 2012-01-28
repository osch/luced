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
#include <errno.h>
#include <signal.h>
#include <fcntl.h>

#include "ProgramExecutor.hpp"
#include "SystemException.hpp"
#include "MemArray.hpp"
#include "Win32Util.hpp"

#if LUCED_USE_CYGWIN_FORK_WORKAROUND
#include "Thread.hpp"
#include "Mutex.hpp"
#endif

using namespace LucED;

ProgramExecutor::ProgramExecutor()
#if !LUCED_USE_CYGWIN_FORK_WORKAROUND
    : inputPosition(0),
      outputPosition(0)
#endif
{}


#if LUCED_USE_CYGWIN_FORK_WORKAROUND
class ProgramExecutor::Win32StdoutThread : public Thread
{
public:
    typedef LucED::OwningPtr<Win32StdoutThread> Ptr;
    
    static Ptr create(Callback<>::Ptr  processFinishedCallback,
                      HANDLE           processHandle,
                      HANDLE           ioHandle) 
    {
        return Ptr(new Win32StdoutThread(processFinishedCallback, processHandle, ioHandle));
    }
    void notifyStdinFinished() {
        Mutex::Lock lock(mutex);
        stdinFinishedFlag = true;
        lock.notify();
    }
    void notifyStderrFinished() {
        Mutex::Lock lock(mutex);
        stderrFinishedFlag = true;
        lock.notify();
    }
    int getReturnCode() const {
        return returnCode;
    }
    RawPtr<ByteBuffer> getOutput() {
        return &output;
    }
private:
    Win32StdoutThread(Callback<>::Ptr  processFinishedCallback,
                      HANDLE           processHandle,
                      HANDLE           ioHandle)
      : processFinishedCallback(processFinishedCallback),
        processHandle(processHandle),
        ioHandle(ioHandle),
        mutex(Mutex::create()),
        outputPosition(0),
        stdinFinishedFlag(false),
        stderrFinishedFlag(false),
        returnCode(0)
    {}
    
    void waitForStinStderrFinished() {
        Mutex::Lock lock(mutex);
        while (!stdinFinishedFlag && !stderrFinishedFlag) {
            lock.waitForNotify();
        }
    }
    
    virtual void main()
    {
        while (true)
        {
            if (output.getLength() - outputPosition < 30000) {
                output.increaseTo(output.getLength() + 30000);
            }
            int possibleLength = output.getLength() - outputPosition;
            
            DWORD readCounter = 0;

            bool wasOK = ReadFile(ioHandle,
                                  output.getAmount(outputPosition, possibleLength),
                                  possibleLength,
                                  &readCounter,
                                  NULL);

            if (wasOK && readCounter > 0) {
                outputPosition += readCounter;
            }
            
            if (!wasOK || readCounter == 0) {
                break;
            }
        }
        output.removeTail(outputPosition);

        CloseHandle(ioHandle);

        WaitForSingleObject(processHandle, INFINITE);
        
        returnCode = 0;
        GetExitCodeProcess(processHandle, &returnCode);
        CloseHandle(processHandle);

        waitForStinStderrFinished();
        
        EventDispatcher::getInstance()->executeTaskOnMainThread(processFinishedCallback);
    }
    
    Callback<>::Ptr  processFinishedCallback;
    HANDLE           processHandle;
    HANDLE           ioHandle;

    Mutex::Ptr       mutex;

    ByteBuffer       output;
    int              outputPosition;

    bool  stdinFinishedFlag;
    bool  stderrFinishedFlag;

    DWORD returnCode;
};
#endif // LUCED_USE_CYGWIN_FORK_WORKAROUND

#if LUCED_USE_CYGWIN_FORK_WORKAROUND
class ProgramExecutor::Win32StdinThread : public Thread
{
public:
    typedef LucED::OwningPtr<Win32StdinThread> Ptr;
    
    static Ptr create(HANDLE                 ioHandle,
                      const String&          input,
                      Win32StdoutThread::Ptr stdoutThread) 
    {
        Ptr rslt = Ptr(new Win32StdinThread(ioHandle, input, stdoutThread));
        return rslt;
    }
private:
    Win32StdinThread(HANDLE                 ioHandle,
                     const String&          input,
                     Win32StdoutThread::Ptr stdoutThread)
      : ioHandle(ioHandle),
        input(input),
        inputPosition(0),
        stdoutThread(stdoutThread)
    {}
    
    virtual void main()
    {
        while (true)
        {
            DWORD writeCounter = 0;
            
            bool wasOK = WriteFile(ioHandle,
                                   input.toCString() + inputPosition, 
                                   input.getLength() - inputPosition,
                                   &writeCounter,
                                   NULL);
            if (wasOK) {
                inputPosition += writeCounter;
            }
            if (!wasOK || inputPosition >= input.getLength()) {
                break;
            }
        }
        CloseHandle(ioHandle);
        input.clear();
        stdoutThread->notifyStdinFinished();
    }
    
    HANDLE                 ioHandle;
    String                 input;
    int                    inputPosition;
    Win32StdoutThread::Ptr stdoutThread;
};
#endif // LUCED_USE_CYGWIN_FORK_WORKAROUND

#if LUCED_USE_CYGWIN_FORK_WORKAROUND
class ProgramExecutor::Win32StderrThread : public Thread
{
public:
    typedef LucED::OwningPtr<Win32StderrThread> Ptr;
    
    static Ptr create(HANDLE                 ioHandle,
                      Win32StdoutThread::Ptr stdoutThread) 
    {
        return Ptr(new Win32StderrThread(ioHandle, stdoutThread));
    }

    RawPtr<ByteBuffer> getOutput() {
        return &output;
    }

private:
    Win32StderrThread(HANDLE                 ioHandle,
                      Win32StdoutThread::Ptr stdoutThread)
      : ioHandle(ioHandle),
        outputPosition(0),
        stdoutThread(stdoutThread)
    {}
    
    virtual void main()
    {
        while (true)
        {
            if (output.getLength() - outputPosition < 30000) {
                output.increaseTo(output.getLength() + 30000);
            }
            int possibleLength = output.getLength() - outputPosition;
            
            DWORD readCounter = 0;

            bool wasOK = ReadFile(ioHandle,
                                  output.getAmount(outputPosition, possibleLength),
                                  possibleLength,
                                  &readCounter,
                                  NULL);
            if (wasOK && readCounter > 0) {
                outputPosition += readCounter;
            }
            
            if (!wasOK || readCounter == 0) {
                break;
            }
        }
        output.removeTail(outputPosition);

        CloseHandle(ioHandle);

        stdoutThread->notifyStderrFinished();
    }
    
    HANDLE           ioHandle;

    ByteBuffer       output;
    int              outputPosition;

    Win32StdoutThread::Ptr stdoutThread;
};
#endif // LUCED_USE_CYGWIN_FORK_WORKAROUND


#if LUCED_USE_CYGWIN_FORK_WORKAROUND
void ProgramExecutor::handleTerminatedChild()
{
        ByteBuffer outputBuffer;
        outputBuffer.takeOver(win32StdoutThread->getOutput());
        {
            ByteBuffer errorOut;
            errorOut.takeOver(win32StderrThread->getOutput());

            outputBuffer.append(errorOut);
        }
        finishedCallback->call(Result(win32StdoutThread->getReturnCode(),
                                      &outputBuffer));

        EventDispatcher::getInstance()->deregisterRunningComponent(this);
}
#endif // LUCED_USE_CYGWIN_FORK_WORKAROUND


void ProgramExecutor::startExecuting()
{
#if LUCED_USE_CYGWIN_FORK_WORKAROUND
    {
        Win32Util::copyCygwinEnvToWin32();
        
        HeapHashMap<String,String>::Ptr envMap = Win32Util::getWindowsEnvironmentMap();

        if (additionalEnvironment.isValid())
        {
            HashMap<String,String>::Iterator iterator = additionalEnvironment->getIterator();
            for (; !iterator.isAtEnd(); iterator.gotoNext()) {
                envMap->set(iterator.getKey(), iterator.getValue());
            }
        }
        
        
        HANDLE inputHandle;
        HANDLE outputHandle;
        HANDLE errorHandle;
        
        HANDLE processHandle = Win32Util::createProcess(Win32Util::getWindowsProgramFileName(commandline->get(0)),
                                                        commandline->toQuotedString(),
                                                        envMap,
                                                        &inputHandle,
                                                        &outputHandle,
                                                        &errorHandle);

        win32StdoutThread = Win32StdoutThread::create(newCallback(this, &ProgramExecutor::handleTerminatedChild),
                                                      processHandle,
                                                      outputHandle);

        win32StdinThread = Win32StdinThread::create(inputHandle,
                                                    input,
                                                    win32StdoutThread);

        win32StderrThread = Win32StderrThread::create(errorHandle,
                                                      win32StdoutThread);
        input.clear();

        Thread::start(win32StdinThread);
        Thread::start(win32StdoutThread);
        Thread::start(win32StderrThread);
    }
#else // !LUCED_USE_CYGWIN_FORK_WORKAROUND
    {
        inputPosition = 0;
    
        output.clear();
        outputPosition = 0;
        
        int inpFd;
        int outFd;
    
        int inpPipe[2];
        int outPipe[2];
        
        if (::pipe(inpPipe) != 0) {
            throw SystemException(String() << "Could not create pipe: " << strerror(errno));
        }
        if (::pipe(outPipe) != 0) {
            throw SystemException(String() << "Could not create pipe: " << strerror(errno));
        }
        
        pid_t pid = ::fork();
        
        if (pid == 0) 
        {
            // we are child process: the new program
            
            sigset_t blockedSignals;
            sigemptyset(&blockedSignals);
            ::sigprocmask(SIG_SETMASK, &blockedSignals, NULL);
    
            ::close(inpPipe[1]);
            ::close(outPipe[0]);
            
            ::dup2(inpPipe[0], 0);
            ::dup2(outPipe[1], 1);
            ::dup2(outPipe[1], 2);
    
            ::close(inpPipe[0]);
            ::close(outPipe[1]);
    
            if (additionalEnvironment.isValid())
            {
                HashMap<String,String>::Iterator iterator = additionalEnvironment->getIterator();
                for (; !iterator.isAtEnd(); iterator.gotoNext()) {
                    const bool overwrite = true;
                    ::setenv(iterator.getKey().toCString(), 
                             iterator.getValue().toCString(), 
                             overwrite);
                }
            }
            
            ::execv(commandline->get(0).toCString(), 
                    (char*const*)commandline->getArgvPtr());
            
            throw SystemException(String() << "Could not execute process: " << strerror(errno));
        }
        else if (pid < 0)
        {
            throw SystemException(String() << "Could not fork process: " << strerror(errno));
        }
        
        // we are parent process
        
        ::close(inpPipe[0]);
        ::close(outPipe[1]);
    
        fcntl(inpPipe[1], F_SETFL, fcntl(inpPipe[1],F_GETFL) | O_NONBLOCK);
        fcntl(outPipe[0], F_SETFL, fcntl(outPipe[0],F_GETFL) | O_NONBLOCK);

        EventDispatcher::getInstance()
            ->registerForTerminatingChildProcess(pid, 
                                                 newCallback(this, 
                                                             &ProgramExecutor::catchTerminatedChild));
        inpFd = inpPipe[1];
        outFd = outPipe[0];

        childInputListener  = FileDescriptorListener::create(inpFd,
                                                             Callback<int>::Ptr(),
                                                             newCallback(this, &ProgramExecutor::writeToChild));
    
        childOutputListener = FileDescriptorListener::create(outFd,
                                                             newCallback(this, &ProgramExecutor::readFromChild),
                                                             Callback<int>::Ptr());
        
        EventDispatcher::getInstance()->registerFileDescriptorListener(childInputListener);
        EventDispatcher::getInstance()->registerFileDescriptorListener(childOutputListener);
    }
#endif // !LUCED_USE_CYGWIN_FORK_WORKAROUND
}

#if !LUCED_USE_CYGWIN_FORK_WORKAROUND
void ProgramExecutor::writeToChild(int fileDescriptor)
{
    int writeCounter = ::write(fileDescriptor, input.toCString() + inputPosition, 
                                               input.getLength() - inputPosition);
    if (writeCounter >= 0)
    {
        inputPosition += writeCounter;
        
        if (inputPosition == input.getLength()) {
            childInputListener->close();
        }
    }
    else {
        childInputListener->close();
    }
}
#endif // !LUCED_USE_CYGWIN_FORK_WORKAROUND


#if !LUCED_USE_CYGWIN_FORK_WORKAROUND
void ProgramExecutor::readFromChild(int fileDescriptor)
{
    if (output.getLength() - outputPosition < 30000) {
        output.increaseTo(output.getLength() + 30000);
    }
    int possibleLength = output.getLength() - outputPosition;

    int readCounter = ::read(fileDescriptor, output.getAmount(outputPosition, possibleLength),
                                             possibleLength);
    if (readCounter > 0) {
        outputPosition += readCounter;
    }
    else if (readCounter == 0)
    {
        childOutputListener->close();
        output.removeTail(outputPosition);
    }
    else {
        childOutputListener->close();
        catchTerminatedChild(0);
    }
}
#endif // !LUCED_USE_CYGWIN_FORK_WORKAROUND


#if !LUCED_USE_CYGWIN_FORK_WORKAROUND
void ProgramExecutor::catchTerminatedChild(int returnCode)
{
    childInputListener->close();
    childOutputListener->close();

    ASSERT(outputPosition <= output.getLength());

    output.removeTail(outputPosition);
    
    finishedCallback->call(Result(returnCode, 
                                  &output));
    input.clear();
    inputPosition = 0;    

    output.clear();
    outputPosition = 0;
    
    EventDispatcher::getInstance()->deregisterRunningComponent(this);
}
#endif // !LUCED_USE_CYGWIN_FORK_WORKAROUND
