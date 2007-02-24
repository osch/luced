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

#ifndef SEARCHHISTORY_H
#define SEARCHHISTORY_H

#include <string>

#include "HeapObject.h"
#include "ObjectArray.h"
#include "SingletonInstance.h"

namespace LucED
{

class SearchHistory : public HeapObject
{
public:
    static SearchHistory* getInstance();    

    class Entry
    {
    public:
        Entry(std::string findString, std::string replaceString)
            : findString(findString),
              replaceString(replaceString)
        {}
        std::string getFindString()    const { return findString; }
        std::string getReplaceString() const { return replaceString; }

        void setReplaceString(std::string replaceString) {
            this->replaceString = replaceString;
        }
        void setFindString(std::string findString) {
            this->findString = findString;
        }

    private:
        std::string findString;
        std::string replaceString;
    };

    void append(std::string findString, std::string replaceString = "");

    const Entry& getLast() const {
        return entries.getLast();
    }

    int getEntryCount() const {
        return entries.getLength();
    }
    
    const Entry& getEntry(int index) const {
        return entries[index];
    }
    
private:
    friend class SingletonInstance<SearchHistory>;
    static SingletonInstance<SearchHistory> instance;

    SearchHistory();
    
    ObjectArray<Entry> entries;
};

} // namespace LucED

#endif // SEARCHHISTORY_H
