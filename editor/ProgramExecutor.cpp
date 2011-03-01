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

#include "ProgramExecutor.hpp"
#include "SystemException.hpp"
#include "MemArray.hpp"


using namespace LucED;

void ProgramExecutor::startExecuting()
{
    inputPosition = 0;

    output.clear();
    outputPosition = 0;
    
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
    
    EventDispatcher::getInstance()
        ->registerForTerminatingChildProcess(pid, 
                                             newCallback(this, 
                                                         &ProgramExecutor::catchTerminatedChild));
    
    childInputListener  = FileDescriptorListener::create(inpPipe[1],
                                                         Callback<int>::Ptr(),
                                                         newCallback(this, &ProgramExecutor::writeToChild));

    childOutputListener = FileDescriptorListener::create(outPipe[0],
                                                         newCallback(this, &ProgramExecutor::readFromChild),
                                                         Callback<int>::Ptr());
    
    EventDispatcher::getInstance()->registerFileDescriptorListener(childInputListener);
    EventDispatcher::getInstance()->registerFileDescriptorListener(childOutputListener);
}

    
void ProgramExecutor::writeToChild(int fileDescriptor)
{
    //printf("writing\n");
    
    int writeCounter = ::write(fileDescriptor, input.toCString() + inputPosition, 
                                               input.getLength() - inputPosition);
    //printf("written %d\n", writeCounter);
    
    if (writeCounter >= 0)
    {
        inputPosition += writeCounter;
        
        if (inputPosition == input.getLength()) {
            childInputListener->close();
        }
    }
    else {
        //printf("Eror while writing %s\n", strerror(errno));
    }
}


void ProgramExecutor::readFromChild(int fileDescriptor)
{
    //printf("reading\n");

    if (output.getLength() - outputPosition < 30000) {
        output.increaseTo(output.getLength() + 30000);
    }

    int readCounter = ::read(fileDescriptor, output.getPtr()    + outputPosition,
                                             output.getLength() - outputPosition);
    //printf("read %d\n", readCounter);

    if (readCounter > 0) {
        outputPosition += readCounter;
    }
    else if (readCounter == 0)
    {
        //printf("reading finished\n");
        childOutputListener->close();
        output.removeTail(outputPosition);
    }
    else {
        //printf("Eror while reading %s\n", strerror(errno));
    }
}


void ProgramExecutor::catchTerminatedChild(int returnCode)
{
    //printf("Catched return code %d\n", returnCode);
    
    childInputListener->close();
    childOutputListener->close();

    ASSERT(outputPosition <= output.getLength());

    finishedCallback->call(Result(returnCode, 
                                  output.getPtr(), 
                                  outputPosition));
    input.clear();
    inputPosition = 0;    

    output.clear();
    outputPosition = 0;
    
    EventDispatcher::getInstance()->deregisterRunningComponent(this);
}

