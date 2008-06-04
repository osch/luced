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

#include <limits.h>

#include "TextData.hpp"
#include "EventDispatcher.hpp"
#include "File.hpp"

using namespace std;
using namespace LucED;

TextData::TextData() 
        : modifiedFlag(false),
          viewCounter(0),
          hasHistoryFlag(false),
          isReadOnlyFlag(false),
          modifiedOnDiskFlag(false),
          ignoreModifiedOnDiskFlag(false),
          fileNamePseudoFlag(false)
{
    numberLines = 1;
    beginChangedPos = 0;
    changedAmount = 0;
    oldEndChangedPos = 0;
    oldLength = 0;
    EventDispatcher::getInstance()->registerUpdateSource(newCallback(this, &TextData::flushPendingUpdates));
}

void TextData::loadFile(const String& filename)
{
    File file(filename);

    file.loadInto(buffer);
    long len = buffer.getLength();
    byte* ptr = buffer.getTotalAmount();

    for (long i = 0; i < len; ++i) {
        if (ptr[i] == '\n') {
            ++this->numberLines;
        }
    }
    this->beginChangedPos = 0;
    this->changedAmount = len;
    this->oldEndChangedPos = 0;
    this->fileName = file.getAbsoluteName();
    fileNameListeners.invokeAllCallbacks(this->fileName);

    if (modifiedFlag == true) {
        modifiedFlag = false;
        changedModifiedFlagListeners.invokeAllCallbacks(modifiedFlag);
    }
    this->fileInfo = file.getInfo();
    this->modifiedOnDiskFlag = false;
    this->ignoreModifiedOnDiskFlag = false;

    if (isReadOnlyFlag != !fileInfo.isWritable()) {
        isReadOnlyFlag = !fileInfo.isWritable();
        readOnlyListeners.invokeAllCallbacks(isReadOnlyFlag);
    }
}


namespace
{
    struct LineAndColumn
    {
        LineAndColumn()
            : line(0), column(0)
        {}
        LineAndColumn(long line, long column)
            : line(line), column(column)
        {}
        long line;
        long column;
    };
}

void TextData::reloadFile()
{
    File file(this->fileName);

    long oldLength = getLength();
    long oldNumberLines = this->numberLines;

    ObjectArray<LineAndColumn> oldMarkPositions;

    for (long i = 0; i < marks.getLength(); ++i) {
        if (marks[i].inUseCounter > 0) {
            oldMarkPositions.append(LineAndColumn(marks[i].line,
                                                  marks[i].column));
        } else {
            oldMarkPositions.append(LineAndColumn(0, 0));
        }
    }
    
    buffer.clear();
    file.loadInto(buffer);
    long len = buffer.getLength();
    byte* ptr = buffer.getTotalAmount();

    this->numberLines = 1;
    for (long i = 0; i < len; ++i) {
        if (ptr[i] == '\n') {
            ++this->numberLines;
        }
    }
    this->beginChangedPos = 0;
    this->changedAmount = len - oldLength;
    this->oldEndChangedPos = oldLength;
    
    updateMarks(0, oldLength, len - oldLength,           // long beginChangedPos, long oldEndChangedPos, long changedAmount,
                0, this->numberLines - oldNumberLines);  // long beginLineNumber, long changedLineNumberAmount)

    for (long i = 0; i < marks.getLength(); ++i) {
        if (marks[i].inUseCounter > 0) {
            moveMarkToLineAndColumn(MarkHandle(i), oldMarkPositions[i].line,
                                                   oldMarkPositions[i].column);
        }
    }
    if (modifiedFlag == true) {
        modifiedFlag = false;
        changedModifiedFlagListeners.invokeAllCallbacks(modifiedFlag);
    }
    this->fileInfo = file.getInfo();
    this->modifiedOnDiskFlag = false;
    this->ignoreModifiedOnDiskFlag = false;

    if (isReadOnlyFlag != !fileInfo.isWritable()) {
        isReadOnlyFlag = !fileInfo.isWritable();
        readOnlyListeners.invokeAllCallbacks(isReadOnlyFlag);
    }
    clearHistory();
}

