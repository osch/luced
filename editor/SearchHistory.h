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

#include "String.h"

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
        Entry()
            : wholeWordFlag(false),
              regexFlag(false),
              ignoreCaseFlag(false),
              hasReplaceStringFlag(false)
        {}
        
        void takeValues(const Entry& rhs) {
            findString     = rhs.findString;
            if (!hasReplaceStringFlag) {
                hasReplaceStringFlag = rhs.hasReplaceStringFlag;
                replaceString        = rhs.replaceString;
            }
            wholeWordFlag  = rhs.wholeWordFlag;
            regexFlag      = rhs.regexFlag;
            ignoreCaseFlag = rhs.ignoreCaseFlag;
        }

        String getFindString()    const { return findString; }
        String getReplaceString() const { return replaceString; }
        bool   hasReplaceString() const { return hasReplaceStringFlag; }

        void setReplaceString(String replaceString) {
            this->hasReplaceStringFlag = true;
            this->replaceString = replaceString;
        }
        void setFindString(String findString) {
            this->findString = findString;
        }
        
        void setWholeWordFlag(bool flag) {
            wholeWordFlag = flag;
        }
        void setRegexFlag(bool flag) {
            regexFlag = flag;
        }
        void setIgnoreCaseFlag(bool flag) {
            ignoreCaseFlag = flag;
        }
        bool getWholeWordFlag() const {
            return wholeWordFlag;
        }
        bool getRegexFlag() const {
            return regexFlag;
        }
        bool getIgnoreCaseFlag() const {
            return ignoreCaseFlag;
        }

    private:
        String findString;
        String replaceString;
        bool wholeWordFlag;
        bool regexFlag;
        bool ignoreCaseFlag;
        bool hasReplaceStringFlag;
    };

    void append(const Entry& newEntry);

    const Entry& getLast() const {
        return entries.getLast();
    }

    int getEntryCount() const {
        return entries.getLength();
    }
    
    const Entry& getEntry(int index) const {
        return entries[index];
    }
    
    Entry& getEntry(int index) {
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
