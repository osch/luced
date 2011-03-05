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

#ifndef PROGRAM_EXECUTOR_HPP
#define PROGRAM_EXECUTOR_HPP

#include "RunningComponent.hpp"
#include "FileDescriptorListener.hpp"
#include "OwningPtr.hpp"
#include "WeakPtr.hpp"
#include "RawPtr.hpp"
#include "ByteBuffer.hpp"
#include "EventDispatcher.hpp"
#include "HeapHashMap.hpp"
#include "Commandline.hpp"

namespace LucED
{

class ProgramExecutor : public RunningComponent
{
public:
    typedef LucED::OwningPtr<ProgramExecutor> OwningPtr;
    typedef LucED::WeakPtr  <ProgramExecutor> WeakPtr;

    struct Result
    {
        Result(int returnCode, RawPtr<ByteBuffer> outputBuffer)
            : returnCode(returnCode),
              outputBuffer(outputBuffer)
        {}
        
        int                returnCode;
        RawPtr<ByteBuffer> outputBuffer;
    };

    static WeakPtr start(Commandline::Ptr                commandline,
                         const String&                   input,
                         HeapHashMap<String,String>::Ptr additionalEnvironment,
                         Callback<Result>::Ptr           finishedCallback)
    {
        OwningPtr rslt(new ProgramExecutor());
        rslt->additionalEnvironment = additionalEnvironment;
        rslt->input                 = input;
        rslt->commandline           = commandline;
        rslt->finishedCallback      = finishedCallback;
        EventDispatcher::getInstance()->registerRunningComponent(rslt);
        rslt->startExecuting();
        return rslt;
    }

private:
    ProgramExecutor()
        : inputPosition(0),
          outputPosition(0)
    {}
    
    void startExecuting();
    
    void writeToChild (int fileDescriptor);
    void readFromChild(int fileDescriptor);
    
    void catchTerminatedChild(int returnCode);
    
    HeapHashMap<String,String>::Ptr additionalEnvironment;
    
    Commandline::Ptr commandline;
    String           input;
    int              inputPosition;
    ByteBuffer       output;
    int              outputPosition;
    
    FileDescriptorListener::Ptr childInputListener;
    FileDescriptorListener::Ptr childOutputListener;
    
    Callback<Result>::Ptr finishedCallback;
};

} // namespace LucED

#endif // PROGRAM_EXECUTOR_HPP
