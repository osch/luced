#ifndef WEAKPTR_H
#define WEAKPTR_H

#include "HeapObject.h"
#include "OwningPtr.h"

namespace LucED {


template<class T> class WeakPtr : private HeapObjectRefManipulator
{
public:
    
    WeakPtr() : ptr(NULL) {
    }
    
    ~WeakPtr() {
        decWeakCounter(ptr);
    }
    
    WeakPtr(const WeakPtr& src) {
        ptr = src.ptr;
        incWeakCounter(ptr);
    }
    
    template<class S> WeakPtr(const WeakPtr<S>& src) {
        ptr = src.getRawPtr();
        incWeakCounter(ptr);
    }
    
    template<class S> WeakPtr(const OwningPtr<S>& src) {
        ptr = src.getRawPtr();
        incWeakCounter(ptr);
    }
    
    WeakPtr& operator=(const WeakPtr& src) {
        if (this != &src) {
            decWeakCounter(ptr);
            ptr = src.ptr;
            incWeakCounter(ptr);
        }
        return *this;
    }
    
    template<class S> WeakPtr& operator=(const WeakPtr<S>& src) {
        if (this != &src) {
            decWeakCounter(ptr);
            ptr = src.ptr;
            incWeakCounter(ptr);
        }
        return *this;
    }
    
    template<class S> WeakPtr& operator=(const OwningPtr<S>& src) {
        decWeakCounter(ptr);
        ptr = src.getRawPtr();
        incWeakCounter(ptr);
        return *this;
    }
    
    void invalidate() {
        decWeakCounter(ptr);
        ptr = NULL;
    }
    
    bool isValid() {
        checkOwningReferences();
        return ptr != NULL;
    }
    
    bool isInvalid() {
        checkOwningReferences();
        return ptr == NULL;
    }
    
    T* operator->() {
        checkOwningReferences();
        return ptr;
    }
    
    T* getRawPtr() {
        checkOwningReferences();
        return ptr;
    }

    T* getRawPtr() const {
        return ptr;
    }
    
    bool operator==(const WeakPtr& rhs) const {
        return ptr == rhs.ptr;
    }
    
    template<class S> bool operator==(const WeakPtr<S>& rhs) const {
        return ptr == rhs.ptr;
    }
    
private:

    void checkOwningReferences() {
        if (ptr != NULL && !hasOwningReferences(ptr)) {
            invalidate();
        }
    }
    
    T *ptr;
    
};


} // namespace LucED

#endif // WEAKPTR_H
