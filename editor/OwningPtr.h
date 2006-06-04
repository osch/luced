#ifndef OWNINGPTR_H
#define OWNINGPTR_H

#include "HeapObject.h"

namespace LucED {

template<class T> class OwningPtr : private HeapObjectRefManipulator
{
public:
    
    OwningPtr(T *ptr = NULL) {
        this->ptr = ptr;
        incRefCounter(ptr);
    }
    
    ~OwningPtr() {
        decRefCounter(ptr);
    }
    
    OwningPtr(const OwningPtr& src) {
        ptr = src.ptr;
        incRefCounter(ptr);
    }
    
    template<class S> OwningPtr(const OwningPtr<S>& src) {
        ptr = src.getRawPtr();
        incRefCounter(ptr);
    }
    
    OwningPtr& operator=(const OwningPtr& src) {
        if (this != &src) {
            decRefCounter(ptr);
            ptr = src.ptr;
            incRefCounter(ptr);
        }
        return *this;
    }
    
    template<class S> OwningPtr& operator=(const OwningPtr<S>& src) {
        if (this != &src) {
            decRefCounter(ptr);
            ptr = src.ptr;
            incRefCounter(ptr);
        }
    }
    
    void invalidate() {
        decRefCounter(ptr);
        ptr = NULL;
    }
    
    bool isValid() const {
        return ptr != NULL;
    }
    
    bool isInvalid() const {
        return ptr == NULL;
    }
    
    T* operator->() const {
        return ptr;
    }
    
    T* getRawPtr() const {
        return ptr;
    }
    
    bool operator==(const OwningPtr& rhs) const {
        return ptr == rhs.ptr;
    }
    
    template<class S> bool operator==(const OwningPtr<S>& rhs) const {
        return ptr == rhs.ptr;
    }
    
private:
    
    T *ptr;
    
};


} // namespace LucED

#endif // OWNINGPTR_H
