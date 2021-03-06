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

#ifndef EDITINGHISTORY_H
#define EDITINGHISTORY_H

#include "String.hpp"

#include "HeapObject.hpp"
#include "OwningPtr.hpp"
#include "MemBuffer.hpp"
#include "Flags.hpp"


namespace LucED
{

class EditingHistory : public HeapObject
{
public:
    typedef OwningPtr<EditingHistory> Ptr;
    
    class SectionHolder : public HeapObject
    {
    public:
        typedef OwningPtr<SectionHolder> Ptr;
    private:
        friend class EditingHistory;
        
        static Ptr create() {
            return Ptr(new SectionHolder());
        }
        
        SectionHolder()
        {}
    };

    enum ActionType {
        ACTION_NONE,
        ACTION_INSERT,
        ACTION_DELETE,
        ACTION_SELECT
    };
    
    enum ActionFlag {
        FLAG_SECTION_MARK,
        FLAG_MERGE_STOP
    };
    
    typedef Flags<ActionFlag> ActionFlags;

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
            && (   getPreviousActionTextPos() + getPreviousActionLength() == beginIndex
                || getPreviousActionTextPos() == beginIndex))
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
    
    void rememberSelectAction(long beginIndex, long length)
    {
        actions.removeTail(nextActionIndex);
        actions.appendAmount(1);
        actions[nextActionIndex].type = ACTION_SELECT;
        actions[nextActionIndex].textDataPos = beginIndex;
        actions[nextActionIndex].length = length;
        actions[nextActionIndex].flags.clear();
        nextActionIndex += 1;
    
        historyData.removeTail(historyDataIndex);
    }
    
    void setSectionMarkOnHistoryTop() {
        if (actions.getLength() > 0 && sectionHolder.getRefCounter() <= 1) {
            actions[actions.getLength() - 1].flags.set(FLAG_SECTION_MARK);
        }
    }

    void setSectionMarkOnPreviousAction() {
        if (nextActionIndex > 0 && sectionHolder.getRefCounter() <= 1) {
            actions[nextActionIndex - 1].flags.set(FLAG_SECTION_MARK);
        }
    }
    
    void setMergeStopMarkOnPreviousAction(bool flag = true) {
        if (nextActionIndex > 0 && sectionHolder.getRefCounter() <= 1) {
            if (flag) {
                actions[nextActionIndex - 1].flags.set(FLAG_MERGE_STOP);
            } else {
                actions[nextActionIndex - 1].flags.clear(FLAG_MERGE_STOP);
            }
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
    
    void undoSelectAction()
    {
        ASSERT(getPreviousActionType() == ACTION_SELECT);
        nextActionIndex -= 1;
    }

    void redoSelectAction()
    {
        ASSERT(getNextActionType() == ACTION_SELECT);
        nextActionIndex += 1;
    }

    void clear()
    {
        nextActionIndex  = 0;
        historyDataIndex = 0;
        savedActionIndex = -1;
        actions.clear();
        historyData.clear();
    }
    
    SectionHolder::Ptr getSectionHolder() {
        if (!sectionHolder.isValid()) {
            sectionHolder = SectionHolder::create();
        }
        return sectionHolder;
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
    SectionHolder::Ptr sectionHolder;
};

} // namespace LucED

#endif // EDITINGHISTORY_H
