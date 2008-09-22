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

#include <iostream>

#include <X11/keysym.h>
#include <X11/Xatom.h>

#include "Clipboard.hpp"
#include "FindUtil.hpp"
#include "KeyModifier.hpp"
#include "FileOpener.hpp"
#include "Regex.hpp"
#include "MultiLineEditActions.hpp"

using namespace LucED;

void MultiLineEditActions::cursorDown()
{
    if (!e->areCursorChangesDisabled())
    {
        e->releaseSelection();
        long pos = e->getCursorTextPosition();
        pos = e->getTextData()->getNextLineBegin(pos);
        if (e->getTextData()->isBeginOfLine(pos)) {
            pos = e->getTextPosForPixX(e->getRememberedCursorPixX(), pos);
            e->moveCursorToTextPosition(pos);
        } else {
            // Cursor is in last line
        }
    }
    e->assureCursorVisible();
}


void MultiLineEditActions::cursorUp()
{
    if (!e->areCursorChangesDisabled())
    {
        e->releaseSelection();
        long pos = e->getCursorTextPosition();
        pos = e->getTextData()->getPrevLineBegin(pos);
        pos = e->getTextPosForPixX(e->getRememberedCursorPixX(), pos);
        e->moveCursorToTextPosition(pos);
    }
    e->assureCursorVisible();
}

void MultiLineEditActions::selectionCursorDown()
{
    if (!e->areCursorChangesDisabled())
    {
        if (e->getCursorLineNumber() + 1 < e->getTextData()->getNumberOfLines())
        {
            long oldCursorPos = e->getCursorTextPosition();
            long newCursorPos = e->getTextPosForPixX(e->getRememberedCursorPixX(),
                                                     e->getTextData()->getNextLineBegin(oldCursorPos));
            
            e->moveCursorToTextPosition(newCursorPos);

            if (!e->hasPrimarySelection()) {
                e->setPrimarySelection(oldCursorPos, newCursorPos);
            } else {
                e->extendSelectionTo(newCursorPos);
            }
        }
    }
    e->assureCursorVisible();
}


void MultiLineEditActions::selectionCursorUp()
{
    if (!e->areCursorChangesDisabled())
    {
        if (e->getCursorLineNumber() > 0)
        {
            long oldCursorPos = e->getCursorTextPosition();
            long newCursorPos = e->getTextPosForPixX(e->getRememberedCursorPixX(), 
                                                     e->getTextData()->getPrevLineBegin(oldCursorPos));
        
            e->moveCursorToTextPosition(newCursorPos);

            if (!e->hasPrimarySelection()) {
                e->setPrimarySelection(oldCursorPos, newCursorPos);
            } else {
                e->extendSelectionTo(newCursorPos);
            }
        }
    }
    e->assureCursorVisible();
}


void MultiLineEditActions::selectionLineCursorDown()
{
    if (!e->areCursorChangesDisabled())
    {
        if (e->getCursorLineNumber() + 1 < e->getTextData()->getNumberOfLines())
        {
            long oldCursorPos = e->getCursorTextPosition();
            long nextLineBeginPos = e->getTextData()->getNextLineBegin(oldCursorPos);
        
            if (e->hasPrimarySelection())
            {
                long spos1 = e->getBeginSelectionPos();
                long spos2 = e->getTextData()->getThisLineBegin(spos1);
                if (spos1 != spos2) {
                    e->moveSelectionBeginTo(spos2);
                }
                long epos1 = e->getEndSelectionPos();
                if (!e->getTextData()->isBeginOfLine(epos1)) {
                    long epos2 = e->getTextData()->getNextLineBegin(epos1);
                    if (epos1 != epos2) {
                        e->moveSelectionEndTo(epos2);
                        epos1 = epos2;
                    }
                }
                if (epos1 == nextLineBeginPos) {
                    e->setAnchorToBeginOfSelection();
                }
            }
    
            long newCursorPos = e->getTextPosForPixX(e->getRememberedCursorPixX(), nextLineBeginPos);
            e->moveCursorToTextPosition(newCursorPos);
    
            if (e->hasPrimarySelection())
            {
                long pos2;
                if (e->isAnchorAtBeginOfSelection()) {
                    pos2 = e->getTextData()->getNextLineBegin(newCursorPos);
                } else {
                    pos2 = e->getTextData()->getThisLineBegin(newCursorPos);
                }
                e->extendSelectionTo(pos2);
            }
            else {
                e->setPrimarySelection(e->getTextData()->getThisLineBegin(oldCursorPos),
                                       e->getTextData()->getNextLineBegin(newCursorPos));
            }
        }
    }
    e->assureCursorVisible();
}


