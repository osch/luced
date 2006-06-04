
#include "TopWinList.h"

using namespace LucED;

static SingletonInstance<TopWinList> instance;

TopWinList* TopWinList::getInstance()
{
    return instance.getPtr();
}

void TopWinList::add(OwningPtr<TopWin> topWin)
{
    topWins.append(topWin);
}


