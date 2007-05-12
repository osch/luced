/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, oliver at luced dot de
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

#include "File.hpp"
#include "ByteArray.hpp"
#include "Regex.hpp"
#include "FileException.hpp"

using namespace LucED;


void File::loadInto(ByteBuffer& buffer)
{
    struct stat stat_st;
    int fd = open(name.toCString(), O_RDONLY);
    long len;
    byte *ptr;

    if (fd != -1) {
        fstat(fd, &stat_st);
        len = stat_st.st_size;
        ptr = buffer.appendAmount(len);
        if (read(fd, ptr, len) == -1) {
            throw FileException(String() << "error reading from file '" << name << "': " << strerror(errno));
        }
        if (close(fd) == -1) {
            throw FileException(String() << "error closing file '" << name << "' after reading: " << strerror(errno));
        }
    } else {
        throw FileException(String() << "error opening file '" << name << "' for reading: " << strerror(errno));
    }
}

void File::storeData(ByteBuffer& data)
{
    int fd = open(name.toCString(), O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
    
    if (fd != -1) {
        long length = data.getLength();
        if (write(fd, data.getAmount(0, length), length) == -1) {
            throw FileException(String() << "error writing to file '" << name << "': " << strerror(errno));
        }
        if (close(fd) == -1) {
            throw FileException(String() << "error closing file '" << name << "' after writing: " << strerror(errno));
        }
    } else {
        throw FileException(String() << "error opening file '" << name << "' for writing: " << strerror(errno));
    }
}

String File::getAbsoluteFileName() const
{
    String buffer;
    if (name.getLength() > 0 && name[0] == '/') {
        buffer = name;
    } else {
        ByteArray cwd;
        cwd.appendAmount(255);
        do {
            if (getcwd((char*)cwd.getPtr(0), cwd.getLength()) == NULL && errno == ERANGE) {
                cwd.increaseTo(2 * cwd.getLength());
                continue;
            }
        } while (false);
        buffer = String() << cwd.toCStr() << "/" << name;
    }
    Regex r("/\\.(?=/)|/(?=/)|[^/]+/\\.\\./");
    MemArray<int> ovec(r.getOvecSize());

    while (r.findMatch(buffer.toCString(), buffer.getLength(), 0, Regex::MatchOptions(), ovec)) {
        buffer.removeAmount(ovec[0], ovec[1] - ovec[0]);
    }
    return buffer;
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
    String absoluteName = getAbsoluteFileName();

    int i = absoluteName.getLength();
    while (i > 0 && absoluteName[i-1] != '/') {
        i -= 1;
    }
    return absoluteName.getSubstring(0, i);
}
