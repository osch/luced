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

#ifndef FILE_DESCRIPTOR_LISTENER_HPP
#define FILE_DESCRIPTOR_LISTENER_HPP

#include <unistd.h>

#include "HeapObject.hpp"
#include "OwningPtr.hpp"
#include "Callback.hpp"

namespace LucED
{

class FileDescriptorListener : public HeapObject
{
public:
    typedef OwningPtr<FileDescriptorListener> Ptr;

    static Ptr create(int fileDescriptor,
                      Callback<int>::Ptr readCallback, 
                      Callback<int>::Ptr writeCallback)
    {
        Ptr rslt = Ptr(new FileDescriptorListener());
        rslt->fileDescriptor = fileDescriptor;
        rslt->readCallback = readCallback;
        rslt->writeCallback = writeCallback;
        return rslt;
    }
    
    ~FileDescriptorListener() { 
        if (fileDescriptor != -1) {
            ::close(fileDescriptor);
        }
    }
    
    bool isActive() const {
        return readCallback.isEnabled() || writeCallback.isEnabled();
    }
    
    bool isWaitingForRead() const {
        return readCallback.isEnabled();
    }
    
    bool isWaitingForWrite() const {
        return writeCallback.isEnabled();
    }
    
    int getFileDescriptor() const {
        return fileDescriptor;
    }

    void handleReading() {
        if (fileDescriptor != -1) {
            readCallback->call(fileDescriptor);
        }
    }
    void handleWriting() {
        if (fileDescriptor != -1) {
            writeCallback->call(fileDescriptor);
        }
    }
    
    void close() {
        if (fileDescriptor != -1) {
            ::close(fileDescriptor);
            fileDescriptor = -1;
        }
        readCallback.invalidate();
        writeCallback.invalidate();
    }
    
private:
    FileDescriptorListener()
        : fileDescriptor(-1)
    {}
    
    int fileDescriptor;
    Callback<int>::Ptr readCallback;
    Callback<int>::Ptr writeCallback;
};

} // namespace LucED

#endif // FILE_DESCRIPTOR_LISTENER_HPP
