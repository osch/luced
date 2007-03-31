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

#ifndef EDITINGHISTORY_H
#define EDITINGHISTORY_H

#include <string.h>

#include "HeapObject.h"
#include "OwningPtr.h"
#include "MemBuffer.h"
#include "OptionBits.h"

namespace LucED
{

class EditingHistory : public HeapObject
{
public:
    typedef OwningPtr<EditingHistory> Ptr;

    enum ActionType {
        ACTION_NONE,
        ACTION_INSERT,
        ACTION_DELETE
    };
    
    enum ActionFlag {
        FLAG_SECTION_MARK = 1,
        FLAG_MERGE_STOP = 2
    };
    
    typedef OptionBits<ActionFlag> ActionFlags;

    class Action
    {
    public:
        
    private:
        friend class EditingHistory;
        
        ActionType  type;
        long        textDataPos;
        long        length;
        ActionFlags flags;
    };
    
    static Ptr create() {
        return Ptr(new EditingHistory());
    }
    
    void rememberInsertAction(long beginIndex, long length)
    {
        if (isPreviousActionMergeable()
            && getPreviousActionType() == ACTION_INSERT
            && getPreviousActionTextPos() + getPreviousActionLength() == beginIndex)
        {
            actions.removeTail(nextActionIndex);
            actions[nextActionIndex - 1].length += length;

            historyData.removeTail(historyDataIndex);
        }
        else
        {
            actions.removeTail(nextActionIndex);
            actions.appendAmount(1);
            actions[nextActionIndex].type = ACTION_INSERT;
            actions[nextActionIndex].textDataPos = beginIndex;
            actions[nextActionIndex].length = length;
            actions[nextActionIndex].flags.clear();
            nextActionIndex += 1;

            historyData.removeTail(historyDataIndex);
        }
    }
    
    void rememberDeleteAction(long beginIndex, long length, const byte* data)
    {
        if (isPreviousActionMergeable()
         && getPreviousActionType() == ACTION_DELETE
         && beginIndex                 <= getPreviousActionTextPos()
         && getPreviousActionTextPos() <= beginIndex + length)
        {
            actions.removeTail(nextActionIndex);
            historyData.removeTail(historyDataIndex);
            
            long lengthBefore = getPreviousActionTextPos() - beginIndex;
            long lengthAfter = beginIndex + length - getPreviousActionTextPos();

            ASSERT(lengthBefore >= 0);
            ASSERT(lengthAfter >= 0);

            if (lengthBefore > 0)
            {
                memcpy(historyData.insertAmount(historyDataIndex - getPreviousActionLength(),
                                                lengthBefore),
                       data,
                       lengthBefore);
                                         
                actions[nextActionIndex - 1].textDataPos -= lengthBefore;
                actions[nextActionIndex - 1].length      += lengthBefore;
                historyDataIndex                     += lengthBefore;
            }
            if (lengthAfter > 0)
            {
                memcpy(historyData.insertAmount(historyDataIndex,
                                                lengthAfter),
                       data + lengthBefore,
                       lengthAfter);

                actions[nextActionIndex - 1].length += lengthAfter;
                historyDataIndex                += lengthAfter;
            }
        }
        else
        {
            actions.removeTail(nextActionIndex);
            actions.appendAmount(1);
            actions[nextActionIndex].type = ACTION_DELETE;
            actions[nextActionIndex].textDataPos = beginIndex;
            actions[nextActionIndex].length = length;
            actions[nextActionIndex].flags.clear();
            nextActionIndex += 1;

            historyData.removeTail(historyDataIndex);
            historyData.appendAmount(length);
            memcpy(historyData.getAmount(historyDataIndex, length),
                   data,
                   length);
            historyDataIndex += length;
        }
    }
    
    void setSectionMarkOnHistoryTop() {
        if (actions.getLength() > 0) {
            actions[actions.getLength() - 1].flags.set(FLAG_SECTION_MARK);
        }
    }

    void setSectionMarkOnPreviousAction() {
        if (nextActionIndex > 0) {
            actions[nextActionIndex - 1].flags.set(FLAG_SECTION_MARK);
        }
    }
    
    void setMergeStopMarkOnPreviousAction() {
        if (nextActionIndex > 0) {
            actions[nextActionIndex - 1].flags.set(FLAG_MERGE_STOP);
        }
    }