void TextData::checkFileInfo()
{
    if (fileNamePseudoFlag == false)
    {
        TimeVal lastModifiedTimeValSinceEpoche;
        bool fileExisted = false;
        
        if (fileInfo.exists())
        {
            fileExisted = true;
            lastModifiedTimeValSinceEpoche = fileInfo.getLastModifiedTimeValSinceEpoche();
        }
    
        this->fileInfo = File(this->fileName).getInfo();
        
        if (fileInfo.exists())
        {
            if (isReadOnlyFlag != !fileInfo.isWritable()) {
                isReadOnlyFlag = !fileInfo.isWritable();
                readOnlyListeners.invokeAllCallbacks(isReadOnlyFlag);
            }
            
            if ( fileExisted 
             && !fileInfo.getLastModifiedTimeValSinceEpoche().isEqualTo(lastModifiedTimeValSinceEpoche))
            {
                modifiedOnDiskFlag = true;
            }
        }
    }
}

void TextData::setRealFileName(const String& filename)
{
    this->fileNamePseudoFlag = false;
    this->fileName = File(filename).getAbsoluteName();
    fileNameListeners.invokeAllCallbacks(this->fileName);
    checkFileInfo();
}

void TextData::setPseudoFileName(const String& filename)
{
    this->fileNamePseudoFlag = true;
    this->fileName = File(filename).getAbsoluteName();
    fileNameListeners.invokeAllCallbacks(this->fileName);
}

void TextData::save()
{
    ASSERT(!fileNamePseudoFlag);

    File file(fileName);
    
    file.storeData(buffer);

    if (hasHistory()) {
        history->setPreviousActionToSavedState();
    }
    if (modifiedFlag == true) {
        modifiedFlag = false;
        changedModifiedFlagListeners.invokeAllCallbacks(modifiedFlag);
    }
    this->modifiedOnDiskFlag = false;
    this->ignoreModifiedOnDiskFlag = false;
    this->fileInfo = file.getInfo();
}

TextData::TextMark TextData::createNewMark() {
    long i = 0;
    for (; i < marks.getLength(); ++i) {
        if (!marks[i].inUseCounter > 0) {
            break;
        }
    }
    if (i == marks.getLength()) {
        marks.appendAmount(1);
    }
    marks[i].inUseCounter = 0;
    marks[i].pos          = 0;
    marks[i].column       = 0;
    marks[i].line         = 0;
    return TextMark(this, i);
}

TextData::TextMark TextData::createNewMark(MarkHandle src)
{
    TextMarkData& srcMark = marks[src.index];
    ASSERT(srcMark.inUseCounter > 0);

    TextMark rslt = createNewMark();
    TextMarkData& rsltMark = marks[rslt.index];
    rsltMark.pos    = srcMark.pos;
    rsltMark.column = srcMark.column;
    rsltMark.line   = srcMark.line;
    return rslt;
}


void TextData::updateMarks(
        long beginChangedPos, long oldEndChangedPos, long changedAmount,
        long beginLineNumber, long changedLineNumberAmount)
{
    long newEndChangedPos = oldEndChangedPos + changedAmount;
    bool endColCalculated = false;
    bool beginColCalculated = false;
    long beginColumns = 0, endColumns = 0;
    
    ASSERT(beginChangedPos <= oldEndChangedPos);
    ASSERT(beginChangedPos <= oldEndChangedPos + changedAmount);

    for (long i = 0; i < marks.getLength(); ++i) {
        if (marks[i].inUseCounter > 0) {
            if (marks[i].pos <= beginChangedPos) {
                continue;
            }
            if (marks[i].pos >= oldEndChangedPos) {
                if (marks[i].pos   - oldEndChangedPos > marks[i].column) {
                    marks[i].pos  += changedAmount;
                    marks[i].line += changedLineNumberAmount;
                } else {
                    if (!endColCalculated) {
                        endColCalculated = true;
                        endColumns = newEndChangedPos - getThisLineBegin(newEndChangedPos);
                    }
                    marks[i].column = marks[i].pos - oldEndChangedPos + endColumns;
                    marks[i].pos   += changedAmount;
                    marks[i].line  += changedLineNumberAmount;
                }
                continue;
            }
            if (!beginColCalculated) {
                long bol = getThisLineBegin(beginChangedPos);
                beginColCalculated = true;
                beginColumns = beginChangedPos - bol;
            }
            marks[i].pos    = beginChangedPos;
            marks[i].column = beginColumns;
            marks[i].line   = beginLineNumber;
            ASSERT(marks[i].pos <= this->getLength());
        }
    }
}

