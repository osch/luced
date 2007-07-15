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

#ifndef DIRECTORYREADER_H
#define DIRECTORYREADER_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

#include "NonCopyable.hpp"
#include "ObjectArray.hpp"
#include "FileException.hpp"

namespace LucED {

class DirectoryReader : NonCopyable
{
public:
    DirectoryReader(const String& path) {
        this->path = path;
        dir = opendir(path.toCString());
        dirent = NULL;
    }
    
    ~DirectoryReader() {
        if (dir != NULL) {
            closedir(dir);
        }
    }
    
    bool next() {
        if (dir != NULL) {
            dirent = readdir(dir);
            wasStat = false;
            if (dirent == NULL) {
                closedir(dir);
                dir = NULL;
            }
        }
        return dirent != NULL;
    }
    String getName() {
        if (dirent != NULL) {
            return dirent->d_name;
        } else {
            return "";
        }
    }
    bool isFile() {
        if (dirent == NULL) {
            return false;
        } else {
            if (!wasStat) {
                String fileName = String() << path << "/" << dirent->d_name;
                if (stat(fileName.toCString(), &statInfo) == -1) {
                    throw FileException(errno, String() << "error accessing file '" << fileName << "': " << strerror(errno));
                }
                wasStat = true;
            }
            return S_ISREG(statInfo.st_mode);
        }
    }
private:
    String path;
    DIR* dir;
    struct dirent* dirent;
    bool wasStat;
    struct stat statInfo;
};


} // namespace LucED

#endif // DIRECTORYREADER_H
