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

#ifndef TOP_WIN_LIST_HPP
#define TOP_WIN_LIST_HPP

#include "HeapObject.hpp"
#include "WeakPtr.hpp"
#include "TopWin.hpp"
#include "OwnedTopWins.hpp"
#include "RunningComponent.hpp"

namespace LucED
{

class TopWin;

class TopWinList : public RunningComponent
{
public:
    static TopWinList* getInstance();
    
    int getNumberOfTopWins() const {
        return ownedTopWins->getNumberOfTopWins();
    }
    
    TopWin* getTopWin(int i) {
        return ownedTopWins->getTopWin(i);
    }

private:
    static LucED::WeakPtr<TopWinList> instance;

    class EmptyChecker : public HeapObject
    {
    public:
        typedef LucED::OwningPtr<EmptyChecker> Ptr;

        static Ptr create(TopWinList* list) {
            return Ptr(new EmptyChecker(list));
        }

        void check();
        
    private:
        EmptyChecker(TopWinList* list)
            : topWinList(list)
        {}
        LucED::WeakPtr<TopWinList> topWinList;
    };
  
    TopWinList() 
        : ownedTopWins(OwnedTopWins::create())
    {
        ownedTopWins->registerNewOwnedTopWinNotifyCallback(newCallback(this, &TopWinList::notifyAboutNewOwnedTopWin));
    }
    
    void notifyAboutNewOwnedTopWin(TopWin* topWin);
    
    friend class TopWin;
    ValidPtr<OwnedTopWins> getOwnedTopWins() {
        return ownedTopWins;
    }

    void notifyRequestCloseChildWindow(TopWin* topWin);

    void checkIfEmpty();
    
    EmptyChecker::Ptr emptyChecker;

    OwnedTopWins::Ptr ownedTopWins;
};

} // namespace LucED

#endif // TOP_WIN_LIST_HPP
