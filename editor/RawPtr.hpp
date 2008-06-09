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

#ifndef RAW_PTR_HPP
#define RAW_PTR_HPP

#include "WeakPtr.hpp"

namespace LucED
{

/**
 * Pointer that should stay valid, once it has been assigned to a
 * valid HeapObject.
 *
 * Use this class if you are shure that the ownership of the
 * underlaying HeapObject is to be preserverd somewhere else.
 *
 * In DEBUG version this is checked through a WeakPtr.
 * In NON-DEBUG version it is simply a raw pointer.
 */
template<class T> class RawPtr
{
public:
    
    RawPtr() : ptr(NULL)
    {
    #ifdef DEBUG
        isValidFlag = false;
    #endif
    }
    
    RawPtr(const RawPtr& src) 
        : ptr(src.ptr)
    {
    #ifdef DEBUG
        isValidFlag = src.isValidFlag;
        ASSERT(isValidFlag == ptr.isValid());
    #endif
    }
    
    template<class S> RawPtr(const RawPtr<S>& src)
        : ptr(src.ptr)
    {
    #ifdef DEBUG
        isValidFlag = src.isValidFlag;
        ASSERT(isValidFlag == ptr.isValid());
    #endif
    }
    
    template<class PtrClass> RawPtr(const PtrClass& src)
        : ptr(src)
    {
    #ifdef DEBUG
        isValidFlag = ptr.isValid();
    #endif
    }
    
    RawPtr& operator=(const RawPtr& src)
    {
        ptr = src.ptr;
    #ifdef DEBUG
        isValidFlag = src.isValidFlag;
        ASSERT(isValidFlag == ptr.isValid());
    #endif
        return *this;
    }
    
    template<class S> RawPtr& operator=(const RawPtr<S>& src)
    {
        ptr = src.ptr;
    #ifdef DEBUG
        isValidFlag = src.isValidFlag;
        ASSERT(isValidFlag == ptr.isValid());
    #endif
        return *this;
    }
    
    template<class PtrClass> RawPtr& operator=(const PtrClass& src)
    {
        ptr = src;
    #ifdef DEBUG
        isValidFlag = ptr.isValid();
    #endif
        return *this;
    }
    
    bool isValid() const {
    #ifdef DEBUG
        ASSERT(isValidFlag == ptr.isValid());
        return isValidFlag;
    #else
        return ptr != NULL;
    #endif
    }
    
    template<class S> bool operator==(const RawPtr<S>& rhs) const {
        return ptr == rhs.ptr;
    }
    template<class PtrClass> bool operator==(const PtrClass& rhs) const {
        return ptr == rhs;
    }
    template<class PtrClass> bool operator!=(const PtrClass& rhs) const {
        return !(*this == rhs);
    }
    
    operator T*() const {
        return getRawPtr();
    }
    
    T* operator->() const {
        ASSERT(getRawPtr() != NULL);
        return getRawPtr();
    }
    
    T* getRawPtr() const {
        ASSERT(isValid());
        return ptr;
    }

    operator WeakPtr<T>() const {
        return WeakPtr<T>(ptr);
    }

    template
    <
        class S
    > 
    operator WeakPtr<S>() const {
        return WeakPtr<S>(ptr);
    }

    void invalidate() {
    #ifdef DEBUG
        ASSERT(isValidFlag == ptr.isValid());
        ptr.invalidate();
        isValidFlag = false;
    #else
        ptr = NULL;
    #endif
    }

private:

#ifdef DEBUG
    WeakPtr<T> ptr;
    bool       isValidFlag;
#else
    T* ptr;
#endif    
};


} // namespace LucED

#endif // RAW_PTR_HPP
