/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2007 Oliver Schmidt, oliver at luced dot de
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

#ifndef CALLBACK_HPP
#define CALLBACK_HPP

#include "HeapObject.hpp"
#include "OwningPtr.hpp"
#include "WeakPtr.hpp"
#include "EmptyClass.hpp"

namespace LucED
{

////////////////////////////////////////////////////////////////////////////////////////////////
// Internal
////////////////////////////////////////////////////////////////////////////////////////////////

class CallbackBase0 : public HeapObject
{
public:
    typedef OwningPtr<CallbackBase0> Ptr;
    
    virtual void call() const = 0;
    virtual HeapObject* getObjectPtr() const = 0;
    virtual void disable() = 0;
};

template
<
    class T
>
class CallbackImpl0 : public CallbackBase0
{
public:
    typedef OwningPtr<CallbackImpl0> Ptr;

    template
    <
        class S
    >
    static Ptr create(T* objectPtr, void (S::*methodPtr)()) {
        return Ptr(new CallbackImpl0(objectPtr, methodPtr));  
    }
    
    virtual void call() const {
        if (objectPtr.isValid())
            (objectPtr->*methodPtr)();
    }

    virtual HeapObject* getObjectPtr() const {
        return objectPtr;
    }
    
    virtual void disable() {
        objectPtr.invalidate();
    }

private:
    
    CallbackImpl0(T* objectPtr, void (T::*methodPtr)())
        : objectPtr(objectPtr),
          methodPtr(methodPtr)
    {}

    WeakPtr<T> objectPtr;
    void (T::* methodPtr)();
};


////////////////////////////////////////////////////////////////////////////////////////////////
// Internal
////////////////////////////////////////////////////////////////////////////////////////////////

template
<
    class A1
>
class CallbackBase1 : public HeapObject
{
public:
    typedef OwningPtr<CallbackBase1> Ptr;

    virtual void call(A1 a1) const = 0;
    virtual HeapObject* getObjectPtr() const = 0;
    virtual void disable() = 0;
};

template
<
    class T,
    class A1
>
class CallbackImpl1 : public CallbackBase1<A1>
{
public:
    typedef OwningPtr<CallbackImpl1> Ptr;

    template
    <
        class S
    >
    static Ptr create(T* objectPtr, void (S::*methodPtr)(A1)) {
        return Ptr(new CallbackImpl1(objectPtr, methodPtr));  
    }
    
    virtual void call(A1 a1) const {
        if (objectPtr.isValid())
            (objectPtr->*methodPtr)(a1);
    }

    virtual HeapObject* getObjectPtr() const {
        return objectPtr;
    }

    virtual void disable() {
        objectPtr.invalidate();
    }

private:
    
    CallbackImpl1(T* objectPtr, void (T::*methodPtr)(A1))
        : objectPtr(objectPtr),
          methodPtr(methodPtr)
    {}

    WeakPtr<T> objectPtr;
    void (T::* methodPtr)(A1);
};


////////////////////////////////////////////////////////////////////////////////////////////////
// Internal
////////////////////////////////////////////////////////////////////////////////////////////////

template
<
    class A1,
    class A2
>
class CallbackBase2 : public HeapObject
{
public:
    typedef OwningPtr<CallbackBase2> Ptr;

    virtual void call(A1 a1, A2 a2) const = 0;
    virtual HeapObject* getObjectPtr() const = 0;
    virtual void disable() = 0;
};


template
<
    class T,
    class A1,
    class A2
>
class CallbackImpl2 : public CallbackBase2<A1,A2>
{
public:
    typedef OwningPtr<CallbackImpl2> Ptr;

    template
    <
        class S
    >
    static Ptr create(T* objectPtr, void (S::*methodPtr)(A1, A2)) {
        return Ptr(new CallbackImpl2(objectPtr, methodPtr));  
    }
    
    virtual void call(A1 a1, A2 a2) const {
        if (objectPtr.isValid())
            (objectPtr->*methodPtr)(a1, a2);
    }

    virtual HeapObject* getObjectPtr() const {
        return objectPtr;
    }

    virtual void disable() {
        objectPtr.invalidate();
    }

private:
    
