/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2009 Oliver Schmidt, oliver at luced dot de
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

#ifndef FILE_HPP
#define FILE_HPP

#include "String.hpp"

#include "NonCopyable.hpp"
#include "ByteBuffer.hpp"
#include "TimeVal.hpp"
#include "Seconds.hpp"
#include "MicroSeconds.hpp"

namespace LucED
{

class File
{
public:

    class Info
    {
    public:
        Info()
            : isFileFlag(false),
              isDirectoryFlag(false),
              isWritableFlag(false),
              existsFlag(false)
        {}
        bool isFile() const {
            ASSERT(existsFlag);
            return isFileFlag;
        }
        bool isDirectory() const {
            ASSERT(existsFlag);
            return isDirectoryFlag;
        }
        bool isWritable() const {
            ASSERT(existsFlag);
            return isWritableFlag;
        }
        TimeVal getLastModifiedTimeValSinceEpoche() const {
            ASSERT(existsFlag);
            return lastModifiedTimeValSinceEpoche;
        }
        bool exists() const {
            return existsFlag;
        }
    private:
        friend class File;
        bool         isFileFlag;
        bool         isDirectoryFlag;
        bool         isWritableFlag;
        bool         existsFlag;
        TimeVal      lastModifiedTimeValSinceEpoche;
    };
    
    class Writer : public HeapObject
    {
    public:
        typedef OwningPtr<Writer> Ptr;
        
        ~Writer();
        
        void write(const char* data, long length) const;
        
    private:
        static Ptr create(int fd, const String& name) {
            return Ptr(new Writer(fd, name));
        }
        explicit Writer(int fd, const String& name)
            : fd(fd),
              name(name)
        {}
        
        friend class File;
        int fd;
        String name;
    };
    
    File(const String& path, const String& fileName);
    
    File(const String& fileName = "")
        : name(fileName)
    {}
    
    Writer::Ptr openForWriting() const;
    
    String getAbsoluteName() const;
    
    String getAbsoluteNameWithResolvedLinks() const;
    
    String getBaseName() const;
    
    String getDirName() const;
    
    File getDir() const {
        return File(getDirName());
    }

    void loadInto(ByteBuffer& buffer) const;
    
    void storeData(const char* data, int length) const;

    void storeData(const char* data) const;
    
    void storeData(ByteBuffer& data) const;
    
    bool exists() const;
    
    Info getInfo() const;
    
    void createDirectory() const;

    operator String() const {
        return name;
    }
    
private:
   String         name;
   mutable String absoluteName;
};

} // namespace LucED

#endif // FILE_HPP