void TextData::setInsertFilterCallback(Callback<const byte**, long*>::Ptr filterCallback)
{
    this->filterCallback = filterCallback;
}


inline long TextData::internalInsertAtMark(MarkHandle m, const byte* insertBuffer, long length)
{
    if (!isReadOnlyFlag)
    {
        if (length > 0)
        {
            int lineCounter = 0;
            for (int i = 0; i < length; ++i) {
                if (insertBuffer[i] == '\n') {
                    ++lineCounter;
                }
            }
    
            TextMarkData& mark = marks[m.index];
            long lineNumber = mark.line;
            long pos = mark.pos;
    
            buffer.insert(pos, insertBuffer, length);
    
            if (pos < this->beginChangedPos) {
                this->beginChangedPos = pos;
                this->changedAmount  += length;
                if (pos > this->oldEndChangedPos) {
                    this->oldEndChangedPos = pos;
                }
            } else if (pos < this->oldEndChangedPos + this->changedAmount) {
                this->changedAmount  += length;
            } else {
                this->oldEndChangedPos += pos - (this->oldEndChangedPos + this->changedAmount);
                this->changedAmount  += length;
            }
            this->numberLines += lineCounter;
            updateMarks(pos, pos, length, lineNumber, lineCounter);
        }
        return length;
    }
    else {
        return 0;
    }
}


long TextData::undo(MarkHandle m)
{
    if (!isReadOnlyFlag)
    {
        long spos = LONG_MAX;
        long epos = 0;
        
        if (hasHistory())
        {
            bool first = true;
            
            do
            {
                switch (history->getPreviousActionType())
                {
                    case EditingHistory::ACTION_INSERT: {
                        long pos               = history->getPreviousActionTextPos();
                        long length            = history->getPreviousActionLength();
                        moveMarkToPos(m, pos);
                        history->undoInsertAction(buffer.getAmount(pos, length));
                        internalRemoveAtMark(m, length);
                        if (spos > pos) {
                            spos = pos;
                        }
                        if (epos < pos + length) {
                            epos = pos;
                        } else {
                            epos -= length;
                        }
                        break;
                    }
                    case EditingHistory::ACTION_DELETE: {
                        long pos               = history->getPreviousActionTextPos();
                        long length            = history->getPreviousActionLength();
                        moveMarkToPos(m, pos);
                        internalInsertAtMark(m,  history->getContentForUndoDeleteAction(), length);
                        history->undoDeleteAction();
    
                        if (spos > pos) {
                            spos = pos;
                        }
                        if (epos < pos + length) {
                            epos = pos + length;
                        } else {
                            epos += length;
                        }
                        break;
                    }
                    case EditingHistory::ACTION_NONE: {
                        break;
                    }
                    case EditingHistory::ACTION_SELECT: {
                        long pos               = history->getPreviousActionTextPos();
                        long length            = history->getPreviousActionLength();
                        
                        history->undoSelectAction();
                        
                        if (spos > pos) {
                            spos = pos;
                        }
                        if (epos < pos + length) {
                            epos = pos + length;
                        }
                        break;
                    }
                    default: ASSERT(false);
                }
            } while (!history->isPreviousActionSectionSeperator());
    
            bool newModifiedFlag = !history->isPreviousActionSavedState();
            if (newModifiedFlag != modifiedFlag) {
                modifiedFlag = newModifiedFlag;
                changedModifiedFlagListeners.invokeAllCallbacks(modifiedFlag);
            }
        }
        if (epos >= spos) {
            moveMarkToPos(m, spos);
            return epos - spos;
        } else {
            return 0;
        }
    }
    else {
        return 0;
    }
}