    CallbackImpl2(T* objectPtr, void (T::*methodPtr)(A1, A2))
        : objectPtr(objectPtr),
          methodPtr(methodPtr)
    {}

    WeakPtr<T> objectPtr;
    void (T::* methodPtr)(A1 a1, A2 a2);
};


////////////////////////////////////////////////////////////////////////////////////////////////
// Internal
////////////////////////////////////////////////////////////////////////////////////////////////

template
<
    class A1,
    class A2,
    class A3
>
class CallbackBase3 : public HeapObject
{
public:
    typedef OwningPtr<CallbackBase3> Ptr;

    virtual void call(A1 a1, A2 a2, A3 a3) const = 0;
    virtual HeapObject* getObjectPtr() const = 0;
    virtual void disable() = 0;
};


template
<
    class T,
    class A1,
    class A2,
    class A3
>
class CallbackImpl3 : public CallbackBase3<A1,A2,A3>
{
public:
    typedef OwningPtr<CallbackImpl3> Ptr;

    template
    <
        class S
    >
    static Ptr create(T* objectPtr, void (S::*methodPtr)(A1, A2, A3)) {
        return Ptr(new CallbackImpl3(objectPtr, methodPtr));  
    }
    
    virtual void call(A1 a1, A2 a2, A3 a3) const {
        if (objectPtr.isValid())
            (objectPtr->*methodPtr)(a1, a2, a3);
    }

    virtual HeapObject* getObjectPtr() const {
        return objectPtr;
    }

    virtual void disable() {
        objectPtr.invalidate();
    }

private:
    
    CallbackImpl3(T* objectPtr, void (T::*methodPtr)(A1, A2, A3))
        : objectPtr(objectPtr),
          methodPtr(methodPtr)
    {}

    WeakPtr<T> objectPtr;
    void (T::* methodPtr)(A1 a1, A2 a2, A3 a3);
};

////////////////////////////////////////////////////////////////////////////////////////////////
// Internal
////////////////////////////////////////////////////////////////////////////////////////////////

template
<
    class A1 = EmptyClass,
    class A2 = EmptyClass,
    class A3 = EmptyClass
>
class Callback;


template
<
    class Callback
>
class CallbackPtr
{
private:
    typedef typename Callback::ImplPtr ImplPtr;
public:
    CallbackPtr()
    {}

    CallbackPtr(ImplPtr implPtr)
        : impl(implPtr)
    {}

    Callback* operator->() const {
        return &impl;
    }

    bool isValid() const {
        return impl.implPtr.isValid();
    }

    void invalidate() {
        impl.implPtr.invalidate();
    }

private:
    mutable Callback impl;
};

////////////////////////////////////////////////////////////////////////////////////////////////
// Class Callback<>
////////////////////////////////////////////////////////////////////////////////////////////////

template
<
>
class Callback<EmptyClass,EmptyClass,EmptyClass>
{
private:
    typedef CallbackBase0::Ptr ImplPtr;
public:
    typedef CallbackPtr<Callback> Ptr;
    
    template<class T, class S> static Ptr create(T* objectPtr, void (S::*methodPtr)());

    template<class T, class S> static Ptr create(OwningPtr<T> objectPtr, void (S::*methodPtr)());

    void call() const {
        if (implPtr.isValid()) {
            implPtr->call();
        }
    }
    
    bool isEnabled() const {
        return implPtr.isValid() && implPtr->getObjectPtr() != NULL;
    }
    
    void disable() {
        if (implPtr.isValid()) {
            implPtr->disable();
            implPtr.invalidate();
        }
    }
    
    HeapObject* getObjectPtr() const {
        if (implPtr.isValid()) {
            return implPtr->getObjectPtr();
        } else {
            return NULL;
        }
    }

private:
    friend class CallbackPtr<Callback>;

    Callback()
    {}
    
    Callback(ImplPtr implPtr)
        : implPtr(implPtr)
    {}

