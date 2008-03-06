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

#include "OwnedTopWins.hpp"
#include "TopWin.hpp"
#include "EventDispatcher.hpp"

using namespace LucED;

      
OwnedTopWins::OwnedTopWins()
    : hasRegisteredAsUpdateSource(false)
{}

void OwnedTopWins::requestCloseChildWindow(TopWin* topWin)
{
    for (int i = 0; i < ownedTopWins.getLength(); ++i)
    {
        if (ownedTopWins[i].getRawPtr() == topWin) {
            //ownedTopWins[i]->hide(); caused problems here (sometimes missing exposure events), probably a bug in the X11-server
            toBeClosedTopWins.append(ownedTopWins[i]);
            ownedTopWins.remove(i);
            if (!hasRegisteredAsUpdateSource) {
                EventDispatcher::getInstance()->registerUpdateSource(newCallback(this, &OwnedTopWins::closePendingChilds));
                hasRegisteredAsUpdateSource = true;
            }
            break;
        }
    }
}

void OwnedTopWins::closePendingChilds()
{
    toBeClosedTopWins.clear();
    hasRegisteredAsUpdateSource = false;
    EventDispatcher::getInstance()->deregisterAllUpdateSourceCallbacksFor(this);
}

void OwnedTopWins::appendOwnedTopWin(OwningPtr<TopWin> topWin)
{
    ownedTopWins.append(topWin);
    notifyAboutNewOwnedTopWinCallbacks.invokeAllCallbacks(topWin);
}

