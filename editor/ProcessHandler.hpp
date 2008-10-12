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

#ifndef PROCESS_HANDLER_HPP
#define PROCESS_HANDLER_HPP

#include "HeapObject.hpp"
#include "TimeVal.hpp"
#include "OwningPtr.hpp"

namespace LucED
{

class ProcessHandler : public HeapObject
{
public:
    typedef OwningPtr<ProcessHandler> Ptr;

    template<class T
            >
    static Ptr create(T* objectPtr, int (T::*methodProcess)(TimeVal), bool (T::*methodNeedsProcess)())
    {
        return Impl<T>::create(objectPtr, methodProcess, methodNeedsProcess);
    }

    virtual void disable() = 0;

    virtual bool isEnabled() = 0;

    virtual bool needsProcessing() = 0;
    
    virtual int process(TimeVal endTime) = 0;

private:
    template<class T
            >
    class Impl;
};

template<class T
        >
class ProcessHandler::Impl : public ProcessHandler
{
public:
    static Ptr create(T* objectPtr, int (T::*methodProcess)(TimeVal), bool (T::*methodNeedsProcess)()) {
        return Ptr(new Impl(objectPtr, methodProcess, methodNeedsProcess));
    }

    virtual void disable() {
        objectPtr = NULL;
    }
    virtual bool isEnabled() {
        return objectPtr != NULL;
    }
    virtual bool needsProcessing() {
        if (objectPtr != NULL) {
            return (objectPtr->*methodNeedsProcess)();
        } else {
            return false;
        }
    }
protected:
    virtual int process(TimeVal endTime) {
        return (objectPtr->*methodProcess)(endTime);
    }
private:

    Impl(T* objectPtr, int (T::*methodProcess)(TimeVal), bool (T::*methodNeedsProcess)())
        : objectPtr(objectPtr), 
          methodProcess(methodProcess), 
          methodNeedsProcess(methodNeedsProcess)
    {}

    WeakPtr<T> objectPtr;
    int (T::*methodProcess)(TimeVal);
    bool (T::*methodNeedsProcess)();
};



} // namespace LucED

#endif // PROCESS_HANDLER_HPP