    bool isFirstAction() const
    {
        return nextActionIndex == 0;
    }

    bool isPreviousActionSectionSeperator() const {
        if (nextActionIndex == 0) {
            return true;
        } else {
            return actions[nextActionIndex - 1].flags.isSet(FLAG_SECTION_MARK);
        }
    }
    
    bool isPreviousActionMergeable() const {
        if (nextActionIndex == 0) {
            return false;
        } else {
            return !actions[nextActionIndex - 1].flags.isSet(FLAG_MERGE_STOP);
        }
    }
    
    bool isPreviousActionSavedState() const {
        return nextActionIndex - 1 == savedActionIndex;
    }
    
    void setPreviousActionToSavedState() {
        savedActionIndex = nextActionIndex - 1;
    }
    
    ActionType getPreviousActionType() const
    {
        if (nextActionIndex == 0) {
            return ACTION_NONE;
        } else {
            return actions[nextActionIndex - 1].type;
        }
    }
    
    long getPreviousActionTextPos() const
    {
        ASSERT(nextActionIndex > 0);
        return actions[nextActionIndex - 1].textDataPos;
    }
    
    long getPreviousActionLength() const
    {
        ASSERT(nextActionIndex > 0);
        return actions[nextActionIndex - 1].length;
    }
    
    bool isLastAction() const
    {
        return nextActionIndex == actions.getLength();
    }
    
    bool isNextActionSectionSeperator() const {
        if (nextActionIndex == actions.getLength()) {
            return true;
        } else {
            return actions[nextActionIndex].flags.isSet(FLAG_SECTION_MARK);
        }
    }
    
    ActionType getNextActionType() const
    {
        if (nextActionIndex == actions.getLength()) {
            return ACTION_NONE;
        } else {
            return actions[nextActionIndex].type;
        }
    }
    
    long getNextActionTextPos() const
    {
        ASSERT(nextActionIndex < actions.getLength());
        return actions[nextActionIndex].textDataPos;
    }
    
    long getNextActionLength() const
    {
        ASSERT(nextActionIndex < actions.getLength());
        return actions[nextActionIndex].length;
    }
    
    void undoInsertAction(const byte* insertedText)
    {
        ASSERT(getPreviousActionType() == ACTION_INSERT);
        long length = getPreviousActionLength();
        
        memcpy(historyData.insertAmount(historyDataIndex, length),
               insertedText,
               length);
                
        nextActionIndex -= 1;
    }

    const byte* getContentForRedoInsertAction() const
    {
        ASSERT(getNextActionType() == ACTION_INSERT);

        long length = getNextActionLength();

        return historyData.getAmount(historyDataIndex, length);
    }

    void redoInsertAction()
    {
        ASSERT(getNextActionType() == ACTION_INSERT);
        long length = getNextActionLength();

        historyData.removeAmount(historyDataIndex, length);
        nextActionIndex += 1;
    }

    const byte* getContentForUndoDeleteAction() const
    {
        ASSERT(getPreviousActionType() == ACTION_DELETE);

        long length = getPreviousActionLength();

        return historyData.getAmount(historyDataIndex - length, length);
    }
    
    void undoDeleteAction()
    {
        ASSERT(getPreviousActionType() == ACTION_DELETE);
        long length = getPreviousActionLength();
        
        historyData.removeAmount(historyDataIndex - length, length);
        historyDataIndex -= length;
        nextActionIndex -= 1;
    }
    
    void redoDeleteAction(const byte* deletedText, long length)
    {
        ASSERT(getNextActionType() == ACTION_DELETE);
        ASSERT(length == getNextActionLength());
        
        memcpy(historyData.insertAmount(historyDataIndex, length),
               deletedText,
               length);
        
        historyDataIndex += length;
        nextActionIndex += 1;
    }

private:

    EditingHistory()
        : nextActionIndex(0),
          historyDataIndex(0),
          savedActionIndex(-1)
    {}
    
    MemBuffer<Action> actions;
    MemBuffer<byte>   historyData;
    long              nextActionIndex;
    long              historyDataIndex;
    long              savedActionIndex;
};

} // namespace LucED

#endif // EDITINGHISTORY_H