void MultiLineEditActions::selectionLineCursorUp()
{
    if (!e->areCursorChangesDisabled())
    {
        if (e->getCursorLineNumber() > 0)
        {
            long oldCursorPos = e->getCursorTextPosition();
            long prevLineBeginPos = e->getTextData()->getPrevLineBegin(oldCursorPos);
            long nextLineBeginPos = e->getTextData()->getNextLineBegin(oldCursorPos);
            
            if (e->hasPrimarySelection())
            {
                long spos1 = e->getBeginSelectionPos();
                long spos2 = e->getTextData()->getThisLineBegin(spos1);
                if (spos1 != spos2) {
                    e->moveSelectionBeginTo(spos2);
                }
                if (spos2 == e->getTextData()->getThisLineBegin(oldCursorPos)) {
                    e->setAnchorToEndOfSelection();
                }
                long epos1 = e->getEndSelectionPos();
                if (!e->getTextData()->isBeginOfLine(epos1)) {
                    long epos2 = e->getTextData()->getNextLineBegin(epos1);
                    if (epos1 != epos2) {
                        e->moveSelectionEndTo(epos2);
                        epos1 = epos2;
                    }
                }
            }
    
            long newCursorPos = e->getTextPosForPixX(e->getRememberedCursorPixX(), prevLineBeginPos);
            e->moveCursorToTextPosition(newCursorPos);

            if (e->hasPrimarySelection())
            {            
                long pos2;
                if (e->isAnchorAtBeginOfSelection()) {
                    pos2 = e->getTextData()->getNextLineBegin(newCursorPos);
                } else {
                    pos2 = e->getTextData()->getThisLineBegin(newCursorPos);
                }
                e->extendSelectionTo(pos2);
            }
            else {
                e->setPrimarySelection(nextLineBeginPos, 
                                       e->getTextData()->getThisLineBegin(newCursorPos));
            }
        }
    }
    e->assureCursorVisible();
}



void MultiLineEditActions::shiftBlockLeft()
{
    if (!e->areCursorChangesDisabled() && e->hasSelection() && !e->isReadOnly())
    {
        RawPtr<TextData> textData                       = e->getTextData();
        TextData::HistorySection::Ptr historySection = textData->createHistorySection();
        
        TextData::TextMark mark   = e->getNewMarkToBeginOfSelection();
        long               endPos = e->getEndSelectionPos();

        textData->rememberChangeAreaInHistory(mark.getPos(), endPos);
        
        if (!mark.isAtBeginOfLine()) {
            mark.moveToNextLineBegin();
        }

        const long tabWidth = e->getLanguageMode()->getHardTabWidth();
        ByteArray expandedTabMinusOne;
        expandedTabMinusOne.appendAndFillAmountWith(tabWidth - 1, ' ');

        while (mark.getPos() < endPos) {
            byte c = textData->getChar(mark);
            if (c == '\t') {
                textData->removeAtMark(mark, 1);
                textData->insertAtMark(mark, expandedTabMinusOne);
                endPos += -1 + (tabWidth - 1);
            } else if (c == ' ') {
                textData->removeAtMark(mark, 1);
                endPos += -1;
            }
            mark.moveToNextLineBegin();
        }
        e->assureSelectionVisible();
    } else {
        e->assureCursorVisible();
    }
    e->rememberCursorPixX();
}

