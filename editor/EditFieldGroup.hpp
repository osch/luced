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

#ifndef EDITFIELDGROUP_H
#define EDITFIELDGROUP_H

#include "HeapObject.hpp"
#include "ObjectArray.hpp"
#include "OwningPtr.hpp"

namespace LucED
{

class EditFieldGroup : public HeapObject
{
public:
    typedef OwningPtr<EditFieldGroup> Ptr;
    
    static Ptr create() {
        return Ptr(new EditFieldGroup());
    }
    
    void registerCursorFocusLostHandler(const Callback0& handleCursorFocusLost) {
        listeners.append(handleCursorFocusLost);
    }
    
    void invokeAllCursorFocusLostExceptFor(HeapObject* caller) {
        for (int i = 0; i < listeners.getLength();) {
            if (!listeners[i].isValid()) {
                listeners.remove(i);
            } else {
                if (listeners[i].getObjectPtr() != caller) {
                    listeners[i].call();
                }
                ++i;
            }
        }
        lastFocusObject = caller;
    }
    
    HeapObject* getLastFocusObject() const {
        return lastFocusObject;
    }
    
private:
    ObjectArray< Callback0 > listeners;
    WeakPtr<HeapObject> lastFocusObject;
};

} // namespace LucED

#endif // EDITFIELDGROUP_H
