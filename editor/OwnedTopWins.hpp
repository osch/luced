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

#ifndef OWNED_TOP_WINS_HPP
#define OWNED_TOP_WINS_HPP

#include "NonCopyable.hpp"
#include "ObjectArray.hpp"
#include "OwningPtr.hpp"
#include "CallbackContainer.hpp"

namespace LucED
{

class TopWin;


class OwnedTopWins : public HeapObject
{
public:
    typedef OwningPtr<OwnedTopWins> Ptr;
    
    static Ptr create() {
        return Ptr(new OwnedTopWins());
    }
    
    int getNumberOfTopWins() const {
        return ownedTopWins.getLength();
    }
    
    TopWin* getTopWin(int i) {
        return ownedTopWins[i];
    }
    
    void requestCloseChildWindow(TopWin* topWin);
    void appendOwnedTopWin(OwningPtr<TopWin> topWin);

    void registerNewOwnedTopWinNotifyCallback(Callback<TopWin*>::Ptr callback) {
        notifyAboutNewOwnedTopWinCallbacks.registerCallback(callback);
    }
    
    class AccessForTopWin
    {
        friend class TopWin;

        static void appendTopWinToOwnedTopWins(OwningPtr<TopWin> topWin, OwnedTopWins* owner);
    };

private:
    OwnedTopWins();

    void closePendingChilds();
    
    ObjectArray< OwningPtr<TopWin> > ownedTopWins;
    bool hasRegisteredAsUpdateSource;
    ObjectArray< OwningPtr<TopWin> > toBeClosedTopWins;
    
    CallbackContainer<TopWin*> notifyAboutNewOwnedTopWinCallbacks;
};


} // namespace LucED

#include "TopWin.hpp"

namespace LucED
{

inline void OwnedTopWins::AccessForTopWin::appendTopWinToOwnedTopWins(OwningPtr<TopWin> topWin,
                                                                      OwnedTopWins*     owner)
{
    owner->appendOwnedTopWin(topWin);
}

} // namespace LucED


#endif // OWNED_TOP_WINS_HPP