void MultiLineEditActions::shiftBlockRight()
{
    if (!e->areCursorChangesDisabled() && e->hasSelection() && !e->isReadOnly())
    {
        RawPtr<TextData> textData                       = e->getTextData();
        TextData::HistorySection::Ptr historySection = textData->createHistorySection();
        
        TextData::TextMark mark   = e->getNewMarkToBeginOfSelection();
        long               endPos = e->getEndSelectionPos();
        
        textData->rememberChangeAreaInHistory(mark.getPos(), endPos);

        if (!mark.isAtBeginOfLine()) {
            mark.moveToNextLineBegin();
        }

        ByteArray space;
        space.append(' ');
        
        while (mark.getPos() < endPos) {
            textData->insertAtMark(mark, space);
            endPos += 1;
            mark.moveToNextLineBegin();
        }
        e->assureSelectionVisible();
    } else {
        e->assureCursorVisible();
    }
    e->rememberCursorPixX();
}

void MultiLineEditActions::cursorPageDown()
{
    if (!e->areCursorChangesDisabled())
    {
        e->releaseSelection();
        long targetLine = e->getCursorLineNumber() + e->getNumberOfVisibleLines() - 1;
        long targetTopLine = e->getTopLineNumber() + e->getNumberOfVisibleLines() - 1;

        if (targetLine > e->getTextData()->getNumberOfLines()) {
            targetLine = e->getCursorLineNumber();
        }
        if (targetTopLine > e->getTextData()->getNumberOfLines() - e->getNumberOfVisibleLines()) {
            targetTopLine = e->getTextData()->getNumberOfLines() - e->getNumberOfVisibleLines();
        }
        TextData::TextMark mark = e->createNewMarkFromCursor();
        mark.moveToLineAndColumn(targetLine, 0);

        long newPos = e->getTextPosForPixX(e->getRememberedCursorPixX(), mark.getPos());
        mark.moveToPos(newPos);

        e->setTopLineNumber(targetTopLine);
        e->moveCursorToTextMark(mark);
    }
    e->assureCursorVisible();
}


void MultiLineEditActions::cursorPageUp()
{
    if (!e->areCursorChangesDisabled())
    {
        e->releaseSelection();
        long targetLine = e->getCursorLineNumber() - (e->getNumberOfVisibleLines() - 1);
        long targetTopLine = e->getTopLineNumber() - (e->getNumberOfVisibleLines() - 1);

        if (targetLine < 0) {
            targetLine = e->getCursorLineNumber();
        }
        if (targetTopLine < 0) {
            targetTopLine = 0;
        }
        TextData::TextMark mark = e->createNewMarkFromCursor();
        mark.moveToLineAndColumn(targetLine, 0);

        long newPos = e->getTextPosForPixX(e->getRememberedCursorPixX(), mark.getPos());
        mark.moveToPos(newPos);

        e->setTopLineNumber(targetTopLine);
        e->moveCursorToTextMark(mark);
    }
    e->assureCursorVisible();
}

void MultiLineEditActions::selectionCursorPageDown()
{
    if (!e->areCursorChangesDisabled())
    {
        long oldCursorPos = e->getCursorTextPosition();

        long targetLine = e->getCursorLineNumber() + e->getNumberOfVisibleLines() - 1;
        long targetTopLine = e->getTopLineNumber() + e->getNumberOfVisibleLines() - 1;

        if (targetLine > e->getTextData()->getNumberOfLines()) {
            targetLine = e->getCursorLineNumber();
        }
        if (targetTopLine > e->getTextData()->getNumberOfLines() - e->getNumberOfVisibleLines()) {
            targetTopLine = e->getTextData()->getNumberOfLines() - e->getNumberOfVisibleLines();
        }
        TextData::TextMark mark = e->createNewMarkFromCursor();
        mark.moveToLineAndColumn(targetLine, 0);

        long newPos = e->getTextPosForPixX(e->getRememberedCursorPixX(), mark.getPos());
        mark.moveToPos(newPos);

        e->setTopLineNumber(targetTopLine);
        e->moveCursorToTextMark(mark);
        
        long newCursorPos = mark.getPos();
        
        if (e->hasPrimarySelection()) {
            e->extendSelectionTo(newCursorPos);
        } else {
            e->setPrimarySelection(oldCursorPos, newCursorPos);
        }
    }
    e->assureCursorVisible();
}


