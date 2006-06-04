#ifndef SINGLETONINSTANCE_H
#define SINGLETONINSTANCE_H

#include "debug.h"
#include "WeakPtr.h"
#include "SingletonKeeper.h"
#include "OwningPtr.h"

namespace LucED {

template<class T> class SingletonInstance : NonCopyable
{
public:

    SingletonInstance() : wasInstantiated(false) {}

    T* getPtr()
    {
        if (!wasInstantiated)
        {
            instancePtr = SingletonKeeper::getInstance()->add(OwningPtr<T>(new T()));
            wasInstantiated = true;
        } 
        else 
        {
            ASSERT(instancePtr.isValid());
        }
        return instancePtr.getRawPtr();
    }

private:
    bool wasInstantiated;
    WeakPtr<T> instancePtr;
};

} // namespace LucED

#endif // SINGLETONINSTANCE_H
