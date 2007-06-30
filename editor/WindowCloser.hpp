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

#ifndef WINDOW_CLOSER_HPP
#define WINDOW_CLOSER_HPP

#include "HeapObject.hpp"
#include "RunningComponent.hpp"
#include "EventDispatcher.hpp"
#include "EditorTopWin.hpp"

namespace LucED
{

class WindowCloser : public RunningComponent
{
public:
    typedef LucED::OwningPtr<WindowCloser> OwningPtr;
    typedef LucED::WeakPtr  <WindowCloser> WeakPtr;

    static WeakPtr start()
    {
        if (instance.isValid()) {
            instance->closeWindows();
            return instance;
        } else {
            OwningPtr ptr(new WindowCloser());
            EventDispatcher::getInstance()->registerRunningComponent(ptr);
            instance = ptr;
            ptr->closeWindows();
            return ptr;
        }
    }
    
private:

    WindowCloser()
    {}
    
    void closeWindows();
    
    void handleSaveFileButton();
    void handleDiscardButton();
    void handleCancelButton();
    
    static WeakPtr    instance;

    EditorTopWin::Ptr referingTopWin;
};

} // namespace LucED

#endif // WINDOW_CLOSER_HPP
