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

#ifndef TOP_WIN_LIST_HPP
#define TOP_WIN_LIST_HPP

#include "HeapObject.hpp"
#include "WeakPtr.hpp"
#include "TopWin.hpp"
#include "TopWinOwner.hpp"
#include "RunningComponent.hpp"

namespace LucED {

class TopWin;

class TopWinList : public TopWinOwner,
                   public RunningComponent
{
public:
    static TopWinList* getInstance();
    
    virtual void requestCloseChildWindow(TopWin *topWin);

    int getNumberOfTopWins() const {
        return getNumberOfChildWindows();
    }
    
    TopWin* getTopWin(int i) {
        return getChildWindow(i);
    }

private:
    static LucED::WeakPtr<TopWinList> instance;
  
    TopWinList() {}
};

} // namespace LucED

#endif // TOP_WIN_LIST_HPP