    ImplPtr implPtr;
};



template
<
    class T, class S
>
inline Callback<EmptyClass,EmptyClass,EmptyClass>::Ptr Callback<EmptyClass,EmptyClass,EmptyClass>::create(T* objectPtr, void (S::*methodPtr)())
{
    return Ptr(CallbackImpl0<T>::create(objectPtr, methodPtr));
}


template
<
    class T, class S
>
inline Callback<EmptyClass,EmptyClass,EmptyClass>::Ptr Callback<EmptyClass,EmptyClass,EmptyClass>::create(OwningPtr<T> objectPtr, void (S::*methodPtr)())
{
    return Ptr(CallbackImpl0<T>::create(objectPtr.getRawPtr(), methodPtr));
}


////////////////////////////////////////////////////////////////////////////////////////////////
// Class Callback<A1>
////////////////////////////////////////////////////////////////////////////////////////////////

template
<
    class A1
>
class Callback<A1,EmptyClass,EmptyClass>
{
private:
    typedef typename CallbackBase1<A1>::Ptr ImplPtr;
public:
    typedef CallbackPtr<Callback> Ptr;
    
    template<class T, class S> static Ptr create(T* objectPtr, void (S::*methodPtr)(A1)) {
        return Ptr(CallbackImpl1<T,A1>::create(objectPtr, methodPtr));
    }
    template<class T, class S> static Ptr create(OwningPtr<T> objectPtr, void (S::*methodPtr)(A1)) {
        return Ptr(CallbackImpl1<T,A1>::create(objectPtr.getRawPtr(), methodPtr));
    }

    void call(A1 a1) const {
        if (implPtr.isValid()) {
            implPtr->call(a1);
        }
    }
    
    bool isEnabled() const {
        return implPtr.isValid() && implPtr->getObjectPtr() != NULL;
    }
    
    void disable() {
        if (implPtr.isValid()) {
            implPtr->disable();
            implPtr.invalidate();
        }
    }
    
    HeapObject* getObjectPtr() const {
        if (implPtr.isValid()) {
            return implPtr->getObjectPtr();
        } else {
            return NULL;
        }
    }

private:
    friend class CallbackPtr<Callback>;

    Callback()
    {}
    
    Callback(ImplPtr implPtr)
        : implPtr(implPtr)
    {}

    ImplPtr implPtr;
};


////////////////////////////////////////////////////////////////////////////////////////////////
// Class Callback<A1,A2>
////////////////////////////////////////////////////////////////////////////////////////////////

template
<
    class A1,
    class A2
>
class Callback<A1,A2,EmptyClass>
{
private:
    typedef typename CallbackBase2<A1,A2>::Ptr ImplPtr;
public:
    typedef CallbackPtr<Callback> Ptr;
    
    template<class T, class S> static Ptr create(T* objectPtr, void (S::*methodPtr)(A1,A2)) {
        return Ptr(CallbackImpl2<T,A1,A2>::create(objectPtr, methodPtr));
    }
    template<class T, class S> static Ptr create(OwningPtr<T> objectPtr, void (S::*methodPtr)(A1,A2)) {
        return Ptr(CallbackImpl2<T,A1,A2>::create(objectPtr.getRawPtr(), methodPtr));
    }

    void call(A1 a1, A2 a2) const {
        if (implPtr.isValid()) {
            implPtr->call(a1, a2);
        }
    }
    
    bool isEnabled() const {
        return implPtr.isValid() && implPtr->getObjectPtr() != NULL;
    }
    
    void disable() {
        if (implPtr.isValid()) {
            implPtr->disable();
            implPtr.invalidate();
        }
    }
    
    HeapObject* getObjectPtr() const {
        if (implPtr.isValid()) {
            return implPtr->getObjectPtr();
        } else {
            return NULL;
        }
    }

private:
    friend class CallbackPtr<Callback>;

    Callback()
    {}
    
    Callback(ImplPtr implPtr)
        : implPtr(implPtr)
    {}

    ImplPtr implPtr;
};


////////////////////////////////////////////////////////////////////////////////////////////////
// Class Callback<A1,A2>
////////////////////////////////////////////////////////////////////////////////////////////////

template
<
    class A1,
    class A2,
    class A3
>
class Callback
{
private:
    typedef typename CallbackBase3<A1,A2,A3>::Ptr ImplPtr;
public:
    typedef CallbackPtr<Callback> Ptr;
    
