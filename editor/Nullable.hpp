/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2009 Oliver Schmidt, oliver at luced dot de
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

#ifndef NULLABLE_HPP
#define NULLABLE_HPP

#include <new>

#include "debug.hpp"
#include "Null.hpp"

namespace LucED
{

template
<
    class V
>
class Nullable
{
public:
    typedef V ValueType;
    
    Nullable()           : valid(false) {}
    Nullable(const V& v) : valid(true)  { new(&value) V(v); }
    
    ~Nullable() {
        if (valid) {
            ((V*)&(value))->~V();
        }
    }
    
    Nullable(const NullType& nullValue) : valid(false)
    {}
    
    Nullable(const Nullable& rhs) : valid (rhs.valid) {
        ASSERT(this != &rhs);
        if (valid) {
            new(&value) V(*(const V*)(&rhs.value));
        }
    }
    
    template<class V2
            >
    Nullable(const Nullable<V2>& rhs) : valid (rhs.valid) {
        ASSERT(this != &rhs);
        if (valid) {
            new(&value) V(*(const V2*)(&rhs.value));
        }
    }
    
    Nullable& operator=(const Nullable& rhs) {
        if (this != &rhs) {
            if (valid && rhs.valid)
            {
                *((V*)&(value)) = *(const V*)(&rhs.value);
            }
            else
            {
                if (valid) {
                    ((V*)&(value))->~V();
                }
                valid = rhs.valid;
                if (valid) {
                    new(&value) V(*(const V*)(&rhs.value));
                }
            }
        }
        return *this;
    }
    
    template<class V2
            >
    Nullable& operator=(const Nullable<V2>& rhs) {
        if (this != &rhs) {
            if (valid && rhs.valid)
            {
                *((V*)&(value)) = *(const V2*)(&rhs.value);
            }
            else
            {
                if (valid) {
                    ((V*)&(value))->~V();
                }
                valid = rhs.valid;
                if (valid) {
                    new(&value) V(*(const V2*)(&rhs.value));
                }
            }
        }
        return *this;
    }

    bool     isValid() const { return valid; }
    bool     isNull()  const { return !valid; }
    operator V()       const { ASSERT(valid); return *(const V*)(&value); }
    const V& get()     const { ASSERT(valid); return *(const V*)(&value); }

    bool operator!=(NullType null) const { return !isNull(); }
    bool operator==(NullType null) const { return  isNull(); }

    bool operator!=(const ValueType& rhs) const { return !isValid() || get() != rhs; }
    bool operator==(const ValueType& rhs) const { return  isValid() && get() == rhs; }

    bool operator!=(const Nullable<ValueType>& rhs) const { return !(isNull() && rhs.isNull()) && !(isValid() && rhs.isValid() && get() == rhs.get()); }
    bool operator==(const Nullable<ValueType>& rhs) const { return  (isNull() && rhs.isNull()) ||  (isValid() && rhs.isValid() && get() == rhs.get()); }

private:
    struct { char data[sizeof(V)]; } value;
    bool valid;
};


template<class T> class OwningPtr;
template<class T> class WeakPtr;
template<class T> class RawPtr;


template
<
    class T
>
class Nullable< OwningPtr<T> >
{
public:
    typedef OwningPtr<T> ValueType;

    Nullable() {}
    Nullable(const OwningPtr<T>& v) : ptr(v) {}
    
    Nullable(const Nullable& rhs)
        : ptr(rhs.ptr)
    {
        ASSERT(this != &rhs);
    }

    bool         isValid()      const { return ptr.isValid(); }
    bool         isNull()       const { return !ptr.isValid(); }
    operator     OwningPtr<T>() const { return ptr; }
    OwningPtr<T> get()          const { return ptr; }

    bool operator!=(NullType null) const { return !isNull(); }
    bool operator==(NullType null) const { return  isNull(); }

    bool operator!=(const ValueType& rhs) const { return !isValid() || get() != rhs; }
    bool operator==(const ValueType& rhs) const { return  isValid() && get() == rhs; }

    bool operator!=(const Nullable<ValueType>& rhs) const { return !(isNull() && rhs.isNull()) && !(isValid() && rhs.isValid() && get() == rhs.get()); }
    bool operator==(const Nullable<ValueType>& rhs) const { return  (isNull() && rhs.isNull()) ||  (isValid() && rhs.isValid() && get() == rhs.get()); }

private:
    OwningPtr<T> ptr;
};

template
<
    class T
>
class Nullable< WeakPtr<T> >
{
public:
    typedef WeakPtr<T> ValueType;

    Nullable() {}
    Nullable(const WeakPtr<T>& v) : ptr(v) {}

    Nullable(const Nullable& rhs)
        : ptr(rhs.ptr)
    {
        ASSERT(this != &rhs);
    }

    bool       isValid()      const { return ptr.isValid(); }
    bool       isNull()       const { return !ptr.isValid(); }
    operator   WeakPtr<T>()   const { return ptr; }
    WeakPtr<T> get()          const { return ptr; }
    
    bool operator!=(NullType null) const { return !isNull(); }
    bool operator==(NullType null) const { return  isNull(); }

    bool operator!=(const ValueType& rhs) const { return !isValid() || get() != rhs; }
    bool operator==(const ValueType& rhs) const { return  isValid() && get() == rhs; }

    bool operator!=(const Nullable<ValueType>& rhs) const { return !(isNull() && rhs.isNull()) && !(isValid() && rhs.isValid() && get() == rhs.get()); }
    bool operator==(const Nullable<ValueType>& rhs) const { return  (isNull() && rhs.isNull()) ||  (isValid() && rhs.isValid() && get() == rhs.get()); }

private:
    WeakPtr<T> ptr;
};


template
<
    class T
>
class Nullable< RawPtr<T> >
{
public:
    typedef RawPtr<T> ValueType;

    Nullable() {}
    Nullable(const RawPtr<T>& v) : ptr(v) {}

    Nullable(const Nullable& rhs)
        : ptr(rhs.ptr)
    {
        ASSERT(this != &rhs);
    }

    bool      isValid()     const { return ptr.isValid(); }
    bool      isNull()      const { return !ptr.isValid(); }
    operator  RawPtr<T>()   const { return ptr; }
    RawPtr<T> get()         const { return ptr; }

    bool operator!=(NullType null) const { return !isNull(); }
    bool operator==(NullType null) const { return  isNull(); }

    bool operator!=(const ValueType& rhs) const { return !isValid() || get() != rhs; }
    bool operator==(const ValueType& rhs) const { return  isValid() && get() == rhs; }

    bool operator!=(const Nullable<ValueType>& rhs) const { return !(isNull() && rhs.isNull()) && !(isValid() && rhs.isValid() && get() == rhs.get()); }
    bool operator==(const Nullable<ValueType>& rhs) const { return  (isNull() && rhs.isNull()) ||  (isValid() && rhs.isValid() && get() == rhs.get()); }

private:
    RawPtr<T> ptr;
};

} // namespace LucED

#endif // NULLABLE_HPP