long TextData::redo(MarkHandle m)
{
    if (!isReadOnlyFlag)
    {
        long spos = LONG_MAX;
        long epos = 0;
            
        if (hasHistory())
        {
            do
            {
                switch (history->getNextActionType())
                {
                    case EditingHistory::ACTION_INSERT: {
                        long pos               = history->getNextActionTextPos();
                        long length            = history->getNextActionLength();
                        moveMarkToPos(m, pos);
                        internalInsertAtMark(m, history->getContentForRedoInsertAction(), length);
                        history->redoInsertAction();
    
                        if (spos > pos) {
                            spos = pos;
                        }
                        if (epos < pos + length) {
                            epos = pos + length;
                        } else {
                            epos += length;
                        }
                        break;
                    }
                    case EditingHistory::ACTION_DELETE: {
                        long pos               = history->getNextActionTextPos();
                        long length            = history->getNextActionLength();
                        moveMarkToPos(m, pos);
                        history->redoDeleteAction(buffer.getAmount(pos, length), length);
                        internalRemoveAtMark(m, length);
    
                        if (spos > pos) {
                            spos = pos;
                        }
                        if (epos < pos + length) {
                            epos = pos;
                        } else {
                            epos -= length;
                        }
                        break;
                    }
                    case EditingHistory::ACTION_NONE: {
                        break;
                    }
                    case EditingHistory::ACTION_SELECT: {
                        long pos               = history->getNextActionTextPos();
                        long length            = history->getNextActionLength();
                        
                        history->redoSelectAction();
                        
                        if (spos > pos) {
                            spos = pos;
                        }
                        if (epos < pos + length) {
                            epos = pos + length;
                        }
                        break;
                    }
                    default: ASSERT(false);
                }
            } while (!history->isLastAction() && !history->isPreviousActionSectionSeperator());
    
            bool newModifiedFlag = !history->isPreviousActionSavedState();
            if (newModifiedFlag != modifiedFlag) {
                modifiedFlag = newModifiedFlag;
                changedModifiedFlagListeners.invokeAllCallbacks(modifiedFlag);
            }
        }
        if (epos >= spos) {
            moveMarkToPos(m, spos);
            return epos - spos;
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}

long TextData::insertAtMark(MarkHandle m, const byte* insertBuffer, long length)
{
    if (!isReadOnlyFlag)
    {
        if (filterCallback.isValid()) {
            filterCallback->call(&insertBuffer, &length);
        }
        if (length > 0)
        {
            if (hasHistory()) {
                TextMarkData& mark = marks[m.index];
                long pos = mark.pos;
                history->rememberInsertAction(pos, length);
            }
            internalInsertAtMark(m, insertBuffer, length);
    
            if (modifiedFlag == false) {
                modifiedFlag = true;
                changedModifiedFlagListeners.invokeAllCallbacks(modifiedFlag);
            }
        }
        return length;
    }
    else {
        return 0;
    }
}




inline void TextData::internalRemoveAtMark(MarkHandle m, long amount)
{
    if (!isReadOnlyFlag)
    {
        TextMarkData& mark = marks[m.index];
        long lineNumber = mark.line;
        long pos = mark.pos;
        
        int lineCounter = 0;
        for (int p = mark.pos, i = 0; i < amount; ++i) {
            if (buffer[p + i] == '\n') {
                ++lineCounter;
            }
        }
    
        buffer.removeAmount(mark.pos, amount);
        
        long newEndChangedPos = this->oldEndChangedPos + this->changedAmount;
        if (newEndChangedPos < pos + amount) {
            newEndChangedPos = pos + amount;
            this->oldEndChangedPos = newEndChangedPos - this->changedAmount;
        }
        this->changedAmount -= amount;
        
        if (pos < this->beginChangedPos) {
            this->beginChangedPos = pos;
        }
        this->numberLines -= lineCounter;
        updateMarks(pos, pos + amount, -amount, lineNumber, -lineCounter);
    
        if (modifiedFlag == false) {
            modifiedFlag = true;
            changedModifiedFlagListeners.invokeAllCallbacks(modifiedFlag);
        }
    }
}

void TextData::setModifiedFlag(bool newFlag)
{
    if (newFlag != modifiedFlag)
    {
        modifiedFlag = newFlag;
        changedModifiedFlagListeners.invokeAllCallbacks(modifiedFlag);
    }
}

void TextData::removeAtMark(MarkHandle m, long amount)
{
    if (!isReadOnlyFlag)
    {
        if (amount > 0)
        {
            TextMarkData& mark = marks[m.index];
    
            if (hasHistory()) {
                history->rememberDeleteAction(mark.pos, 
                                              amount, 
                                              buffer.getAmount(mark.pos, amount));
            }
            internalRemoveAtMark(m, amount);
    
            if (modifiedFlag == false) {
                modifiedFlag = true;
                changedModifiedFlagListeners.invokeAllCallbacks(modifiedFlag);
            }
        }
    }
}

void TextData::clear()
{
    TextMark m = createNewMark();
    removeAtMark(m, getLength());
}


void TextData::clearHistory()
{
    if (hasHistory()) {
        history->clear();
    }
}

void TextData::reset()
{
    int oldLength = getLength();
    if (oldLength > 0) {
        int oldNumberLines = numberLines;
        this->buffer.clear();
        this->numberLines = 1;
        this->changedAmount -= oldLength;
        this->oldEndChangedPos = oldLength;
        this->beginChangedPos = 0;
        updateMarks(0, oldLength, -oldLength, 0, -(oldNumberLines - 1));
    }

    if (hasHistory()) {
        history->clear();
    }

    if (modifiedFlag == false) {
        modifiedFlag = true;
        changedModifiedFlagListeners.invokeAllCallbacks(modifiedFlag);
    }
}


void TextData::moveMarkToLineAndColumn(MarkHandle m, long newLine, long newColumn)
{
    if (newLine >= numberLines) {
        newLine = numberLines - 1;
    }

    TextMarkData& mark = marks[m.index];

    if (newLine < mark.line) {
        if (newLine - 0 <= mark.line - newLine) {
            mark.pos = 0;
            mark.line = 0;
            for (int i = 0, count = newLine - mark.line; i < count; ++i) {
                mark.pos = getNextLineBegin(mark.pos);
            }
        } else {
            mark.pos = getThisLineBegin(mark.pos);
            for (int i = 0, count = mark.line - newLine; i < count; ++i) {
                mark.pos = getPrevLineBegin(mark.pos);
            }
        }
    } else if (newLine > mark.line) {
        if (getNumberOfLines() - newLine <= newLine - mark.line) {
            mark.pos = getThisLineBegin(getLength());
            mark.line = getNumberOfLines() - 1;
            for (int i = 0, count = mark.line - newLine; i < count; ++i) {
                mark.pos = getPrevLineBegin(mark.pos);
            }
        } else {
            mark.pos = getThisLineBegin(mark.pos);
            for (int i = 0, count = newLine - mark.line; i < count; ++i) {
                mark.pos = getNextLineBegin(mark.pos);
            }
        }
    } else {
        mark.pos = getThisLineBegin(mark.pos);
    }
    ASSERT(isBeginOfLine(mark.pos));
    mark.line   = newLine;
    int c = 0;
    for (int i = mark.pos; !isEndOfLine(i) && i < mark.pos + newColumn; ++i) {
        ++c;
    }
    mark.pos   += c;
    mark.column = c;
}

void TextData::moveMarkToBeginOfLine(MarkHandle m)
{
    TextMarkData& mark = marks[m.index];
    long pos = getThisLineBegin(mark.pos);
    mark.pos = pos;
    mark.column = 0;
}

void TextData::moveMarkToEndOfLine(MarkHandle m)
{
    TextMarkData& mark = marks[m.index];
    long pos = getThisLineEnding(mark.pos);
    mark.column += pos - mark.pos;
    mark.pos = pos;
}

void TextData::moveMarkToNextLineBegin(MarkHandle m)
{
    TextMarkData& mark = marks[m.index];
    long pos = getNextLineBegin(mark.pos);
    mark.line += 1;
    mark.pos = pos;
    mark.column = 0;
}

void TextData::moveMarkToPrevLineBegin(MarkHandle m)
{
    TextMarkData& mark = marks[m.index];
    long pos = getPrevLineBegin(mark.pos);
    mark.line -= 1;
    mark.pos = pos;
    mark.column = 0;
}

void TextData::moveMarkToPos(MarkHandle m, long pos)
{
    TextMarkData& mark = marks[m.index];
    
    if (pos < mark.pos)
    {
        if (pos - 0 <= mark.pos - pos) {
            mark.pos = 0;
            mark.line = 0;
            while (mark.pos < pos) {
                if (isEndOfLine(mark.pos)) {
                    mark.pos += getLengthOfLineEnding(mark.pos);
                    mark.line += 1;
                } else {
                    mark.pos += 1;
                }
            }
            mark.column = pos - getThisLineBegin(pos);
        } else {
            do {
                if (isBeginOfLine(mark.pos)) {
                    mark.line -= 1;
                    mark.pos -= getLengthOfPrevLineEnding(mark.pos);
                } else {
                    mark.pos -= 1;
                }
            } while (pos < mark.pos);
            mark.column = pos - getThisLineBegin(pos);
        }
    } 
    else if (mark.pos < pos)
    {
        if (getLength() - pos <= pos - mark.pos) {
            mark.pos = getLength();
            mark.line = getNumberOfLines() - 1;
            while (pos < mark.pos) {
                if (isBeginOfLine(mark.pos)) {
                    mark.line -= 1;
                    mark.pos -= getLengthOfPrevLineEnding(mark.pos);
                } else {
                    mark.pos -= 1;
                }
            }
            mark.column = pos - getThisLineBegin(pos);
        } else {
            do {
                if (isEndOfLine(mark.pos)) {
                    mark.pos += getLengthOfLineEnding(mark.pos);
                    mark.line += 1;
                } else {
                    mark.pos += 1;
                }
            } while (mark.pos < pos);
            mark.column = pos - getThisLineBegin(pos);
        }
    }    
}

void TextData::moveMarkToPosOfMark(MarkHandle m, MarkHandle toMark)
{
    if (m.index != toMark.index)
    {
        ASSERT(marks[     m.index].inUseCounter > 0);
        ASSERT(marks[toMark.index].inUseCounter > 0);
        TextMarkData& mark = marks[     m.index];
        TextMarkData& to   = marks[toMark.index];
        mark.pos = to.pos;
        mark.line = to.line;
        mark.column = to.column;
    }
}

void TextData::flushPendingUpdatesIntern()
{
    ASSERT(changedAmount != 0 || oldEndChangedPos != 0);
    
    if (hasHistory()) {
        history->setSectionMarkOnHistoryTop();
    }

    updateListeners.invokeAllCallbacks(UpdateInfo(beginChangedPos, oldEndChangedPos, changedAmount));
    
    beginChangedPos = getLength();
    changedAmount = 0;
    oldEndChangedPos = 0;
    
    if (oldLength != getLength())
    {
        lengthListeners.invokeAllCallbacks(getLength());
        oldLength = getLength();
    }
}

void TextData::registerUpdateListener(Callback<UpdateInfo>::Ptr updateCallback)
{
    updateListeners.registerCallback(updateCallback);
}

void TextData::registerFileNameListener(Callback<const String&>::Ptr fileNameCallback)
{
    fileNameListeners.registerCallback(fileNameCallback);
    fileNameCallback->call(fileName);
}

void TextData::registerReadOnlyListener(Callback<bool>::Ptr readOnlyCallback)
{
    readOnlyListeners.registerCallback(readOnlyCallback);
    readOnlyCallback->call(isReadOnlyFlag);
}

void TextData::registerLengthListener(Callback<long>::Ptr lengthCallback)
{
    lengthListeners.registerCallback(lengthCallback);
    lengthCallback->call(getLength());
}

void TextData::registerModifiedFlagListener(Callback<bool>::Ptr modifiedFlagCallback)
{
    changedModifiedFlagListeners.registerCallback(modifiedFlagCallback);
    modifiedFlagCallback->call(modifiedFlag);
}

void TextData::setHistorySeparator()
{
    if (hasHistory()) {
        history->setSectionMarkOnPreviousAction();
        history->setMergeStopMarkOnPreviousAction(true);
    }
}

void TextData::setMergableHistorySeparator()
{
    if (hasHistory()) {
        history->setSectionMarkOnPreviousAction();
    }
}