void MultiLineEditActions::selectionCursorPageUp()
{
    if (!e->areCursorChangesDisabled())
    {
        long oldCursorPos = e->getCursorTextPosition();

        long targetLine = e->getCursorLineNumber() - (e->getNumberOfVisibleLines() - 1);
        long targetTopLine = e->getTopLineNumber() - (e->getNumberOfVisibleLines() - 1);

        if (targetLine < 0) {
            targetLine = e->getCursorLineNumber();
        }
        if (targetTopLine < 0) {
            targetTopLine = 0;
        }
        TextData::TextMark mark = e->createNewMarkFromCursor();
        mark.moveToLineAndColumn(targetLine, 0);

        long newPos = e->getTextPosForPixX(e->getRememberedCursorPixX(), mark.getPos());
        mark.moveToPos(newPos);

        e->setTopLineNumber(targetTopLine);
        e->moveCursorToTextMark(mark);
        
        long newCursorPos = mark.getPos();

        if (e->hasPrimarySelection()) {
            e->extendSelectionTo(newCursorPos);
        } else {
            e->setPrimarySelection(oldCursorPos, newCursorPos);
        }
    }
    e->assureCursorVisible();
}

void MultiLineEditActions::scrollCursorDown()
{
    e->assureCursorVisible();
    if (!e->areCursorChangesDisabled())
    {
        e->hideCursor();
        e->releaseSelection();
        if (e->scrollDown()) {
            long pos = e->getCursorTextPosition();
            pos = e->getTextData()->getNextLineBegin(pos);
            if (e->getTextData()->isBeginOfLine(pos)) {
                pos = e->getTextPosForPixX(e->getRememberedCursorPixX(), pos);
                e->moveCursorToTextPosition(pos);
            } else {
                // Cursor is in last line
            }
        }
        e->showCursor();
    }
}


void MultiLineEditActions::scrollCursorUp()
{
    e->assureCursorVisible();
    if (!e->areCursorChangesDisabled())
    {
        e->hideCursor();
        e->releaseSelection();
        if (e->scrollUp()) {
            long pos = e->getCursorTextPosition();
            pos = e->getTextData()->getPrevLineBegin(pos);
            pos = e->getTextPosForPixX(e->getRememberedCursorPixX(), pos);
            e->moveCursorToTextPosition(pos);
        }
        e->showCursor();
    }
}


void MultiLineEditActions::insertNewLineAutoIndent()
{
    newLineAutoIndent(true);
}
void MultiLineEditActions::appendNewLineAutoIndent()
{
    newLineAutoIndent(false);
}

