#ifndef DIRECTORYREADER_H
#define DIRECTORYREADER_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include "NonCopyable.h"
#include "ObjectArray.h"

namespace LucED {

class DirectoryReader : NonCopyable
{
public:
    DirectoryReader(const string& path) {
        this->path = path;
        dir = opendir(path.c_str());
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
    string getName() {
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
                stat((path + "/" + dirent->d_name).c_str(), &statInfo);
                wasStat = true;
            }
            return S_ISREG(statInfo.st_mode);
        }
    }
private:
    string path;
    DIR* dir;
    struct dirent* dirent;
    bool wasStat;
    struct stat statInfo;
};


} // namespace LucED

#endif // DIRECTORYREADER_H
