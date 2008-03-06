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
#include "EventDispatcher.hpp"

namespace LucED
{

class ProgramExecutor : public RunningComponent
{
public:
    typedef LucED::OwningPtr<ProgramExecutor> OwningPtr;
    typedef LucED::WeakPtr  <ProgramExecutor> WeakPtr;

    struct Result
    {
        Result(int returnCode, const char* outputBuffer, int outputLength)
            : returnCode(returnCode),
              outputBuffer(outputBuffer),
              outputLength(outputLength)
        {}
        
        int         returnCode;
        const char* outputBuffer;
        int         outputLength;
    };

    static WeakPtr start(const String& programName,
                         const String& input,
                         Callback<Result>::Ptr finishedCallback)
    {
        OwningPtr rslt(new ProgramExecutor());
        rslt->programName = programName;
        rslt->input       = input;
        rslt->finishedCallback = finishedCallback;
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
    
    String         programName;
    String         input;
    int            inputPosition;
    MemArray<char> output;
    int            outputPosition;
    
    FileDescriptorListener::Ptr childInputListener;
    FileDescriptorListener::Ptr childOutputListener;
    
    Callback<Result>::Ptr finishedCallback;
};

} // namespace LucED

#endif // PROGRAM_EXECUTOR_HPP
