
#include "WeakPtr.h"
#include "SingletonKeeper.h"

using namespace LucED;

static WeakPtr<SingletonKeeper> instance;

SingletonKeeper::Ptr SingletonKeeper::create()
{
    ASSERT(instance.isInvalid());
    Ptr rslt(new SingletonKeeper());
    instance = rslt;
    return rslt;
}

SingletonKeeper* SingletonKeeper::getInstance()
{
    ASSERT(instance.isValid());
    return instance.getRawPtr();
}


SingletonKeeper::~SingletonKeeper()
{
    // desctruct last singleton first
    while(singletons.getLength() > 0) {
        singletons.remove(singletons.getLength() - 1);
    }
}
