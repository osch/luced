/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, osch@luced.de
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

#include "File.h"
#include "ByteArray.h"
#include "Regex.h"

using namespace LucED;
using std::string;

void File::loadInto(ByteBuffer& buffer)
{
    struct stat stat_st;
    int fd = open(name.c_str(), O_RDONLY);
    long len;
    byte *ptr;

    if (fd != -1) {
        fstat(fd, &stat_st);
        len = stat_st.st_size;
        ptr = buffer.appendAmount(len);
        read(fd, ptr, len);
        close(fd);
    }
}

string File::getAbsoluteFileName()
{
    string buffer;
    if (name.length() > 0 && name[0] == '/') {
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
        buffer = string(cwd.toCStr()) + "/" + name;
    }
    Regex r("/\\.(?=/)|/(?=/)|[^/]+/\\.\\./");
    MemArray<int> ovec(r.getOvecSize());

    while (r.findMatch(buffer.c_str(), buffer.length(), 0, Regex::MatchOptions(), ovec)) {
        buffer.erase(ovec[0], ovec[1] - ovec[0]);
    }
    return buffer;
}

