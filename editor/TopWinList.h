#ifndef TOPWINLIST_H
#define TOPWINLIST_H

#include "HeapObject.h"
#include "SingletonInstance.h"
#include "OwningPtr.h"

namespace LucED {

class TopWin;

class TopWinList : public HeapObject
{
public:
    static TopWinList* getInstance();
    void add(OwningPtr<TopWin> topWin);

private:
    friend class SingletonInstance<TopWinList>;
    
    TopWinList() {}
    
    ObjectArray< OwningPtr<TopWin> > topWins;
};

} // namespace LucED

#include "TopWin.h"

#endif // TOPWINLIST_H
