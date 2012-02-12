/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2012 Oliver Schmidt, oliver at luced dot de
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

#ifndef EVENT_LOOP_HPP
#define EVENT_LOOP_HPP

#include "HeapObject.hpp"
#include "OwningPtr.hpp"
#include "RawPtr.hpp"
#include "ObjectArray.hpp"
#include "FileDescriptorListener.hpp"

namespace LucED
{

class EventLoop : public HeapObject
{
public:
    typedef OwningPtr<EventLoop> Ptr;
    
    static Ptr create() {
        return Ptr(new EventLoop());
    }
    
    void processStep();
    
    void registerFileDescriptorListener(FileDescriptorListener::Ptr listener) {
        fileDescriptorListeners.append(listener);
    }
    void deregisterFileDescriptorListener(RawPtr<FileDescriptorListener> listener) {
        for (int i = 0, n = fileDescriptorListeners.getLength(); i < n; ++i) {
            if (fileDescriptorListeners[i] == listener) {
                fileDescriptorListeners.remove(i);
            }
        }
    }
    
private:
    EventLoop()
    {}
    
    ObjectArray<FileDescriptorListener::Ptr> fileDescriptorListeners;
};

} // namespace LucED

#endif // EVENT_LOOP_HPP
