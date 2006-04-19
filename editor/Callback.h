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

#ifndef CALLBACK_H
#define CALLBACK_H

#include "HeapObject.h"
#include "ObjectArray.h"

namespace LucED {

/*
 * Namespace for internal implementation details.
 */
namespace CallbackInternal {

//////////////////////////////////////////////////////////////////////////////////////////

class GeneralCallbackBase : public HeapObject
{
public:
    typedef HeapObjectPtr<GeneralCallbackBase> Ptr;

    GeneralCallbackBase() {}
    virtual ~GeneralCallbackBase() {}
    virtual void disable() = 0;
    virtual bool isEnabled() const  = 0;
};

class CallbackBase0 : public GeneralCallbackBase
{
public:
    typedef HeapObjectPtr<CallbackBase0> Ptr;

    virtual ~CallbackBase0() {}
    virtual void call() const = 0;
};

template<class T> class CallbackImpl0 : public CallbackBase0 
{
public:
    static Ptr create(T* objectPtr, void (T::*methodPtr)()) {
        return Ptr(new CallbackImpl0(objectPtr, methodPtr));
    }

    virtual ~CallbackImpl0() {}

    virtual void call() const {
        if (objectPtr != NULL)
            (objectPtr->*methodPtr)();
    }
    void disable() {
        objectPtr = NULL;
    }
    bool isEnabled() const {
        return objectPtr != NULL;
    }
private:
    T* objectPtr;
    void (T::* methodPtr)();
    
    CallbackImpl0(T* objectPtr, void (T::*methodPtr)()) {
        this->objectPtr = objectPtr;
        this->methodPtr = methodPtr;
    }
};


//////////////////////////////////////////////////////////////////////////////////////////

template<class A1> class CallbackBase1 : public GeneralCallbackBase
{
public:
    typedef HeapObjectPtr< CallbackBase1<A1> > Ptr;

    virtual ~CallbackBase1() {}
    virtual void call(A1) const = 0;
};

template<class T, class A1> class CallbackImpl1 : public CallbackBase1<A1> 
{
public:
    typedef HeapObjectPtr< CallbackBase1<A1> > Ptr;

    static Ptr create(T* objectPtr, void (T::*methodPtr)(A1)) {
        return Ptr(new CallbackImpl1(objectPtr, methodPtr));
    }

    virtual ~CallbackImpl1() {}

    virtual void call(A1 a) const {
        if (objectPtr != NULL)
            (objectPtr->*methodPtr)(a);
    }
    void disable() {
        objectPtr = NULL;
    }
    bool isEnabled() const {
        return objectPtr != NULL;
    }
private:
    T* objectPtr;
    void (T::* methodPtr)(A1);
    
    CallbackImpl1(T* objectPtr, void (T::*methodPtr)(A1)) {
        this->objectPtr = objectPtr;
        this->methodPtr = methodPtr;
    }
};


//////////////////////////////////////////////////////////////////////////////////////////

template<class A1, class A2> class CallbackBase2 : public GeneralCallbackBase
{
public:
    typedef HeapObjectPtr< CallbackBase2<A1,A2> > Ptr;

    virtual ~CallbackBase2() {}
    virtual void call(A1, A2) const = 0;
};

template<class T, class A1, class A2> class CallbackImpl2 : public CallbackBase2<A1,A2> 
{
public:
    typedef HeapObjectPtr< CallbackBase2<A1,A2> > Ptr;

    static Ptr create(T* objectPtr, void (T::*methodPtr)(A1,A2)) {
        return Ptr(new CallbackImpl2(objectPtr, methodPtr));
    }

    virtual ~CallbackImpl2() {}

    virtual void call(A1 a1, A2 a2) const {
        if (objectPtr != NULL)
            (objectPtr->*methodPtr)(a1, a2);
    }
    void disable() {
        objectPtr = NULL;
    }
    bool isEnabled() const {
        return objectPtr != NULL;
    }
private:
    T* objectPtr;
    void (T::* methodPtr)(A1,A2);
    
    CallbackImpl2(T* objectPtr, void (T::*methodPtr)(A1,A2)) {
        this->objectPtr = objectPtr;
        this->methodPtr = methodPtr;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////

template<class A1, class A2, class A3> class CallbackBase3 : public GeneralCallbackBase
{
public:
    typedef HeapObjectPtr< CallbackBase3<A1,A2,A3> > Ptr;

    virtual ~CallbackBase3() {}
    virtual void call(A1, A2, A3) const = 0;
};

template<class T, class A1, class A2, class A3> class CallbackImpl3 : public CallbackBase3<A1,A2,A3> 
{
public:
    typedef HeapObjectPtr< CallbackBase3<A1,A2,A3> > Ptr;

    static Ptr create(T* objectPtr, void (T::*methodPtr)(A1,A2,A3)) {
        return Ptr(new CallbackImpl3(objectPtr, methodPtr));
    }

    virtual ~CallbackImpl3() {}

    virtual void call(A1 a1, A2 a2, A3 a3) const {
        if (objectPtr != NULL)
            (objectPtr->*methodPtr)(a1, a2, a3);
    }
    void disable() {
        objectPtr = NULL;
    }
    bool isEnabled() const {
        return objectPtr != NULL;
    }
private:
    T* objectPtr;
    void (T::* methodPtr)(A1,A2,A3);
    
    CallbackImpl3(T* objectPtr, void (T::*methodPtr)(A1,A2,A3)) {
        this->objectPtr = objectPtr;
        this->methodPtr = methodPtr;
    }
};

} // namespace CallbackInternal

//////////////////////////////////////////////////////////////////////////////////////////

/**
 * Callback with 0 Arguments.
 */
class Callback0
{
public:
    Callback0() {}
protected:
    template<class T> Callback0(T* objectPtr, void (T::*methodPtr)()) {
        this->callback = CallbackInternal::CallbackImpl0<T>::create(objectPtr, methodPtr);
    }
    template<class T> Callback0(HeapObjectPtr<T> objectPtr, void (T::*methodPtr)()) {
        this->callback = CallbackInternal::CallbackImpl0<T>::create(objectPtr.getRawPtr(), methodPtr);
    }
public:
    void call() const {
        if (isValid())
            callback->call();
    }
    bool isValid() const {
        return callback.isValid() && callback->isEnabled();
    }
    bool isInvalid() const {
        return !isValid();
    }
    void disable() {
        if (callback.isValid())
            callback->disable();
    }
private:
    CallbackInternal::CallbackBase0::Ptr callback;
};


/**
 * Callback with 1 Arguments.
 */
template<class A1> class Callback1
{
public:
    Callback1() {}
protected:
    template<class T> Callback1(T* objectPtr, void (T::*methodPtr)(A1)) {
        this->callback = CallbackInternal::CallbackImpl1<T, A1>::create(objectPtr, methodPtr);
    }
    template<class T> Callback1(HeapObjectPtr<T> objectPtr, void (T::*methodPtr)(A1)) {
        this->callback = CallbackInternal::CallbackImpl1<T, A1>::create(objectPtr.getRawPtr(), methodPtr);
    }
public:
    void call(A1 a) const {
        if (isValid())
            callback->call(a);
    }
    bool isValid() const {
        return callback.isValid() && callback->isEnabled();
    }
    bool isInvalid() const {
        return !isValid();
    }
    void disable() {
        if (callback.isValid())
            callback->disable();
    }
private:
    typename CallbackInternal::CallbackBase1<A1>::Ptr callback;
};


/**
 * Callback with 2 Arguments.
 */
template<class A1, class A2> class Callback2
{
public:
    Callback2() {}
protected:
    template<class T> Callback2(T* objectPtr, void (T::*methodPtr)(A1,A2)) {
        this->callback = CallbackInternal::CallbackImpl2<T, A1, A2>::create(objectPtr, methodPtr);
    }
    template<class T> Callback2(HeapObjectPtr<T> objectPtr, void (T::*methodPtr)(A1,A2)) {
        this->callback = CallbackInternal::CallbackImpl2<T, A1, A2>::create(objectPtr.getRawPtr(), methodPtr);
    }
public:
    void call(A1 a1, A2 a2) const {
        if (isValid())
            callback->call(a1, a2);
    }
    bool isValid() const {
        return callback.isValid() && callback->isEnabled();
    }
    bool isInvalid() const {
        return !isValid();
    }
    void disable() {
        if (callback.isValid())
            callback->disable();
    }
private:
    typename CallbackInternal::CallbackBase2<A1,A2>::Ptr callback;
};

/**
 * Callback with 3 Arguments.
 */
template<class A1, class A2, class A3> class Callback3
{
public:
    Callback3() {}
protected:
    template<class T> Callback3(T* objectPtr, void (T::*methodPtr)(A1,A2,A3)) {
        this->callback = CallbackInternal::CallbackImpl3<T, A1, A2, A3>::create(objectPtr, methodPtr);
    }
    template<class T> Callback3(HeapObjectPtr<T> objectPtr, void (T::*methodPtr)(A1,A2,A3)) {
        this->callback = CallbackInternal::CallbackImpl3<T, A1, A2, A3>::create(objectPtr.getRawPtr(), methodPtr);
    }
public:
    void call(A1 a1, A2 a2, A3 a3) const {
        if (isValid())
            callback->call(a1, a2, a3);
    }
    bool isValid() const {
        return callback.isValid() && callback->isEnabled();
    }
    bool isInvalid() const {
        return !isValid();
    }
    void disable() {
        if (callback.isValid())
            callback->disable();
    }
private:
    typename CallbackInternal::CallbackBase3<A1,A2,A3>::Ptr callback;
};

//////////////////////////////////////////////////////////////////////////////////////////


} // namespace LucED

#endif // CALLBACK_H
