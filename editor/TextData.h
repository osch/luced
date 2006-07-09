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

#ifndef TEXTDATA_H
#define TEXTDATA_H

#include "HeapObject.h"
#include "MemBuffer.h"
#include "ByteArray.h"
#include "ObjectArray.h"
#include "Slot.h"
#include "CallbackContainer.h"
#include "OwningPtr.h"

namespace LucED {


class TextData : public HeapObject
{
public:

    typedef OwningPtr<TextData> Ptr;

    struct UpdateInfo
    {
        long beginChangedPos;
        long oldEndChangedPos;
        long changedAmount;
        
        UpdateInfo(long beginChangedPos, long oldEndChangedPos, long changedAmount)
          : beginChangedPos(beginChangedPos), oldEndChangedPos(oldEndChangedPos), changedAmount(changedAmount)
        {}
    };
    typedef Callback1<UpdateInfo> UpdateCallback;

    class MarkHandle
    {
    protected:
        friend class TextData;
        long index;
    };

    class TextMark : public MarkHandle
    {
    public:
        TextMark() : textData(NULL) {}
        ~TextMark() {
            if (textData != NULL) {
                textData->getTextMarkData(index)->inUseCounter -= 1;
            }
        }
        TextMark(const TextMark& src) {
            index = src.index;
            textData = src.textData;
            if (textData != NULL) {
                textData->getTextMarkData(index)->inUseCounter += 1;
            }
        }
        TextMark& operator=(const TextMark& src) {
            ASSERT(textData == src.textData || textData ==  NULL || src.textData == NULL);
            if (src.textData != NULL) {
                src.textData->getTextMarkData(src.index)->inUseCounter += 1;
            }
            if (textData != NULL) {
                textData->getTextMarkData(index)->inUseCounter -= 1;
            }
            index = src.index;
            textData = src.textData;
        }
        void moveToLineAndColumn(long line, long column) {
            textData->moveMarkToLineAndColumn(*this, line, column);
        }
        void moveToPos(long pos) {
            textData->moveMarkToPos(*this, pos);
        }
        long getPos() {
            return textData->getTextPositionOfMark(*this);
        }
        long getLine() {
            return textData->getLineNumberOfMark(*this);
        }
        long getColumn() {
            return textData->getColumnNumberOfMark(*this);
        }
        void moveToBeginOfLine() {
            textData->moveMarkToBeginOfLine(*this);
        }
        void moveToEndOfLine() {
            textData->moveMarkToEndOfLine(*this);
        }
        byte getChar() {
            return textData->getChar(*this);
        }
        void inc() {
            textData->incMark(*this);
        }
        bool isEndOfText() {
            return textData->isEndOfText(*this);
        }
        bool isValid() {
            return textData != NULL;
        }
    private:
        friend class TextData;
        TextMark(TextData *textData, long index) {
            this->index = index;
            this->textData = textData;
            textData->getTextMarkData(index)->inUseCounter += 1;
        }
        
        TextData *textData;
    };
    
    class TextMarkData
    {
    private:
        friend class TextData;
        friend class TextMark;
        int inUseCounter;
        long pos;
        long line;
        long column;
    };
    
    static TextData::Ptr create() {
        return TextData::Ptr(new TextData());
    }

    void loadFile(const char *filename);

    long getLength() const;
    long getNumberOfLines() const;

    TextMark createNewMark();
    TextMark createNewMark(MarkHandle src);
    TextMarkData* getTextMarkData(long index) {
        return &(marks[index]);
    }

    byte *getAmount(long pos, long amount);

    const byte& operator[](long pos) const {
        return buffer[pos];
    }
    byte getChar(long pos) const {
        return buffer[pos];
    }
    byte getChar(MarkHandle m) const {
        return buffer[marks[m.index].pos];
    }
    
