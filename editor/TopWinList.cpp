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

#include "EventDispatcher.hpp"
#include "TopWinList.hpp"

using namespace LucED;

WeakPtr<TopWinList> TopWinList::instance;


namespace // anonymous namespace
{


} // anonymous namespace

TopWinList* TopWinList::getInstance()
{
    if (instance.isInvalid())
    {
        LucED::OwningPtr<TopWinList> p = LucED::OwningPtr<TopWinList>(new TopWinList());
                                     p->emptyChecker = EmptyChecker::create(p);

        EventDispatcher::getInstance()->registerRunningComponent(p);
        EventDispatcher::getInstance()->registerUpdateSource(newCallback(p->emptyChecker, &EmptyChecker::check));

        instance = p;
    }
    return instance;
}


void TopWinList::requestCloseChildWindow(TopWin* topWin)
{
    TopWinOwner::requestCloseChildWindow(topWin);
    
    if (getNumberOfChildWindows() == 0)
    {
        EventDispatcher::getInstance()->deregisterRunningComponent(this);
        instance.invalidate();
    }
}


void TopWinList::EmptyChecker::check()
{
    if (topWinList.isValid()) {
        topWinList->checkIfEmpty();
    }
}

void TopWinList::checkIfEmpty()
{
    if (getNumberOfChildWindows() == 0)
    {
        EventDispatcher::getInstance()->deregisterRunningComponent(this);
        instance.invalidate();
    }
    emptyChecker.invalidate();
}

