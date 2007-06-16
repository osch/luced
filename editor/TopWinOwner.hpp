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

#ifndef TOPWINOWNER_H
#define TOPWINOWNER_H

#include "NonCopyable.hpp"
#include "ObjectArray.hpp"
#include "OwningPtr.hpp"

namespace LucED {

class TopWin;


class TopWinOwner : public virtual HeapObject
{
public:
    virtual void requestCloseChildWindow(TopWin *topWin);
    
protected:
    TopWinOwner();

    int getNumberOfChildWindows() const {
        return ownedTopWins.getLength();
    }
    
    TopWin* getChildWindow(int i) {
        return ownedTopWins[i];
    }
    
    TopWin* getLastFocusedOwnedTopWin() const { return lastFocusedOwnedTopWin; }

protected:
    virtual void appendOwnedTopWin(OwningPtr<TopWin> topWin);

private:
    friend class TopWinOwnerAccessForTopWin;
    virtual void reportFocusOwnership(TopWin *topWin);

    void closePendingChilds();
    
    ObjectArray< OwningPtr<TopWin> > ownedTopWins;
    TopWin* lastFocusedOwnedTopWin;
    bool hasRegisteredAsUpdateSource;
    ObjectArray< OwningPtr<TopWin> > toBeClosedTopWins;
};

class TopWinOwnerAccessForTopWin : NonCopyable
{
protected:
    static void reportFocusOwnershipToTopWinOwner(TopWin* topWin, TopWinOwner* owner) {
        owner->reportFocusOwnership(topWin);
    }
    static void appendTopWinToTopWinOwner(OwningPtr<TopWin> topWin, TopWinOwner* owner) {
        owner->appendOwnedTopWin(topWin);
    }
};

} // namespace LucED

#endif // TOPWINOWNER_H