    bool isEndOfText(long pos) const {
        return pos == buffer.getLength();
    }
    bool isEndOfLine(long pos) const {
        return isEndOfText(pos) || buffer[pos] == '\n';
    }
    bool isBeginOfLine(long pos) const {
        return pos == 0 || buffer[pos - 1] == '\n'; 
    }
    long getLengthOfLineEnding(long pos) const {
        ASSERT(isEndOfLine(pos));
        return isEndOfText(pos) ? 0 : 1;
    }
    long getLengthOfPrevLineEnding(long pos) const {
        ASSERT(isBeginOfLine(pos));
        return pos == 0 ? 0 : 1;
    }
    long getLengthToEndOfLine(long pos) const {
        long epos = pos;
        for (; epos < buffer.getLength() && buffer[epos] != '\n'; ++epos);
        return epos - pos;
    }
    long getNextLineBegin(long pos) const {
        pos += getLengthToEndOfLine(pos);
        pos += getLengthOfLineEnding(pos);
        return pos;
    }
    long getThisLineBegin(long pos) const {
        while (pos > 0) {
            if (buffer[pos - 1] == '\n')
                return pos;
            --pos;
        }
        return pos;
    }
    long getThisLineEnding(long pos) const {
        while (!isEndOfLine(pos)) {
            pos += 1;
        }
        return pos;
    }
    long getPrevLineBegin(long pos) const {
        pos = getThisLineBegin(pos);
        pos -= getLengthOfPrevLineEnding(pos);
        return getThisLineBegin(pos);
    }
    long insertAtMark(MarkHandle m, byte c);
    long insertAtMark(MarkHandle m, const ByteArray& buffer);
    long insertAtMark(MarkHandle m, const byte* buffer, long length);
    void removeAtMark(MarkHandle m, long amount);
    
    void moveMarkToLineAndColumn(MarkHandle m, long line, long column);
    void moveMarkToBeginOfLine(MarkHandle m);
    void moveMarkToEndOfLine(MarkHandle m);
    void moveMarkToNextLineBegin(MarkHandle m);
    void moveMarkToPrevLineBegin(MarkHandle m);
    void moveMarkToPos(MarkHandle m, long pos);
    void moveMarkToPosOfMark(MarkHandle m, MarkHandle toMark);
    
    void incMark(MarkHandle m) {
        moveMarkToPos(m, marks[m.index].pos + 1);
    }
    bool isEndOfText(MarkHandle m) {
        return marks[m.index].pos == buffer.getLength();
    }
    void setInsertFilterCallback(Callback2<const byte**, long*>& filterCallback);
    void registerUpdateListener(UpdateCallback& updateCallback);
    void registerFileNameListener(Callback1<const string&>& fileNameCallback);
    void registerLengthListener(Callback1<long>& lengthCallback);
    
    void flushPendingUpdatesIntern();
    void flushPendingUpdates() {
        if (changedAmount != 0 || oldEndChangedPos != 0) {
            flushPendingUpdatesIntern();
        }
    }
    
    long getBeginChangedPos() {return beginChangedPos;}
    long getChangedAmount()   {return changedAmount;}
    long getTextPositionOfMark(MarkHandle mark) const {
    	return marks[mark.index].pos;
    }
    long getColumnNumberOfMark(MarkHandle mark) const {
    	return marks[mark.index].column;
    }
    long getLineNumberOfMark(MarkHandle mark) const {
    	return marks[mark.index].line;
    }
    
private:

    TextData();
    MemBuffer<byte> buffer;
    long numberLines;
    long beginChangedPos;
    long changedAmount;
    long oldEndChangedPos;
    ObjectArray<TextMarkData> marks;

    Slot0 slotForFlushPendingUpdates;

    void updateMarks(
        long beginChangedPos, long oldEndChangedPos, long changedAmount,
        long beginLineNumber, long changedLineNumberAmount);
        
    Callback1Container<UpdateInfo> updateListeners;
    Callback1Container<const string&> fileNameListeners;
    Callback1Container<long> lengthListeners;
    
    Callback2<const byte**, long*> filterCallback;
    
    string fileName;
    long oldLength;
};

} // namespace LucED

#endif // TEXTDATA_H
