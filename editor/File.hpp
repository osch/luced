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

#ifndef FILE_HPP
#define FILE_HPP

#include "String.hpp"

#include "NonCopyable.hpp"
#include "ByteBuffer.hpp"
#include "TimeVal.hpp"
#include "Seconds.hpp"
#include "MicroSeconds.hpp"

namespace LucED {



class File : public NonCopyable
{
public:

    class Info
    {
    public:
        bool isFile() const {
            return isFileFlag;
        }
        bool isDirectory() const {
            return isDirectoryFlag;
        }
        bool isWritable() const {
            return isWritableFlag;
        }
        TimeVal getLastModifiedTimeValSinceEpoche() const {
            return lastModifiedTimeValSinceEpoche;
        }
    private:
        friend class File;
        bool         isFileFlag;
        bool         isDirectoryFlag;
        bool         isWritableFlag;
        TimeVal      lastModifiedTimeValSinceEpoche;
    };
    
    File(const String& path, const String& fileName);
    
    File(const String& fileName)
        : name(fileName)
    {}
    
    String getAbsoluteFileName() const;
    
    String getBaseName() const;
    
    String getDirName() const;

    void loadInto(ByteBuffer& buffer);
    
    void storeData(ByteBuffer& data);
    
    bool exists() const;
    
    Info getInfo() const;
    
private:
   String name;
};

} // namespace LucED

#endif // FILE_HPP

