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

#include "SearchHistory.h"

using namespace LucED;



SingletonInstance<SearchHistory> SearchHistory::instance;

SearchHistory* SearchHistory::getInstance()
{
    return instance.getPtr();
}

SearchHistory::SearchHistory()
{
    entries.append(Entry());
}


void SearchHistory::append(const Entry& newEntry)
{
    String findString = newEntry.getFindString();
    String replaceString = newEntry.getReplaceString();
    if (findString == "" && replaceString == "") {
        return;
    }
    bool wasAppended = false;

    Entry& last = entries.getLast();
    if (last.getFindString() == "" || last.getFindString() == findString)
    {
        if (last.getReplaceString() == "" || last.getReplaceString() == replaceString) {
            last = newEntry;
            wasAppended = true;
        }
    }

    if (!wasAppended) {
        entries.append(newEntry);
    }
}