void MultiLineEditActions::newLineAutoIndent(bool insert)
{
    if (!e->areCursorChangesDisabled() && !e->isReadOnly())
    {
        e->setCurrentActionCategory(TextEditorWidget::ACTION_NEWLINE);
        
        TextData::HistorySection::Ptr historySectionHolder = e->getTextData()->createHistorySection();

        if (e->hasPrimarySelection())
        {
            long selBegin  = e->getBeginSelectionPos();
            long selLength = e->getEndSelectionPos() - selBegin;
            e->moveCursorToTextPosition(selBegin);
            if (insert) {
                e->removeAtCursor(selLength);
            }
            e->releaseSelection();
        } else if (e->hasPseudoSelection()) {
            e->releaseSelection();
        }
        TextData::TextMark mark = e->createNewMarkFromCursor();
        ByteArray whiteSpace;
        whiteSpace.append('\n');

        e->hideCursor();
        if (!insert) {
            mark.moveToEndOfLine();
            e->moveCursorToTextMark(mark);
        }

        mark.moveToBeginOfLine();
        while (!mark.isEndOfText() && mark.getPos() < e->getCursorTextPosition()) {
            byte c = mark.getChar();
            if (c == ' ' || c == '\t') {
                whiteSpace.append(c);
            } else {
                break;
            }
            mark.inc();
        }

        e->insertAtCursor(whiteSpace);
        e->moveCursorToTextPosition(e->getCursorTextPosition() + whiteSpace.getLength());
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
    e->showCursor();
}

void MultiLineEditActions::newLineFixedColumnIndent(bool forward)
{
    if (!e->areCursorChangesDisabled() && !e->isReadOnly())
    {
        e->setCurrentActionCategory(TextEditorWidget::ACTION_NEWLINE);
        
        TextData::HistorySection::Ptr historySectionHolder = e->getTextData()->createHistorySection();

        if (e->hasPrimarySelection()) {
            long selBegin  = e->getBeginSelectionPos();
            long selLength = e->getEndSelectionPos() - selBegin;
            e->moveCursorToTextPosition(selBegin);
            e->removeAtCursor(selLength);
            e->releaseSelection();
        } else if (e->hasPseudoSelection()) {
            e->releaseSelection();
        }
        TextData::TextMark mark = e->createNewMarkFromCursor();

        const long opticalCursorColumn = e->getOpticalCursorColumn();

        ByteArray whiteSpace;
        whiteSpace.append('\n');

        long currentOpticalColumn = 0;
        long hardTabWidth = e->getLanguageMode()->getHardTabWidth();

        mark.moveToBeginOfLine();
        while (!mark.isEndOfText() && mark.getPos() < e->getCursorTextPosition()) {
            byte c = mark.getChar();
            if (c == ' ' || c == '\t') {
                whiteSpace.append(c);
                if (c == '\t') {
                    currentOpticalColumn = ((currentOpticalColumn / hardTabWidth) + 1) * hardTabWidth;
                } else {
                    currentOpticalColumn += 1;
                }
            } else {
                break;
            }
            mark.inc();
        }
        while (currentOpticalColumn < opticalCursorColumn) {
            whiteSpace.append(' ');
            ++currentOpticalColumn;
        }

        e->hideCursor();
        e->insertAtCursor(whiteSpace);

        if (forward) {
            e->moveCursorToTextPosition(e->getCursorTextPosition() + whiteSpace.getLength());
        }
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
    e->showCursor();
}

void MultiLineEditActions::newLineFixedColumnIndentForward()
{
    newLineFixedColumnIndent(true);
}

void MultiLineEditActions::newLineFixedColumnIndentBackward()
{
    newLineFixedColumnIndent(false);
}


void MultiLineEditActions::findNextLuaStructureElement()
{
    #define LUA_BEGIN_WORDS "function|if|do|repeat"
    #define LUA_END_WORDS   "end|until"
    #define LUA_ALL_WORDS   LUA_BEGIN_WORDS "|" LUA_END_WORDS "|then|else|elseif|while|for"

    if (!e->areCursorChangesDisabled())
    {
        e->hideCursor();

        FindUtil findUtil(e->getTextData());

        long cursorPos = e->getCursorTextPosition();
        
        long beginWordPos   = findUtil.find("(?<=^|\\W)\\w+", cursorPos, FindUtil::Options() | FindUtil::REGEX | FindUtil::BACKWARD);
        long endWordPos     = -1;
        bool isCursorAtWord = false;
        
        if (beginWordPos >= 0) {
            endWordPos = findUtil.getMatchEndPos();
            isCursorAtWord = (beginWordPos <= cursorPos && cursorPos <= endWordPos);
        }
        
        bool wasFound = false;
        
        if (   isCursorAtWord
            && findUtil.doesMatch("\\b(" LUA_ALL_WORDS ")\\b", beginWordPos, FindUtil::Options() | FindUtil::REGEX))
        {
            if (findUtil.doesMatch("\\b(function|repeat|do)\\b", beginWordPos, FindUtil::Options() | FindUtil::REGEX))
            {
                int counter = 1;
                while (counter > 0)
                {
                    if (counter == 1) {
                        beginWordPos = findUtil.find("\\b(" LUA_BEGIN_WORDS "|" LUA_END_WORDS "|then|else|elseif)\\b", endWordPos, FindUtil::Options() | FindUtil::REGEX);
                    } else {
                        beginWordPos = findUtil.find("\\b(" LUA_BEGIN_WORDS "|" LUA_END_WORDS ")\\b", endWordPos, FindUtil::Options() | FindUtil::REGEX);
                    }
                    if (beginWordPos == -1) {
                        break;
                    }
                    endWordPos   = findUtil.getMatchEndPos();

                    if (findUtil.doesMatch("\\b(" LUA_BEGIN_WORDS ")\\b", beginWordPos, FindUtil::Options() | FindUtil::REGEX))
                    {
                        ++counter;
                    }
                    else
                    {
                        --counter;
                    }
                }
                wasFound = (counter == 0);
            }
            else if (findUtil.doesMatch("\\b(if|while|for|elseif)\\b", beginWordPos, FindUtil::Options() | FindUtil::REGEX))
            {
                beginWordPos = findUtil.find("\\b(" LUA_ALL_WORDS ")\\b", endWordPos, FindUtil::Options() | FindUtil::REGEX);
                if (beginWordPos >= 0)
                {
                    endWordPos = findUtil.getMatchEndPos();
                    wasFound = true;
                }
            }
            else if (findUtil.doesMatch("\\b(end|until)\\b", beginWordPos, FindUtil::Options() | FindUtil::REGEX))
            {
                beginWordPos = findUtil.find("\\b(" LUA_ALL_WORDS ")\\b", endWordPos, FindUtil::Options() | FindUtil::REGEX);
                if (beginWordPos >= 0)
                {
                    endWordPos = findUtil.getMatchEndPos();
                    if (!findUtil.doesMatch("\\b(end|until|then|else|elseif)\\b", beginWordPos, FindUtil::Options() | FindUtil::REGEX))
                    {
                        wasFound = true;
                    }
                }
            }
            else if (findUtil.doesMatch("\\b(then|else)\\b", beginWordPos, FindUtil::Options() | FindUtil::REGEX))
            {
                int counter = 1;
                while (counter > 0)
                {
                    if (counter == 1) {
                        beginWordPos = findUtil.find("\\b(" LUA_BEGIN_WORDS "|" LUA_END_WORDS "|then|else|elseif)\\b", endWordPos, FindUtil::Options() | FindUtil::REGEX);
                    } else {
                        beginWordPos = findUtil.find("\\b(" LUA_BEGIN_WORDS "|" LUA_END_WORDS ")\\b", endWordPos, FindUtil::Options() | FindUtil::REGEX);
                    }

                    if (beginWordPos == -1) {
                        break;
                    }
                    endWordPos   = findUtil.getMatchEndPos();

                    if (findUtil.doesMatch("\\b(" LUA_BEGIN_WORDS ")\\b", beginWordPos, FindUtil::Options() | FindUtil::REGEX))
                    {
                        ++counter;
                    }
                    else
                    {
                        --counter;
                    }
                }
                wasFound = (counter == 0);
            }
            else {
                wasFound = true;
            }
        }
        else
        {
            beginWordPos = findUtil.find("\\b(" LUA_ALL_WORDS ")\\b", cursorPos, FindUtil::Options() | FindUtil::REGEX); 
            if (beginWordPos >= 0) {
                wasFound = true;
                endWordPos = findUtil.getMatchEndPos();
            }
        }
        if (wasFound)
        {
            e->moveCursorToTextPosition(endWordPos);
            e->setPrimarySelection(beginWordPos, endWordPos);
        }
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
    e->showCursor();

    #undef LUA_BEGIN_WORDS
    #undef LUA_END_WORDS
    #undef LUA_ALL_WORDS
}



void MultiLineEditActions::findPrevLuaStructureElement()
{
    #define LUA_BEGIN_WORDS "function|if|do|repeat"
    #define LUA_END_WORDS   "end|until"
    #define LUA_ALL_WORDS   LUA_BEGIN_WORDS "|" LUA_END_WORDS "|then|else|elseif|while|for"
    
    if (!e->areCursorChangesDisabled())
    {
        e->hideCursor();

        FindUtil findUtil(e->getTextData());

        long cursorPos = e->getCursorTextPosition();
        
        long beginWordPos   = findUtil.find("(?<=^|\\W)\\w+", cursorPos, FindUtil::Options() | FindUtil::REGEX | FindUtil::BACKWARD);
        long endWordPos     = -1;
        bool isCursorAtWord = false;
        
        if (beginWordPos >= 0) {
            endWordPos = findUtil.getMatchEndPos();
            isCursorAtWord = (beginWordPos <= cursorPos && cursorPos <= endWordPos);
        }
        
        bool wasFound = false;
        
        if (   isCursorAtWord
            && findUtil.doesMatch("\\b(" LUA_ALL_WORDS ")\\b", beginWordPos, FindUtil::Options() | FindUtil::REGEX))
        {
            if (findUtil.doesMatch("\\b(end|until)\\b", beginWordPos, FindUtil::Options() | FindUtil::REGEX))
            {
                int counter = 1;
                while (counter > 0)
                {
                    if (counter == 1) {
                        beginWordPos = findUtil.find("\\b(" LUA_BEGIN_WORDS "|" LUA_END_WORDS "|then|else|elseif|repeat)\\b", beginWordPos - 1, FindUtil::Options() | FindUtil::REGEX | FindUtil::BACKWARD);
                    } else {
                        beginWordPos = findUtil.find("\\b(" LUA_BEGIN_WORDS "|" LUA_END_WORDS ")\\b", beginWordPos - 1, FindUtil::Options() | FindUtil::REGEX | FindUtil::BACKWARD);
                    }

                    if (beginWordPos == -1) {
                        break;
                    }
                    endWordPos   = findUtil.getMatchEndPos();

                    if (findUtil.doesMatch("\\b(" LUA_END_WORDS ")\\b", beginWordPos, FindUtil::Options() | FindUtil::REGEX))
                    {
                        ++counter;
                    }
                    else
                    {
                        --counter;
                    }
                }
                wasFound = (counter == 0);
            }
            else if (findUtil.doesMatch("\\b(then)\\b", beginWordPos, FindUtil::Options() | FindUtil::REGEX))
            {
                beginWordPos = findUtil.find("\\b(" LUA_ALL_WORDS ")\\b", beginWordPos - 1, FindUtil::Options() | FindUtil::REGEX | FindUtil::BACKWARD);
                if (beginWordPos >= 0)
                {
                    endWordPos = findUtil.getMatchEndPos();
                    wasFound = true;
                }
            }
            else if (findUtil.doesMatch("\\b(do|function|if|while|for|repeat)\\b", beginWordPos, FindUtil::Options() | FindUtil::REGEX))
            {
                beginWordPos = findUtil.find("\\b(" LUA_ALL_WORDS ")\\b", beginWordPos - 1, FindUtil::Options() | FindUtil::REGEX | FindUtil::BACKWARD);
                if (beginWordPos >= 0)
                {
                    endWordPos = findUtil.getMatchEndPos();
                    if (!findUtil.doesMatch("\\b(function|if|repeat|do|then|else)\\b", beginWordPos, FindUtil::Options() | FindUtil::REGEX))
                    {
                        wasFound = true;
                    }
                }
            }
            else if (findUtil.doesMatch("\\b(then|else|elseif)\\b", beginWordPos, FindUtil::Options() | FindUtil::REGEX))
            {
                int counter = 1;
                while (counter > 0)
                {
                    if (counter == 1) {
                        beginWordPos = findUtil.find("\\b(" LUA_BEGIN_WORDS "|" LUA_END_WORDS "|then|else|elseif)\\b", beginWordPos - 1, FindUtil::Options() | FindUtil::REGEX | FindUtil::BACKWARD);
                    } else {
                        beginWordPos = findUtil.find("\\b(" LUA_BEGIN_WORDS "|" LUA_END_WORDS ")\\b", beginWordPos - 1, FindUtil::Options() | FindUtil::REGEX | FindUtil::BACKWARD);
                    }

                    if (beginWordPos == -1) {
                        break;
                    }
                    endWordPos   = findUtil.getMatchEndPos();

                    if (findUtil.doesMatch("\\b(" LUA_END_WORDS ")\\b", beginWordPos, FindUtil::Options() | FindUtil::REGEX))
                    {
                        ++counter;
                    }
                    else
                    {
                        --counter;
                    }
                }
                wasFound = (counter == 0);
            }
            else {
                wasFound = true;
            }
        }
        else
        {
            beginWordPos = findUtil.find("\\b(" LUA_ALL_WORDS ")\\b", cursorPos - 1, FindUtil::Options() | FindUtil::REGEX | FindUtil::BACKWARD);
            if (beginWordPos >= 0) {
                wasFound = true;
                endWordPos = findUtil.getMatchEndPos();
            }
        }
        if (wasFound)
        {
            e->moveCursorToTextPosition(beginWordPos);
            e->setPrimarySelection(beginWordPos, endWordPos);
        }
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
    e->showCursor();

    #undef LUA_BEGIN_WORDS
    #undef LUA_END_WORDS
    #undef LUA_ALL_WORDS
}


void MultiLineEditActions::openCorrespondingFile()
{
    RawPtr<TextData> textData = e->getTextData();
    if (!textData->isFileNamePseudo())
    {
        String currentFile = textData->getFileName();
        String correspondingFile;
        
        Regex r("(.*\\.)(cpp|hpp|c|h)((?:\\.emlua)|)");
        
        if (r.matches(currentFile))
        {
            String ext = currentFile.getSubstring(r.getCaptureBegin(2),
                                                  r.getCaptureLength(2));
            String newExt;
            
            if (ext == "c") {
                newExt = "h";
            } else if (ext == "h") {
                newExt = "c";
            } else if (ext == "cpp") {
                newExt = "hpp";
            } else if (ext == "hpp") {
                newExt = "cpp";
            }                                  
            correspondingFile = String() << currentFile.getSubstring(r.getCaptureBegin(1),
                                                                     r.getCaptureLength(1))
                                         << newExt
                                         << currentFile.getSubstring(r.getCaptureBegin(3),
                                                                     r.getCaptureLength(3));
            if (File(correspondingFile).exists())
            {
                FileOpener::start(correspondingFile);
            }
            else
            {
                Regex r2("(.*)(1\\.c|2\\.h)((?:\\.emlua)|)");

                if (r2.matches(currentFile))
                {
                    String ext = currentFile.getSubstring(r2.getCaptureBegin(2),
                                                          r2.getCaptureLength(2));
                    String newExt;
                    
                    if (ext == "1.c") {
                        newExt = "2.h";
                    } else if (ext == "2.h") {
                        newExt = "1.c";
                    }                                  
                    correspondingFile = String() << currentFile.getSubstring(r2.getCaptureBegin(1),
                                                                             r2.getCaptureLength(1))
                                                 << newExt
                                                 << currentFile.getSubstring(r2.getCaptureBegin(3),
                                                                             r2.getCaptureLength(3));
                    if (File(correspondingFile).exists())
                    {
                        FileOpener::start(correspondingFile);
                    }
                }
            }
        }
    }
}
