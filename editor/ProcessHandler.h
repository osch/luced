/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, osch@luced.de
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

#ifndef PROCESSHANDLER_H
#define PROCESSHANDLER_H

#include "HeapObject.h"
#include "TimeVal.h"
#include "OwningPtr.h"

namespace LucED {


class AbstractProcessHandler : public HeapObject
{
public:

    typedef OwningPtr<AbstractProcessHandler> Ptr;
    
    virtual ~AbstractProcessHandler() {}
    
    virtual void disable() = 0;
    virtual bool isEnabled() = 0;
    virtual bool needsProcessing() = 0;

    void execute(long requestedMicroSecs) {
        ASSERT(requestedMicroSecs >= 0);
        int processingAmount = this->processingAmount;
        if (this->microSecs != 0) {
            processingAmount = (int)(
                    (((double)processingAmount)/((double)this->microSecs)) * ((double)requestedMicroSecs));
            if (processingAmount == 0)
                processingAmount = 1;
            else if (processingAmount > 1000)
                processingAmount = 1000;
        }
        ASSERT(processingAmount > 0);
        TimeVal startTime;
        TimeVal endTime;
        startTime.setToCurrentTime();
        this->processingAmount = process(processingAmount);
        endTime.setToCurrentTime();
        this->microSecs = TimeVal::diffMicroSecs(startTime, endTime);
    }
    long getMicroSecs() {
        return microSecs;
    }
    long getProcessingAmount() {
        return processingAmount;
    }
protected:
    AbstractProcessHandler() {
        microSecs = 0;
        processingAmount = 10;
    }
    virtual int process(int requestedMicroSecs) = 0;
private:
    long microSecs;
    int processingAmount;
};


template<class T> class ProcessHandlerImpl : public AbstractProcessHandler
{
public:
    static Ptr create(T* objectPtr, int (T::*methodProcess)(int), bool (T::*methodNeedsProcess)()) {
        return Ptr(new ProcessHandlerImpl(objectPtr, methodProcess, methodNeedsProcess));
    }
    
    virtual void disable() {
        objectPtr = NULL;
    }
    virtual bool isEnabled() {
        return objectPtr != NULL;
    }
    virtual bool needsProcessing() {
        return (objectPtr->*methodNeedsProcess)();
    }
protected:
    virtual int process(int requestedProcessingAmount) {
        return (objectPtr->*methodProcess)(requestedProcessingAmount);
    }
private:
    
    ProcessHandlerImpl(T* objectPtr, int (T::*methodProcess)(int), bool (T::*methodNeedsProcess)())
        : objectPtr(objectPtr), methodProcess(methodProcess), methodNeedsProcess(methodNeedsProcess)
    {}
    
    T* objectPtr;
    int (T::*methodProcess)(int);
    bool (T::*methodNeedsProcess)();
};


class ProcessHandler : public AbstractProcessHandler::Ptr
{
public:
    ProcessHandler() {}
protected:
    template<class T> ProcessHandler(T* objectPtr, int (T::*methodProcess)(int), bool (T::*methodNeedsProcess)())
        : AbstractProcessHandler::Ptr(ProcessHandlerImpl<T>::create(objectPtr, methodProcess, methodNeedsProcess))
    {}
};

class ProcessHandlerSlot : public ProcessHandler
{
public:
    template<class T> ProcessHandlerSlot(T* objectPtr, int (T::*methodProcess)(int), bool (T::*methodNeedsProcess)())
        : ProcessHandler(objectPtr, methodProcess, methodNeedsProcess) {}

    ~ProcessHandlerSlot() {
        (*this)->disable();
    }
};



} // namespace LucED

#endif // PROCESSHANDLER_H
