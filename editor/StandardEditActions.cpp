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


#include <X11/keysym.h>
#include <X11/Xatom.h>

#include "Clipboard.hpp"
#include "StandardEditActions.hpp"
#include "FindUtil.hpp"

using namespace LucED;

StandardEditActions::StandardEditActions(TextEditorWidget *editWidget)
    : e(editWidget)
{}

void StandardEditActions::cursorLeft()
{
    if (!e->areCursorChangesDisabled())
    {
        e->releaseSelectionOwnership();
        long cursorPos = e->getCursorTextPosition();
        if (cursorPos > 0) {
            e->moveCursorToTextPosition(cursorPos - 1);
        }
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
}


void StandardEditActions::cursorRight()
{
    if (!e->areCursorChangesDisabled())
    {
        e->releaseSelectionOwnership();
        long cursorPos = e->getCursorTextPosition();
        if (cursorPos < e->getTextData()->getLength()) {
            e->moveCursorToTextPosition(cursorPos + 1);
        }
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
}

void StandardEditActions::cursorDown()
{
    if (!e->areCursorChangesDisabled())
    {
        e->releaseSelectionOwnership();
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


void StandardEditActions::cursorUp()
{
    if (!e->areCursorChangesDisabled())
    {
        e->releaseSelectionOwnership();
        long pos = e->getCursorTextPosition();
        pos = e->getTextData()->getPrevLineBegin(pos);
        pos = e->getTextPosForPixX(e->getRememberedCursorPixX(), pos);
        e->moveCursorToTextPosition(pos);
    }
    e->assureCursorVisible();
}

void StandardEditActions::gotoMatchingBracket()
{
    if (!e->areCursorChangesDisabled())
    {
        long cursorPos = e->getCursorTextPosition();
        
        if (cursorPos > 0 )
        {
            byte openBracket;
            byte closeBracket;
            bool forward;
            bool hasBracket = false;
            
            TextData* textData = e->getTextData();
            
            switch (textData->getChar(cursorPos - 1)) {
                case '(': hasBracket = true; forward = true; openBracket = '('; closeBracket = ')'; break;
                case '{': hasBracket = true; forward = true; openBracket = '{'; closeBracket = '}'; break;
                case '[': hasBracket = true; forward = true; openBracket = '['; closeBracket = ']'; break;
                case '<': hasBracket = true; forward = true; openBracket = '<'; closeBracket = '>'; break;

                case ')': hasBracket = true; forward = false; openBracket = '('; closeBracket = ')'; break;
                case '}': hasBracket = true; forward = false; openBracket = '{'; closeBracket = '}'; break;
                case ']': hasBracket = true; forward = false; openBracket = '['; closeBracket = ']'; break;
                case '>': hasBracket = true; forward = false; openBracket = '<'; closeBracket = '>'; break;
            }
            if (hasBracket) 
            {
                e->releaseSelectionOwnership();

                long pos;
                
                if (forward) {
                    int counter = 1;
                    pos = cursorPos;
                    const long textLength = textData->getLength();
                    while (pos < textLength && counter != 0) {
                        byte c = textData->getChar(pos);
                        if (c == closeBracket) {
                            --counter;
                        } else if (c == openBracket) {
                            ++counter;
                        }
                        ++pos;
                    }
                    if (counter != 0) {
                        pos = cursorPos;
                    }
                } else {
                    int counter = 1;
                    pos = cursorPos - 2;
                    while (pos > 0 && counter != 0) {
                        byte c = textData->getChar(pos);
                        if (c == openBracket) {
                            --counter;
                        } else if (c == closeBracket) {
                            ++counter;
                        }
                        --pos;
                    }
                    if (counter != 0) {
                        pos = cursorPos;
                    } else {
                        pos += 2;
                    }
                }
                e->moveCursorToTextPosition(pos);
                e->showCursor();
            }
        }
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
}


void StandardEditActions::cursorWordLeft()
{
    if (!e->areCursorChangesDisabled())
    {
        e->releaseSelectionOwnership();

        long cursorPos = e->getCursorTextPosition();
        long pos = cursorPos;
        
        while (pos > 0 && !e->isWordCharacter(e->getTextData()->getChar(pos - 1))) {
            --pos;
        }
        while (pos > 0 && e->isWordCharacter(e->getTextData()->getChar(pos - 1))) {
            --pos;
        }
        e->moveCursorToTextPosition(pos);
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
}


void StandardEditActions::cursorWordRight()
{
    if (!e->areCursorChangesDisabled())
    {
        e->releaseSelectionOwnership();

        long cursorPos = e->getCursorTextPosition();
        long len = e->getTextData()->getLength();

        long pos = cursorPos;

#if 0        
        bool gotoEndOfWordFlag = true;
        if ((pos == 0 || !e->isWordCharacter(e->getTextData()->getChar(pos - 1)))
          && !e->isWordCharacter(e->getTextData()->getChar(pos))) {
            gotoEndOfWordFlag = false;
        }
        while (pos < len && !e->isWordCharacter(e->getTextData()->getChar(pos))) {
            ++pos;
        }
        if (gotoEndOfWordFlag) {
            while (pos < len && e->isWordCharacter(e->getTextData()->getChar(pos))) {
                ++pos;
            }
        }
#else
        if (pos < len) {
            ++pos;
            if (pos > 0) {
                while (pos < len 
                  && !(  !e->isWordCharacter(e->getTextData()->getChar(pos - 1))
                       && e->isWordCharacter(e->getTextData()->getChar(pos))))
                {
                    ++pos;
                }
            }
        }
#endif
        e->moveCursorToTextPosition(pos);
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
}



void StandardEditActions::selectionCursorLeft()
{
    if (!e->areCursorChangesDisabled())
    {
        long cursorPos = e->getCursorTextPosition();
        if (!e->hasSelectionOwnership()) {
            e->requestSelectionOwnership();
            e->getBackliteBuffer()->activateSelection(cursorPos);
        }
        if (cursorPos > 0) {
            e->moveCursorToTextPosition(cursorPos - 1);
        }
        e->getBackliteBuffer()->extendSelectionTo(e->getCursorTextPosition());
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
}


void StandardEditActions::selectionCursorRight()
{
    if (!e->areCursorChangesDisabled())
    {
        long cursorPos = e->getCursorTextPosition();
        if (!e->hasSelectionOwnership()) {
            e->requestSelectionOwnership();
            e->getBackliteBuffer()->activateSelection(cursorPos);
        }
        if (cursorPos < e->getTextData()->getLength()) {
            e->moveCursorToTextPosition(cursorPos + 1);
        }
        e->getBackliteBuffer()->extendSelectionTo(e->getCursorTextPosition());
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
}

void StandardEditActions::selectionCursorDown()
{
    if (!e->areCursorChangesDisabled())
    {
        long pos = e->getCursorTextPosition();
        if (!e->hasSelectionOwnership()) {
            e->requestSelectionOwnership();
            e->getBackliteBuffer()->activateSelection(pos);
        }
        pos = e->getTextData()->getNextLineBegin(pos);
        if (e->getTextData()->isBeginOfLine(pos)) {
            pos = e->getTextPosForPixX(e->getRememberedCursorPixX(), pos);
            e->moveCursorToTextPosition(pos);
        } else {
            // Cursor is in last line
        }
        e->getBackliteBuffer()->extendSelectionTo(e->getCursorTextPosition());
    }
    e->assureCursorVisible();
}


void StandardEditActions::selectionCursorUp()
{
    if (!e->areCursorChangesDisabled())
    {
        long pos = e->getCursorTextPosition();
        if (!e->hasSelectionOwnership()) {
            e->requestSelectionOwnership();
            e->getBackliteBuffer()->activateSelection(pos);
        }
        pos = e->getTextData()->getPrevLineBegin(pos);
        pos = e->getTextPosForPixX(e->getRememberedCursorPixX(), pos);
        e->moveCursorToTextPosition(pos);
        e->getBackliteBuffer()->extendSelectionTo(e->getCursorTextPosition());
    }
    e->assureCursorVisible();
}


void StandardEditActions::selectionLineCursorDown()
{
    if (!e->areCursorChangesDisabled())
    {
        long cursorPos = e->getCursorTextPosition();
        long nextLineBeginPos = e->getTextData()->getNextLineBegin(cursorPos);
        
        if (e->hasSelectionOwnership())
        {
            long spos1 = e->getBackliteBuffer()->getBeginSelectionPos();
            long spos2 = e->getTextData()->getThisLineBegin(spos1);
            if (spos1 != spos2) {
                bool wasAnchorAtBegin = e->getBackliteBuffer()->isAnchorAtBegin();
                e->getBackliteBuffer()->setAnchorToEndOfSelection();
                e->getBackliteBuffer()->extendSelectionTo(spos2);
                if (wasAnchorAtBegin) {
                    e->getBackliteBuffer()->setAnchorToBeginOfSelection();
                }
            }
            long epos1 = e->getBackliteBuffer()->getEndSelectionPos();
            if (!e->getTextData()->isBeginOfLine(epos1)) {
                long epos2 = e->getTextData()->getNextLineBegin(epos1);
                if (epos1 != epos2) {
                    bool wasAnchorAtBegin = e->getBackliteBuffer()->isAnchorAtBegin();
                    e->getBackliteBuffer()->setAnchorToBeginOfSelection();
                    e->getBackliteBuffer()->extendSelectionTo(epos2);
                    if (!wasAnchorAtBegin) {
                        e->getBackliteBuffer()->setAnchorToEndOfSelection();
                    }
                    epos1 = epos2;
                }
            }
            if (epos1 == nextLineBeginPos) {
                e->getBackliteBuffer()->setAnchorToBeginOfSelection();
            }
        } else {
            e->requestSelectionOwnership();
            long pos1 = e->getTextData()->getThisLineBegin(cursorPos);
            e->getBackliteBuffer()->activateSelection(pos1);
        }

        if (e->getTextData()->isBeginOfLine(nextLineBeginPos)) {
            cursorPos = e->getTextPosForPixX(e->getRememberedCursorPixX(), nextLineBeginPos);
            e->moveCursorToTextPosition(cursorPos);
        } else {
            // Cursor is in last line
        }
        long pos2;
        if (e->getBackliteBuffer()->isAnchorAtBegin()) {
            pos2 = e->getTextData()->getNextLineBegin(cursorPos);
        } else {
            pos2 = e->getTextData()->getThisLineBegin(cursorPos);
        }
        e->getBackliteBuffer()->extendSelectionTo(pos2);
    }
    e->assureCursorVisible();
}


void StandardEditActions::selectionLineCursorUp()
{
    if (!e->areCursorChangesDisabled())
    {
        long cursorPos = e->getCursorTextPosition();
        long prevLineBeginPos = e->getTextData()->getPrevLineBegin(cursorPos);
        long nextLineBeginPos = e->getTextData()->getNextLineBegin(cursorPos);
        if (e->hasSelectionOwnership())
        {
            long spos1 = e->getBackliteBuffer()->getBeginSelectionPos();
            long spos2 = e->getTextData()->getThisLineBegin(spos1);
            if (spos1 != spos2) {
                bool wasAnchorAtBegin = e->getBackliteBuffer()->isAnchorAtBegin();
                e->getBackliteBuffer()->setAnchorToEndOfSelection();
                e->getBackliteBuffer()->extendSelectionTo(spos2);
                if (wasAnchorAtBegin) {
                    e->getBackliteBuffer()->setAnchorToBeginOfSelection();
                }
            }
            if (spos2 == e->getTextData()->getThisLineBegin(cursorPos)) {
                e->getBackliteBuffer()->setAnchorToEndOfSelection();
            }
            long epos1 = e->getBackliteBuffer()->getEndSelectionPos();
            if (!e->getTextData()->isBeginOfLine(epos1)) {
                long epos2 = e->getTextData()->getNextLineBegin(epos1);
                if (epos1 != epos2) {
                    bool wasAnchorAtBegin = e->getBackliteBuffer()->isAnchorAtBegin();
                    e->getBackliteBuffer()->setAnchorToBeginOfSelection();
                    e->getBackliteBuffer()->extendSelectionTo(epos2);
                    if (!wasAnchorAtBegin) {
                        e->getBackliteBuffer()->setAnchorToEndOfSelection();
                    }
                    epos1 = epos2;
                }
            }
        } else {
            e->requestSelectionOwnership();
            e->getBackliteBuffer()->activateSelection(nextLineBeginPos);
            e->getBackliteBuffer()->setAnchorToEndOfSelection();
        }

        cursorPos = e->getTextPosForPixX(e->getRememberedCursorPixX(), prevLineBeginPos);
        e->moveCursorToTextPosition(cursorPos);
        long pos2;
        if (e->getBackliteBuffer()->isAnchorAtBegin()) {
            pos2 = e->getTextData()->getNextLineBegin(cursorPos);
        } else {
            pos2 = e->getTextData()->getThisLineBegin(cursorPos);
        }
        e->getBackliteBuffer()->extendSelectionTo(pos2);
    }
    e->assureCursorVisible();
}


void StandardEditActions::selectionCursorWordLeft()
{
    if (!e->areCursorChangesDisabled())
    {
        long cursorPos = e->getCursorTextPosition();
        if (!e->hasSelectionOwnership()) {
            e->requestSelectionOwnership();
            e->getBackliteBuffer()->activateSelection(cursorPos);
        }
        long pos = cursorPos;
        while (pos > 0 && !e->isWordCharacter(e->getTextData()->getChar(pos - 1))) {
            --pos;
        }
        while (pos > 0 && e->isWordCharacter(e->getTextData()->getChar(pos - 1))) {
            --pos;
        }
        e->moveCursorToTextPosition(pos);
        e->getBackliteBuffer()->extendSelectionTo(e->getCursorTextPosition());
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
}


void StandardEditActions::selectionCursorWordRight()
{
    if (!e->areCursorChangesDisabled())
    {
        long cursorPos = e->getCursorTextPosition();
        if (!e->hasSelectionOwnership()) {
            e->requestSelectionOwnership();
            e->getBackliteBuffer()->activateSelection(cursorPos);
        }
        long pos = cursorPos;
        long len = e->getTextData()->getLength();

#if 0
        bool gotoEndOfWordFlag = true;
        if ((pos == 0 || !e->isWordCharacter(e->getTextData()->getChar(pos - 1)))
          && !e->isWordCharacter(e->getTextData()->getChar(pos))) {
            gotoEndOfWordFlag = false;
        }
        while (pos < len && !e->isWordCharacter(e->getTextData()->getChar(pos))) {
            ++pos;
        }
        if (gotoEndOfWordFlag) {
            while (pos < len && e->isWordCharacter(e->getTextData()->getChar(pos))) {
                ++pos;
            }
        }
#else
        if (pos < len) {
            ++pos;
            if (pos > 0) {
                while (pos < len 
                  && !(  !e->isWordCharacter(e->getTextData()->getChar(pos - 1))
                       && e->isWordCharacter(e->getTextData()->getChar(pos))))
                {
                    ++pos;
                }
            }
        }
#endif
        e->moveCursorToTextPosition(pos);
        e->getBackliteBuffer()->extendSelectionTo(e->getCursorTextPosition());
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
}


void StandardEditActions::shiftBlockLeft()
{
    if (!e->areCursorChangesDisabled() && e->getBackliteBuffer()->hasActiveSelection())
    {
        TextData* textData                                = e->getTextData();
        EditingHistory::SectionHolder::Ptr historySection = textData->createHistorySection();
        
        TextData::TextMark mark = e->getBackliteBuffer()->createMarkToBeginOfSelection();
        long endPos       = e->getBackliteBuffer()->getEndSelectionPos();
        
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

void StandardEditActions::shiftBlockRight()
{
    if (!e->areCursorChangesDisabled() && e->getBackliteBuffer()->hasActiveSelection())
    {
        TextData* textData                                = e->getTextData();
        EditingHistory::SectionHolder::Ptr historySection = textData->createHistorySection();
        
        TextData::TextMark mark = e->getBackliteBuffer()->createMarkToBeginOfSelection();
        long endPos       = e->getBackliteBuffer()->getEndSelectionPos();
        
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

void StandardEditActions::cursorPageDown()
{
    if (!e->areCursorChangesDisabled())
    {
        e->releaseSelectionOwnership();
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


void StandardEditActions::cursorPageUp()
{
    if (!e->areCursorChangesDisabled())
    {
        e->releaseSelectionOwnership();
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

void StandardEditActions::selectionCursorPageDown()
{
    if (!e->areCursorChangesDisabled())
    {
        long cursorPos = e->getCursorTextPosition();
        if (!e->hasSelectionOwnership()) {
            e->requestSelectionOwnership();
            e->getBackliteBuffer()->activateSelection(cursorPos);
        }

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
        e->getBackliteBuffer()->extendSelectionTo(e->getCursorTextPosition());
    }
    e->assureCursorVisible();
}


void StandardEditActions::selectionCursorPageUp()
{
    if (!e->areCursorChangesDisabled())
    {
        long cursorPos = e->getCursorTextPosition();
        if (!e->hasSelectionOwnership()) {
            e->requestSelectionOwnership();
            e->getBackliteBuffer()->activateSelection(cursorPos);
        }

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
        e->getBackliteBuffer()->extendSelectionTo(e->getCursorTextPosition());
    }
    e->assureCursorVisible();
}

void StandardEditActions::cursorBeginOfLine()
{
    if (!e->areCursorChangesDisabled())
    {
        e->releaseSelectionOwnership();
        TextData::TextMark mark = e->createNewMarkFromCursor();
        mark.moveToBeginOfLine();
        e->moveCursorToTextMark(mark);
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
}


void StandardEditActions::cursorEndOfLine()
{
    if (!e->areCursorChangesDisabled())
    {
        e->releaseSelectionOwnership();
        TextData::TextMark mark = e->createNewMarkFromCursor();
        mark.moveToEndOfLine();
        e->moveCursorToTextMark(mark);
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
}

void StandardEditActions::selectionCursorBeginOfLine()
{
    if (!e->areCursorChangesDisabled())
    {
        long cursorPos = e->getCursorTextPosition();
        if (!e->hasSelectionOwnership()) {
            e->requestSelectionOwnership();
            e->getBackliteBuffer()->activateSelection(cursorPos);
        }

        TextData::TextMark mark = e->createNewMarkFromCursor();
        mark.moveToBeginOfLine();
        e->moveCursorToTextMark(mark);
        e->getBackliteBuffer()->extendSelectionTo(e->getCursorTextPosition());
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
}


void StandardEditActions::selectionCursorEndOfLine()
{
    if (!e->areCursorChangesDisabled())
    {
        long cursorPos = e->getCursorTextPosition();
        if (!e->hasSelectionOwnership()) {
            e->requestSelectionOwnership();
            e->getBackliteBuffer()->activateSelection(cursorPos);
        }

        TextData::TextMark mark = e->createNewMarkFromCursor();
        mark.moveToEndOfLine();
        e->moveCursorToTextMark(mark);
        e->getBackliteBuffer()->extendSelectionTo(e->getCursorTextPosition());
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
}


void StandardEditActions::cursorBeginOfText()
{
    if (!e->areCursorChangesDisabled())
    {
        e->releaseSelectionOwnership();
        e->moveCursorToTextPosition(0);
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
}


void StandardEditActions::cursorEndOfText()
{
    if (!e->areCursorChangesDisabled())
    {
        e->releaseSelectionOwnership();
        e->moveCursorToTextPosition(e->getTextData()->getLength());
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
}


void StandardEditActions::scrollDown()
{
    e->scrollDown();
}

void StandardEditActions::scrollCursorDown()
{
    e->assureCursorVisible();
    if (!e->areCursorChangesDisabled())
    {
        e->hideCursor();
        e->releaseSelectionOwnership();
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


void StandardEditActions::scrollUp()
{
    e->scrollUp();
}

void StandardEditActions::scrollCursorUp()
{
    e->assureCursorVisible();
    if (!e->areCursorChangesDisabled())
    {
        e->hideCursor();
        e->releaseSelectionOwnership();
        if (e->scrollUp()) {
            long pos = e->getCursorTextPosition();
            pos = e->getTextData()->getPrevLineBegin(pos);
            pos = e->getTextPosForPixX(e->getRememberedCursorPixX(), pos);
            e->moveCursorToTextPosition(pos);
        }
        e->showCursor();
    }
}


void StandardEditActions::scrollLeft()
{
    e->scrollLeft();
}


void StandardEditActions::scrollRight()
{
    e->scrollRight();
}


void StandardEditActions::scrollPageUp()
{
    e->scrollPageUp();
}


void StandardEditActions::scrollPageDown()
{
    e->scrollPageDown();
}


void StandardEditActions::scrollPageLeft()
{
    e->scrollPageLeft();
}


void StandardEditActions::scrollPageRight()
{
    e->scrollPageRight();
}


void StandardEditActions::newLine()
{
    if (!e->areCursorChangesDisabled())
    {
        e->setCurrentAction(TextEditorWidget::ACTION_NEWLINE);
        
        EditingHistory::SectionHolder::Ptr historySectionHolder = e->getTextData()->createHistorySection();

        if (e->hasSelectionOwnership()) {
            long selBegin = e->getBackliteBuffer()->getBeginSelectionPos();
            long selLength = e->getBackliteBuffer()->getEndSelectionPos() - selBegin;
            e->moveCursorToTextPosition(selBegin);
            e->removeAtCursor(selLength);
            e->releaseSelectionOwnership();
        } else if (e->getBackliteBuffer()->hasActiveSelection()) {
            e->getBackliteBuffer()->deactivateSelection();
        }
        TextData::TextMark mark = e->createNewMarkFromCursor();
        ByteArray whiteSpace;
        whiteSpace.append('\n');

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

        e->hideCursor();
        e->insertAtCursor(whiteSpace);
        e->moveCursorToTextPosition(e->getCursorTextPosition() + whiteSpace.getLength());
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
    e->showCursor();
}

void StandardEditActions::tabForward()
{
    if (!e->areCursorChangesDisabled())
    {
        e->getTextData()->setMergableHistorySeparator();
        EditingHistory::SectionHolder::Ptr historySection = e->getTextData()->getHistorySectionHolder();
        
        if (e->hasSelectionOwnership()) {
            long selBegin = e->getBackliteBuffer()->getBeginSelectionPos();
            long selLength = e->getBackliteBuffer()->getEndSelectionPos() - selBegin;
            e->moveCursorToTextPosition(selBegin);
            e->removeAtCursor(selLength);
            e->releaseSelectionOwnership();
        } else if (e->getBackliteBuffer()->hasActiveSelection()) {
            if (   (   e->getLastAction() != TextEditorWidget::ACTION_KEYBOARD_INPUT 
                    && e->getLastAction() != TextEditorWidget::ACTION_TABULATOR)
                || e->getCursorTextPosition() != e->getBackliteBuffer()->getEndSelectionPos())
            {
                e->getBackliteBuffer()->deactivateSelection();
            }
        }
        TextData::TextMark mark = e->createNewMarkFromCursor();
        ByteArray whiteSpace;

        long softTabWidth = e->getLanguageMode()->getSoftTabWidth();

        if (softTabWidth <= 0) {
            whiteSpace.append('\t');
        } else {
            long opticalCursorColumn = e->getOpticalCursorColumn();
            long newOpticalCursorColumn = ((opticalCursorColumn / softTabWidth) + 1) * softTabWidth;
            long diff = newOpticalCursorColumn - opticalCursorColumn;
            ASSERT(diff > 0);
            whiteSpace.appendAndFillAmountWith(diff, ' ');
        }
        

        e->hideCursor();
        e->insertAtCursor(whiteSpace);
        e->moveCursorToTextPosition(e->getCursorTextPosition() + whiteSpace.getLength());
        e->setCurrentAction(TextEditorWidget::ACTION_TABULATOR);
        if (e->getBackliteBuffer()->hasActiveSelection()) {
            e->getBackliteBuffer()->extendSelectionTo(e->getCursorTextPosition());
        }
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
    e->showCursor();
}



void StandardEditActions::backSpace()
{
    if (!e->areCursorChangesDisabled())
    {

        e->hideCursor();
        if (e->hasSelectionOwnership())
        {
            {
                EditingHistory::SectionHolder::Ptr historySectionHolder = e->getTextData()->createHistorySection();

                long selBegin = e->getBackliteBuffer()->getBeginSelectionPos();
                long selLength = e->getBackliteBuffer()->getEndSelectionPos() - selBegin;
                e->moveCursorToTextPosition(selBegin);
                e->removeAtCursor(selLength);
                e->releaseSelectionOwnership();
            }
            e->getTextData()->setHistorySeparator();
        }
        else
        {
            e->getTextData()->setMergableHistorySeparator();
            EditingHistory::SectionHolder::Ptr historySection = e->getTextData()->getHistorySectionHolder();

            const long cursorPos = e->getCursorTextPosition(); 

            if (    (   e->getLastAction() != TextEditorWidget::ACTION_KEYBOARD_INPUT 
                     && e->getLastAction() != TextEditorWidget::ACTION_TABULATOR)
                 && e->getBackliteBuffer()->hasActiveSelection())
            {
                e->getBackliteBuffer()->deactivateSelection();
            }
            e->setCurrentAction(TextEditorWidget::ACTION_KEYBOARD_INPUT);

            
            if (cursorPos > 0)
            {
                long softTabWidth = e->getLanguageMode()->getSoftTabWidth();
                long amountToBeRemoved;
                
                if (softTabWidth <= 0 || (   e->getLastAction() != TextEditorWidget::ACTION_TABULATOR
                                          && e->getLastAction() != TextEditorWidget::ACTION_NEWLINE))
                {
                    amountToBeRemoved = 1;
                }
                else 
                {
#if 0
                    TextData* textData = e->getTextData();
                    bool wasAllSpace = true;
                    for (long p = cursorPos - e->getCursorColumn(); p < cursorPos; ++p) {
                        byte c = textData->getChar(p);
                        if (c != ' ') {
                            wasAllSpace = false;
                            break;
                        }
                    }
                    if (wasAllSpace && e->getCursorColumn() > 0) {
                        long opticalCursorColumn = e->getOpticalCursorColumn();
                        long newOpticalCursorColumn = ((opticalCursorColumn - 1) / softTabWidth) * softTabWidth;
                        amountToBeRemoved = opticalCursorColumn - newOpticalCursorColumn;
                        ASSERT(amountToBeRemoved > 0);
                    } else {
                        amountToBeRemoved = 1;
                    }
#else
                    TextData* textData = e->getTextData();
                    long p             = cursorPos;
                    long opticalColumn = e->getOpticalCursorColumn();
                    
                    while (p - 1 >= 0 && textData->getChar(p - 1) == ' ') {
                        p             -= 1;
                        opticalColumn -= 1;
                        if ((opticalColumn / softTabWidth) * softTabWidth == opticalColumn) {
                            break;
                        }
                    }
                    if (p == cursorPos && p > 0) {
                        p -= 1;
                    }
                    amountToBeRemoved = cursorPos - p;
                    e->setCurrentAction(TextEditorWidget::ACTION_TABULATOR);
#endif
                }
                e->moveCursorToTextPosition(cursorPos - amountToBeRemoved);
                e->removeAtCursor(amountToBeRemoved);
            }
            
        }
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
    e->showCursor();
}

void StandardEditActions::deleteKey()
{
    if (!e->areCursorChangesDisabled())
    {
        e->hideCursor();
        if (e->hasSelectionOwnership()) 
        {
            {
                EditingHistory::SectionHolder::Ptr historySectionHolder = e->getTextData()->createHistorySection();

                long selBegin = e->getBackliteBuffer()->getBeginSelectionPos();
                long selLength = e->getBackliteBuffer()->getEndSelectionPos() - selBegin;
                e->moveCursorToTextPosition(selBegin);
                e->removeAtCursor(selLength);
                e->releaseSelectionOwnership();
            }
            e->getTextData()->setHistorySeparator();
        }
        else
        {
            e->getTextData()->setMergableHistorySeparator();
            EditingHistory::SectionHolder::Ptr historySection = e->getTextData()->getHistorySectionHolder();
            
            if (e->getBackliteBuffer()->hasActiveSelection()) {
                e->getBackliteBuffer()->deactivateSelection();
            }
            e->removeAtCursor(1);
        }
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
    e->showCursor();
}


void StandardEditActions::copyToClipboard()
{
    if (!e->areCursorChangesDisabled())
    {
        EditingHistory::SectionHolder::Ptr historySectionHolder = e->getTextData()->createHistorySection();

        if (e->hasSelectionOwnership() && e->getBackliteBuffer()->hasActiveSelection()) {
            long selBegin = e->getBackliteBuffer()->getBeginSelectionPos();
            long selLength = e->getBackliteBuffer()->getEndSelectionPos() - selBegin;
            Clipboard::getInstance()->copyToClipboard(e->getTextData()->getAmount(selBegin, selLength), selLength);
        } else {
            Clipboard::getInstance()->copyActiveSelectionToClipboard();
        }
    }
}

void StandardEditActions::cutToClipboard()
{
    if (!e->areCursorChangesDisabled())
    {
        if (e->getBackliteBuffer()->hasActiveSelection() && e->getBackliteBuffer()->isSelectionPrimary())
        {
            {
                EditingHistory::SectionHolder::Ptr historySectionHolder = e->getTextData()->createHistorySection();

                long selBegin = e->getBackliteBuffer()->getBeginSelectionPos();
                long selLength = e->getBackliteBuffer()->getEndSelectionPos() - selBegin;
                Clipboard::getInstance()->copyToClipboard(e->getTextData()->getAmount(selBegin, selLength), selLength);
                e->moveCursorToTextPosition(selBegin);
                e->removeAtCursor(selLength);
                e->releaseSelectionOwnership();
            }
            e->getTextData()->setHistorySeparator();
        }
    }
}

void StandardEditActions::selectAll()
{
    if (!e->areCursorChangesDisabled() && e->getTextData()->getLength() > 0)
    {
        if (!e->hasSelectionOwnership() && e->getBackliteBuffer()->hasActiveSelection()) { 
            if (e->requestSelectionOwnership()) {
                e->getBackliteBuffer()->makeSecondarySelectionToPrimarySelection();
            }
        } else {
            if (e->requestSelectionOwnership()) {
                e->getBackliteBuffer()->activateSelection(0);
                e->getBackliteBuffer()->extendSelectionTo(e->getTextData()->getLength());
            }
        }
    }
}


bool doAbort = false;

void StandardEditActions::pasteFromClipboard()
{
    if (!e->areCursorChangesDisabled())
    {
//doAbort = true;        
        EditingHistory::SectionHolder::Ptr historySection = e->getTextData()->createHistorySection();

        TextData::TextMark m;

        if (e->hasSelectionOwnership()) {
            m = e->getBackliteBuffer()->createMarkToBeginOfSelection();
            long selLength = e->getBackliteBuffer()->getEndSelectionPos() - m.getPos();
            e->getTextData()->removeAtMark(m, selLength);
//            e->releaseSelectionOwnership();
        } else {
            m = e->createNewMarkFromCursor();
        }
//        if (e->getBackliteBuffer()->hasActiveSelection()) {
//            e->getBackliteBuffer()->deactivateSelection();
//        }
        e->requestClipboardPasting(m);
//doAbort = false;        
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
}

void StandardEditActions::undo()
{
    if (!e->areCursorChangesDisabled())
    {
        e->hideCursor();
        TextData::TextMark mark = e->createNewMarkFromCursor();
        long length = e->getTextData()->undo(mark);
        e->moveCursorToTextMark(mark);
        if (length > 0) {
            e->requestSelectionOwnership();
            if (e->hasSelectionOwnership()) {
                long cursorPos = e->getCursorTextPosition();
                e->getBackliteBuffer()->activateSelection(cursorPos);
                e->moveCursorToTextPosition(cursorPos + length);
                e->getBackliteBuffer()->extendSelectionTo(e->getCursorTextPosition());
            }
        } else {
            if (e->hasSelectionOwnership()) {
                e->releaseSelectionOwnership();
            }
            if (e->getBackliteBuffer()->hasActiveSelection() && !e->getBackliteBuffer()->isSelectionPrimary()) {
                e->getBackliteBuffer()->deactivateSelection();
            }
        }
        e->adjustCursorVisibility();
    }
    else {
        e->assureCursorVisible();
    }
    e->rememberCursorPixX();
    e->showCursor();
}

void StandardEditActions::redo()
{
    if (!e->areCursorChangesDisabled())
    {
        TextData::TextMark mark = e->createNewMarkFromCursor();
        long length = e->getTextData()->redo(mark);
        e->moveCursorToTextMark(mark);
        if (length > 0) {
            e->requestSelectionOwnership();
            if (e->hasSelectionOwnership()) {
                long cursorPos = e->getCursorTextPosition();
                e->getBackliteBuffer()->activateSelection(cursorPos);
                e->moveCursorToTextPosition(cursorPos + length);
                e->getBackliteBuffer()->extendSelectionTo(e->getCursorTextPosition());
            }
        } else {
            if (e->hasSelectionOwnership()) {
                e->releaseSelectionOwnership();
            }
        }
        e->adjustCursorVisibility();
    }
    else {
        e->assureCursorVisible();
    }
    e->rememberCursorPixX();
}


void StandardEditActions::selectWordForward()
{
    if (!e->areCursorChangesDisabled())
    {
        TextData*       textData       = e->getTextData();
        BackliteBuffer* backliteBuffer = e->getBackliteBuffer();
        
        long len  = textData->getLength();
        long cursorPos = e->getCursorTextPosition();
        long spos = cursorPos;
        long epos = cursorPos;
        
        bool extended = false;
        
        if (e->hasSelectionOwnership())
        {
            spos = backliteBuffer->getBeginSelectionPos();
            epos = backliteBuffer->getEndSelectionPos();
            
            const long spos0 = spos;
            
            while (spos - 1 >= 0
                   && e->isWordCharacter(textData->getChar(spos - 1)))
            {
                --spos;
            }

            if (spos == spos0)
            {
                while (epos < len
                       && !e->isWordCharacter(textData->getChar(epos)))
                {
                    ++epos;
                }
            }
            while (epos < len
                   && e->isWordCharacter(textData->getChar(epos)))
            {
                ++epos;
            }
            extended = true;
        }
        else
        {
            e->requestSelectionOwnership();
            spos = cursorPos;
            epos = cursorPos;

            while (spos - 1 >= 0
                   && e->isWordCharacter(textData->getChar(spos - 1)))
            {
                --spos;
            }
            while (epos < len
                   && e->isWordCharacter(textData->getChar(epos)))
            {
                ++epos;
            }
        }
        
        if (!backliteBuffer->hasActiveSelection()) {
            backliteBuffer->activateSelection(spos);
        } else {
            backliteBuffer->makeSecondarySelectionToPrimarySelection();
            if (backliteBuffer->getBeginSelectionPos() != spos)
            {
                backliteBuffer->setAnchorToEndOfSelection();
                backliteBuffer->extendSelectionTo(spos);
            }
        }
        backliteBuffer->setAnchorToBeginOfSelection();
        backliteBuffer->extendSelectionTo(epos);
        if (extended) {
            e->moveCursorToTextPosition(epos);
        }
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
}

void StandardEditActions::selectWordBackward()
{
    if (!e->areCursorChangesDisabled())
    {
        TextData*       textData       = e->getTextData();
        BackliteBuffer* backliteBuffer = e->getBackliteBuffer();

        long len  = textData->getLength();
        long cursorPos = e->getCursorTextPosition();
        long spos = cursorPos;
        long epos = cursorPos;

        if (e->hasSelectionOwnership()) {
            backliteBuffer->setAnchorToBeginOfSelection();
            spos = backliteBuffer->getBeginSelectionPos();
            epos = backliteBuffer->getEndSelectionPos();
        
            while (epos - 1 >= 0
                   && e->isWordCharacter(textData->getChar(epos - 1)))
            {
                --epos;
            }
            while (epos - 1 >= 0
                   && !e->isWordCharacter(textData->getChar(epos - 1)))
            {
                --epos;
            }
            if (epos > spos)
            {
                backliteBuffer->extendSelectionTo(epos);
                e->moveCursorToTextPosition(epos);
            }
        }
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
}


void StandardEditActions::findNextStructureElement()
{
//    long cursorPos = e->getCursorTextPosition();
//    
//    FindUtil findUtil;
//    findUtil.setTextData                 (e->getTextData());
//    findUtil.setTextPosition             (cursorPos);
//    findUtil.setSearchString             ("\b(if|do|function|repeat|then|while|elseif|else)\b");
//    findUtil.setRegexFlag                (true);
//    findUtil.setWholeWordFlag            (false);
//    findUtil.setIgnoreCaseFlag           (false);
//    findUtil.setSearchForwardFlag        (false);
//    
//    findUtil.findNext();
//    
//    if (findUtil.wasFound() && findUtil.getMatchEndPos() >= cursorPos)
//    {
//        int counter = 1;
//        while (counter > 0)
//        {
//            if (counter == 1) {
//                findUtil.setSearchString("\b(if|do|function|repeat|then|while|elseif|else|end|until)\b");
//            } else {
//                findUtil.setSearchString("\b(if|do|function|repeat|end|until)\b");
//            }
//            if (!findUtil.wasFound()) {
//                break;
//            }
//            
//        }
//    }
//    else
//    {
//        findUtil.setSearchString         ("\b(end|until)\b");
//    }
}


void StandardEditActions::findPrevStructureElement()
{
}


void StandardEditActions::registerSingleLineEditActionsToEditWidget()
{
    e->setEditAction(                    0, XK_Left,      this, &StandardEditActions::cursorLeft);
    e->setEditAction(                    0, XK_Right,     this, &StandardEditActions::cursorRight);
    e->setEditAction(                    0, XK_KP_Left,   this, &StandardEditActions::cursorLeft);
    e->setEditAction(                    0, XK_KP_Right,  this, &StandardEditActions::cursorRight);

    e->setEditAction(             Mod1Mask, XK_Left,      this, &StandardEditActions::cursorBeginOfLine);
    e->setEditAction(             Mod1Mask, XK_Right,     this, &StandardEditActions::cursorEndOfLine);
    e->setEditAction(             Mod1Mask, XK_KP_Left,   this, &StandardEditActions::cursorBeginOfLine);
    e->setEditAction(             Mod1Mask, XK_KP_Right,  this, &StandardEditActions::cursorEndOfLine);

    e->setEditAction(                    0, XK_Home,      this, &StandardEditActions::cursorBeginOfLine);
    e->setEditAction(                    0, XK_Begin,     this, &StandardEditActions::cursorBeginOfLine);
    e->setEditAction(                    0, XK_End,       this, &StandardEditActions::cursorEndOfLine);

    e->setEditAction( ControlMask|Mod1Mask, XK_Left,      this, &StandardEditActions::scrollLeft);
    e->setEditAction( ControlMask|Mod1Mask, XK_Right,     this, &StandardEditActions::scrollRight);
    e->setEditAction( ControlMask|Mod1Mask, XK_KP_Left,   this, &StandardEditActions::scrollLeft);
    e->setEditAction( ControlMask|Mod1Mask, XK_KP_Right,  this, &StandardEditActions::scrollRight);

    e->setEditAction(          ControlMask, XK_Home,      this, &StandardEditActions::cursorBeginOfText);
    e->setEditAction(          ControlMask, XK_Begin,     this, &StandardEditActions::cursorBeginOfText);
    e->setEditAction(          ControlMask, XK_End,       this, &StandardEditActions::cursorEndOfText);

    e->setEditAction(                    0, XK_BackSpace, this, &StandardEditActions::backSpace);
    e->setEditAction(                    0, XK_Delete,    this, &StandardEditActions::deleteKey);

    e->setEditAction(          ControlMask, XK_c,         this, &StandardEditActions::copyToClipboard);
    e->setEditAction(          ControlMask, XK_x,         this, &StandardEditActions::cutToClipboard);
    e->setEditAction(          ControlMask, XK_v,         this, &StandardEditActions::pasteFromClipboard);
    e->setEditAction(          ControlMask, XK_a,         this, &StandardEditActions::selectAll);

    e->setEditAction(            ShiftMask, XK_Left,      this, &StandardEditActions::selectionCursorLeft);
    e->setEditAction(            ShiftMask, XK_Right,     this, &StandardEditActions::selectionCursorRight);
    e->setEditAction(            ShiftMask, XK_KP_Left,   this, &StandardEditActions::selectionCursorLeft);
    e->setEditAction(            ShiftMask, XK_KP_Right,  this, &StandardEditActions::selectionCursorRight);
    
    e->setEditAction(          ControlMask, XK_Left,      this, &StandardEditActions::cursorWordLeft);
    e->setEditAction(          ControlMask, XK_Right,     this, &StandardEditActions::cursorWordRight);
    e->setEditAction(ShiftMask|ControlMask, XK_Left,      this, &StandardEditActions::selectionCursorWordLeft);
    e->setEditAction(ShiftMask|ControlMask, XK_Right,     this, &StandardEditActions::selectionCursorWordRight);

    e->setEditAction(            ShiftMask, XK_Home,      this, &StandardEditActions::selectionCursorBeginOfLine);
    e->setEditAction(            ShiftMask, XK_Begin,     this, &StandardEditActions::selectionCursorBeginOfLine);
    e->setEditAction(            ShiftMask, XK_End,       this, &StandardEditActions::selectionCursorEndOfLine);

    e->setEditAction(          ControlMask, XK_z,         this, &StandardEditActions::undo);
    e->setEditAction(ShiftMask|ControlMask, XK_z,         this, &StandardEditActions::redo);

    e->setEditAction(          ControlMask, XK_space,     this, &StandardEditActions::selectWordForward);
    e->setEditAction(ShiftMask|ControlMask, XK_space,     this, &StandardEditActions::selectWordBackward);
    e->setEditAction(          ControlMask, XK_m,         this, &StandardEditActions::gotoMatchingBracket);
    
    e->setEditAction(                    0, XK_Tab,       this, &StandardEditActions::tabForward);
}


void StandardEditActions::registerMultiLineEditActionsToEditWidget()
{
    StandardEditActions::registerSingleLineEditActionsToEditWidget();

    e->setEditAction(                    0, XK_Down,      this, &StandardEditActions::cursorDown);
    e->setEditAction(                    0, XK_Up,        this, &StandardEditActions::cursorUp);
    e->setEditAction(                    0, XK_KP_Down,   this, &StandardEditActions::cursorDown);
    e->setEditAction(                    0, XK_KP_Up,     this, &StandardEditActions::cursorUp);
    
    e->setEditAction(                    0, XK_Page_Down, this, &StandardEditActions::cursorPageDown);
    e->setEditAction(                    0, XK_Page_Up,   this, &StandardEditActions::cursorPageUp);

    e->setEditAction(             Mod1Mask, XK_Down,      this, &StandardEditActions::scrollCursorDown);
    e->setEditAction(             Mod1Mask, XK_Up,        this, &StandardEditActions::scrollCursorUp);
    e->setEditAction(             Mod1Mask, XK_KP_Down,   this, &StandardEditActions::scrollCursorDown);
    e->setEditAction(             Mod1Mask, XK_KP_Up,     this, &StandardEditActions::scrollCursorUp);
    
    e->setEditAction( ControlMask|Mod1Mask, XK_Down,      this, &StandardEditActions::scrollDown);
    e->setEditAction( ControlMask|Mod1Mask, XK_Up,        this, &StandardEditActions::scrollUp);
    e->setEditAction( ControlMask|Mod1Mask, XK_KP_Down,   this, &StandardEditActions::scrollDown);
    e->setEditAction( ControlMask|Mod1Mask, XK_KP_Up,     this, &StandardEditActions::scrollUp);

    e->setEditAction(                    0, XK_Return,    this, &StandardEditActions::newLine);
    e->setEditAction(                    0, XK_KP_Enter,  this, &StandardEditActions::newLine);

    e->setEditAction(            ShiftMask, XK_Down,      this, &StandardEditActions::selectionCursorDown);
    e->setEditAction(            ShiftMask, XK_Up,        this, &StandardEditActions::selectionCursorUp);
    e->setEditAction(            ShiftMask, XK_KP_Down,   this, &StandardEditActions::selectionCursorDown);
    e->setEditAction(            ShiftMask, XK_KP_Up,     this, &StandardEditActions::selectionCursorUp);
    
    e->setEditAction(ControlMask|ShiftMask, XK_Down,      this, &StandardEditActions::selectionLineCursorDown);
    e->setEditAction(ControlMask|ShiftMask, XK_Up,        this, &StandardEditActions::selectionLineCursorUp);
    e->setEditAction(ControlMask|ShiftMask, XK_KP_Down,   this, &StandardEditActions::selectionLineCursorDown);
    e->setEditAction(ControlMask|ShiftMask, XK_KP_Up,     this, &StandardEditActions::selectionLineCursorUp);

    e->setEditAction(            ShiftMask, XK_Page_Down, this, &StandardEditActions::selectionCursorPageDown);
    e->setEditAction(            ShiftMask, XK_Page_Up,   this, &StandardEditActions::selectionCursorPageUp);

    e->setEditAction(          ControlMask, XK_9,         this, &StandardEditActions::shiftBlockLeft);
    e->setEditAction(          ControlMask, XK_0,         this, &StandardEditActions::shiftBlockRight);

    e->setEditAction(             Mod1Mask, XK_m,         this, &StandardEditActions::findNextStructureElement);
    e->setEditAction(   ShiftMask|Mod1Mask, XK_m,         this, &StandardEditActions::findPrevStructureElement);
}

