#ifndef SINGLETONKEEPER_H
#define SINGLETONKEEPER_H

#include "debug.h"
#include "HeapObject.h"
#include "ObjectArray.h"
#include "OwningPtr.h"

namespace LucED {

class SingletonKeeper : public HeapObject
{
public:
    typedef OwningPtr<SingletonKeeper>     Ptr;
    
    static Ptr create();
    
    static SingletonKeeper* getInstance();
    
    template<class T> WeakPtr<T> add(OwningPtr<T> singletonPtr) {
        singletons.append(singletonPtr);
        return singletonPtr;
    }
    
private:
    SingletonKeeper() {}
    ~SingletonKeeper();

    ObjectArray< OwningPtr<HeapObject> > singletons;
};


} // namespace LucED

#endif // SINGLETONKEEPER_H