    template<class T, class S> static Ptr create(T* objectPtr, void (S::*methodPtr)(A1,A2,A3)) {
        return Ptr(CallbackImpl3<T,A1,A2,A3>::create(objectPtr, methodPtr));
    }
    template<class T, class S> static Ptr create(OwningPtr<T> objectPtr, void (S::*methodPtr)(A1,A2,A3)) {
        return Ptr(CallbackImpl3<T,A1,A2,A3>::create(objectPtr.getRawPtr(), methodPtr));
    }

    void call(A1 a1, A2 a2, A3 a3) const {
        if (implPtr.isValid()) {
            implPtr->call(a1, a2, a3);
        }
    }
    
    bool isEnabled() const {
        return implPtr.isValid() && implPtr->getObjectPtr() != NULL;
    }
    
    void disable() {
        if (implPtr.isValid()) {
            implPtr->disable();
            implPtr.invalidate();
        }
    }
    
    HeapObject* getObjectPtr() const {
        if (implPtr.isValid()) {
            return implPtr->getObjectPtr();
        } else {
            return NULL;
        }
    }

    
private:
    friend class CallbackPtr<Callback>;
    
    Callback()
    {}
    
    Callback(ImplPtr implPtr)
        : implPtr(implPtr)
    {}

    ImplPtr implPtr;
};


////////////////////////////////////////////////////////////////////////////////////////////////
// Internal
////////////////////////////////////////////////////////////////////////////////////////////////

template
<
    class X,
    class Y,
    class Z
>
X newCallback(Y objectPtr, Z methodPtr);


////////////////////////////////////////////////////////////////////////////////////////////////
// Function newCallback()
////////////////////////////////////////////////////////////////////////////////////////////////

template
<
    class T, class S
>
typename Callback<>::Ptr newCallback(OwningPtr<T> objectPtr, void (S::*methodPtr)()) {
    return Callback<>::create(objectPtr, methodPtr);
}

template
<
    class T, class S
>
typename Callback<>::Ptr newCallback(T* objectPtr, void (S::*methodPtr)()) {
    return Callback<>::create(objectPtr, methodPtr);
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Function newCallback()
////////////////////////////////////////////////////////////////////////////////////////////////

template
<
    class T, class S,
    class A1
>
typename Callback<A1>::Ptr newCallback(OwningPtr<T> objectPtr, void (S::*methodPtr)(A1)) {
    return Callback<A1>::create(objectPtr, methodPtr);
}

template
<
    class T, class S,
    class A1
>
typename Callback<A1>::Ptr newCallback(T* objectPtr, void (S::*methodPtr)(A1)) {
    return Callback<A1>::create(objectPtr, methodPtr);
}


////////////////////////////////////////////////////////////////////////////////////////////////
// Function newCallback()
////////////////////////////////////////////////////////////////////////////////////////////////

template
<
    class T, class S,
    class A1,
    class A2
>
typename Callback<A1, A2>::Ptr newCallback(OwningPtr<T> objectPtr, void (S::*methodPtr)(A1,A2)) {
    return Callback<A1, A2>::create(objectPtr, methodPtr);
}

template
<
    class T, class S,
    class A1,
    class A2
>
typename Callback<A1, A2>::Ptr newCallback(T* objectPtr, void (S::*methodPtr)(A1,A2)) {
    return Callback<A1, A2>::create(objectPtr, methodPtr);
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Function newCallback()
////////////////////////////////////////////////////////////////////////////////////////////////

template
<
    class T, class S,
    class A1,
    class A2,
    class A3
>
typename Callback<A1, A2, A3>::Ptr newCallback(OwningPtr<T> objectPtr, void (S::*methodPtr)(A1,A2,A3)) {
    return Callback<A1, A2, A3>::create(objectPtr, methodPtr);
}

template
<
    class T, class S,
    class A1,
    class A2,
    class A3
>
typename Callback<A1, A2, A3>::Ptr newCallback(T* objectPtr, void (S::*methodPtr)(A1,A2,A3)) {
    return Callback<A1, A2, A3>::create(objectPtr, methodPtr);
}


} // namespace LucED

#endif // CALLBACK_HPP
