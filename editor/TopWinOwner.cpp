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

#include "TopWinOwner.hpp"
#include "TopWin.hpp"
#include "EventDispatcher.hpp"

using namespace LucED;

      
TopWinOwner::TopWinOwner()
    : lastFocusedOwnedTopWin(NULL),
      hasRegisteredAsUpdateSource(false)
{}

void TopWinOwner::requestCloseChildWindow(TopWin* topWin)
{
    for (int i = 0; i < ownedTopWins.getLength(); ++i)
    {
        if (ownedTopWins[i].getRawPtr() == topWin) {
            ownedTopWins[i]->hide();
            toBeClosedTopWins.append(ownedTopWins[i]);
            ownedTopWins.remove(i);
            if (!hasRegisteredAsUpdateSource) {
                EventDispatcher::getInstance()->registerUpdateSource(Callback0(this, &TopWinOwner::closePendingChilds));
                hasRegisteredAsUpdateSource = true;
            }
            if (lastFocusedOwnedTopWin == topWin) {
                lastFocusedOwnedTopWin = NULL;
            }
            break;
        }
    }
}

void TopWinOwner::reportFocusOwnership(TopWin* topWin)
{
    lastFocusedOwnedTopWin = topWin;
}

void TopWinOwner::closePendingChilds()
{
    toBeClosedTopWins.clear();
    hasRegisteredAsUpdateSource = false;
    EventDispatcher::getInstance()->deregisterAllUpdateSourceCallbacksFor(this);
}

void TopWinOwner::appendOwnedTopWin(OwningPtr<TopWin> topWin)
{
    ownedTopWins.append(topWin);
}

