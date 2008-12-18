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

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "File.hpp"
#include "ByteArray.hpp"
#include "Regex.hpp"
#include "FileException.hpp"
#include "System.hpp"

using namespace LucED;

File::File(const String& path, const String& fileName)
{
    if (path.getLength() > 0 && path[path.getLength() - 1] != '/'
     && fileName.getLength() > 0 && fileName[0] != '/') 
    {
        name << path << "/" << fileName;
    } else {
        name << path << fileName;
    }
}



void File::loadInto(ByteBuffer& buffer) const
{
    struct stat statData;
    int fd = open(name.toCString(), O_RDONLY);
    long len;
    byte *ptr;

    if (fd != -1) {
        if (fstat(fd, &statData) == -1) {
            throw FileException(errno, String() << "error accessing file '" << name << "': " << strerror(errno));
        }
        len = statData.st_size;
        ptr = buffer.appendAmount(len);
        if (read(fd, ptr, len) == -1) {
            throw FileException(errno, String() << "error reading from file '" << name << "': " << strerror(errno));
        }
        if (close(fd) == -1) {
            throw FileException(errno, String() << "error closing file '" << name << "' after reading: " << strerror(errno));
        }
    } else {
        throw FileException(errno, String() << "error opening file '" << name << "' for reading: " << strerror(errno));
    }
}

void File::storeData(ByteBuffer& data) const
{
    int fd = open(name.toCString(), O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
    
    if (fd != -1) {
        long length = data.getLength();
        if (write(fd, data.getAmount(0, length), length) == -1) {
            throw FileException(errno, String() << "error writing to file '" << name << "': " << strerror(errno));
        }
        if (close(fd) == -1) {
            throw FileException(errno, String() << "error closing file '" << name << "' after writing: " << strerror(errno));
        }
    } else {
        throw FileException(errno, String() << "error opening file '" << name << "' for writing: " << strerror(errno));
    }
}

String File::getAbsoluteName() const
{
    if (absoluteName.getLength() == 0)
    {
        String buffer;
        if (name.getLength() > 0 && name[0] == '/') {
            buffer = name;
        } else {
            buffer = String() << System::getInstance()->getCurrentDirectory() 
                              << "/" 
                              << name;
        }
        Regex r("/\\.(?=/)|/(?=/)|[^/]+/\\.\\./");
    
        while (r.findMatch(buffer, Regex::MatchOptions())) {
            buffer.removeAmount(r.getCaptureBegin(0), r.getCaptureLength(0));
        }
        absoluteName = buffer;
    }
    return absoluteName;
}

String File::getAbsoluteNameWithResolvedLinks() const
{
    String origName = getAbsoluteName();
    String absName = origName;

    while (true)
    {
        MemArray<char> buffer(2000);
        long len = 0;
        
        while (true)
        {
            len = readlink(absName.toCString(), buffer.getPtr(), buffer.getLength());
            if (len == buffer.getLength()) {
                buffer.increaseTo(len + 1000);
                continue;
            } else {
                break;
            }
        }
        if (len <= 0) {
            // The named file is not a symbolic link or does not exist or...
            return absName;
        } else {
            String linkContent = String(buffer.getPtr(), len);
            if (linkContent.getLength() > 0 && linkContent[0] == '/') {
                absName = linkContent;
            } else {
                absName = File(File(absName).getDirName(), linkContent).getAbsoluteName();
            }
        }
    }
}

String File::getBaseName() const
{
    int i = name.getLength();
    while (i > 0 && name[i-1] != '/') {
        i -= 1;
    }
    return name.getTail(i);
}

String File::getDirName() const
{
    String absoluteName = getAbsoluteName();

    int i = absoluteName.getLength();
    while (i > 0 && absoluteName[i-1] != '/') {
        i -= 1;
    }
    if (i > 0) {
        return absoluteName.getSubstring(0, i - 1);
    } else {
        return absoluteName;
    }
}

bool File::exists() const
{
    struct stat statData;
    String      fileName = getAbsoluteName();

    int rc = stat(fileName.toCString(), &statData);
    
    if (rc == -1 && errno == ENOENT) {
        return false;
    }
    else if (rc == 0) {
        return true;
    }
    else {
        throw FileException(errno, String() << "error accessing file '" << fileName << "': " << strerror(errno));
    }
}

File::Info File::getInfo() const
{
    struct stat statData;
    String      fileName = getAbsoluteName();

    if (stat(fileName.toCString(), &statData) == 0)
    {
        Info rslt;
        rslt.isFileFlag                  = S_ISREG(statData.st_mode);
        rslt.isDirectoryFlag             = S_ISDIR(statData.st_mode);
#if defined(STAT_HAS_ST_MTIM_TV_NSEC)
        rslt.lastModifiedTimeValSinceEpoche = TimeVal(     Seconds(statData.st_mtime),
                                                      MicroSeconds(statData.st_mtim.tv_nsec / 1000));
#elif defined(STAT_HAS_ST_MTIMENSE)
        rslt.lastModifiedTimeValSinceEpoche = TimeVal(     Seconds(statData.st_mtime),
                                                      MicroSeconds(statData.st_mtimensec / 1000));
#else
        rslt.lastModifiedTimeValSinceEpoche = TimeVal(     Seconds(statData.st_mtime),
                                                      MicroSeconds(0));
#endif
        rslt.isWritableFlag = (::access(getAbsoluteName().toCString(), W_OK) == 0);
        rslt.existsFlag = true;
        return rslt;
    }
    else if (errno == ENOENT)
    {
        Info rslt;
        rslt.existsFlag = false;
        return rslt;
    }
    else
    {
        throw FileException(errno, String() << "error accessing file '" << fileName << "': " << strerror(errno));
    }
}

