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

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "config.h"
#include "File.hpp"
#include "ByteArray.hpp"
#include "Regex.hpp"
#include "FileException.hpp"
#include "System.hpp"
#include "Nullable.hpp"

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



void File::loadInto(RawPtr<ByteBuffer> buffer) const
{
    int fd = open(name.toCString(), O_RDONLY);

    if (fd != -1)
    {
        struct stat statData;
    
        if (fstat(fd, &statData) == -1) {
            throw FileException(errno, String() << "error accessing file '" << name << "': " << strerror(errno));
        }
        long  len       = statData.st_size;
        long  oldLen    = buffer->getLength();
        if (len > oldLen) {
            buffer->appendAmount(len - oldLen);
        } else if (len < oldLen) {
            buffer->removeTail(len);
        }
        long  bytesRead = read(fd, buffer->getPtr(), len);

        if (bytesRead < 0) {
            buffer->clear();
            throw FileException(errno, String() << "error reading from file '" << name << "': " << strerror(errno));
        }
        if (bytesRead < len) {
            buffer->removeTail(oldLen + bytesRead);
        }
        if (close(fd) == -1) {
            throw FileException(errno, String() << "error closing file '" << name << "' after reading: " << strerror(errno));
        }
    } else {
        throw FileException(errno, String() << "error opening file '" << name << "' for reading: " << strerror(errno));
    }
}

File::Writer::Ptr File::openForWriting() const
{
    int fd = open(name.toCString(), O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);

    if (fd == -1) {
        throw FileException(errno, String() << "error opening file '" << name << "' for writing: " << strerror(errno));
    }
    return Writer::create(fd, name);
}

File::Writer::~Writer()
{
    if (close(fd) == -1) {
        throw FileException(errno, String() << "error closing file '" << name << "' after writing: " << strerror(errno));
    }
}

void File::Writer::write(const char* data, long length) const
{
    if (length <= 0) {
        return;
    }
    if (::write(fd, data, length) == -1) {
        throw FileException(errno, String() << "error writing to file '" << name << "': " << strerror(errno));
    }
}

void File::storeData(const char* data, int length) const
{
    int fd = open(name.toCString(), O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
    
    if (fd != -1) {
        if (write(fd, data, length) == -1) {
            throw FileException(errno, String() << "error writing to file '" << name << "': " << strerror(errno));
        }
        if (close(fd) == -1) {
            throw FileException(errno, String() << "error closing file '" << name << "' after writing: " << strerror(errno));
        }
    } else {
        throw FileException(errno, String() << "error opening file '" << name << "' for writing: " << strerror(errno));
    }
}

void File::storeData(const char* data) const
{
    storeData(data, strlen(data));
}

void File::storeData(RawPtr<ByteBuffer> data) const
{
    int length = data->getLength();
    storeData((char*)data->getAmount(0, length), length);
}

String internalCanonicalize(const String& fname)
{
    String buffer = fname;
    
    Regex r("/\\.(?=/)|/(?=/)|[^/]+/\\.\\./|/+$");

    while (r.findMatch(buffer, Regex::MatchOptions())) {
        buffer.removeAmount(Pos(r.getCaptureBegin(0)), Len(r.getCaptureLength(0)));
    }
    return buffer;
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
        absoluteName = internalCanonicalize(buffer);
    }
    return absoluteName;
}

static Nullable<String> internalReadLink1(const String& fname)
{
    MemArray<char> buffer(2000);
    long len = 0;
    
    while (true)
    {
        len = readlink(fname.toCString(), buffer.getPtr(), buffer.getLength());
        if (len == buffer.getLength()) {
            buffer.increaseTo(len + 1000);
            continue;
        } else {
            break;
        }
    }
    if (len > 0) {
        return String(buffer.getPtr(), len);
    } else {
        return Null;
    }
}
static String internalReadLink2(const String& fname)
{
    String s = fname;
    
    while (true)
    {
        Nullable<String> rslt = internalReadLink1(s);
        
        if (rslt.isValid()) {
            s = rslt.get();
        } else {
            return s;
        }
    }
}

String File::getAbsoluteNameWithResolvedLinks() const
{
    String origName = getAbsoluteName();
    int    origLen  = origName.getLength();
    String rsltName;
    
    int p1 = 0;

    while (p1 < origLen && origName[p1] == '/')
    {
        int p2 = p1 + 1;
        
        while (p2 < origLen && origName[p2] != '/') ++p2;
        if (p2 <= origLen) {
            String fragment = origName.getSubstring(Pos(p1), Pos(p2));
            rsltName = internalReadLink2(String() << rsltName << fragment);
        }
        p1 = p2;
    }
    rsltName = internalCanonicalize(rsltName);

    return rsltName;
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
        return absoluteName.getHead(i - 1);
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

        TimePeriod timePeriodSincePosixEpoch;
        {
        #if HAVE_STAT_MTIME_TV_NSEC
            timePeriodSincePosixEpoch +=      Seconds(statData.st_mtime);
            timePeriodSincePosixEpoch += MicroSeconds(statData.st_mtim.tv_nsec / 1000);
        #elif HAVE_STAT_MTIME_MTIMENSEC
            timePeriodSincePosixEpoch +=      Seconds(statData.st_mtime);
            timePeriodSincePosixEpoch += MicroSeconds(statData.st_mtimensec / 1000);
        #elif HAVE_STAT_MTIME_MTIMESPEC
            timePeriodSincePosixEpoch +=      Seconds(statData.st_mtime);
            timePeriodSincePosixEpoch += MicroSeconds(statData.st_mtimespec.tv_nsec / 1000);
        #else
            timePeriodSincePosixEpoch +=      Seconds(statData.st_mtime);
            timePeriodSincePosixEpoch += MicroSeconds(0);
        #endif
        }
        rslt.lastModifiedTime = TimeStamp::forTimePeriodSincePosixEpoch(timePeriodSincePosixEpoch);
        
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


void File::createDirectory() const
{
    const String fileName = getAbsoluteName();
    const int    len      = fileName.getLength();

    int i = 0;
    while (i < len && fileName[i] == '/') ++i;
    do {
        while (i < len && fileName[i] != '/') ++i;
        String path = fileName.getHead(i);
        if (!File(path).exists())
        {
            int rc = mkdir(path.toCString(), S_IRUSR|S_IWUSR|S_IXUSR|
                                             S_IRGRP|S_IWGRP|S_IXGRP|
                                             S_IROTH|S_IWOTH|S_IXOTH);
            if (rc != 0 && errno != EEXIST) {
                throw FileException(errno, String() << "error creating directory '" << path << "': " << strerror(errno));
            }
        }
        while (i < len && fileName[i] == '/') ++i;
    }
    while (i < len);
}

