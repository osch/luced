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

#include <iostream>

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
        e->releaseSelection();
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
        e->releaseSelection();
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


void StandardEditActions::cursorUp()
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
                e->releaseSelection();

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
        e->releaseSelection();

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
        e->releaseSelection();

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
        long oldCursorPos = e->getCursorTextPosition();
        if (oldCursorPos > 0)
        {
            e->moveCursorToTextPosition(oldCursorPos - 1);

            long newCursorPos = e->getCursorTextPosition();
            
            if (!e->hasPrimarySelection()) {
                e->setPrimarySelection(oldCursorPos, newCursorPos);
            } else {
                e->extendSelectionTo(newCursorPos);
            }
        }
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
}


void StandardEditActions::selectionCursorRight()
{
    if (!e->areCursorChangesDisabled())
    {
        long oldCursorPos = e->getCursorTextPosition();

        if (oldCursorPos < e->getTextData()->getLength())
        {
            e->moveCursorToTextPosition(oldCursorPos + 1);

            long newCursorPos = e->getCursorTextPosition();

            if (!e->hasPrimarySelection()) {
                e->setPrimarySelection(oldCursorPos, newCursorPos);
            } else {
                e->extendSelectionTo(newCursorPos);
            }
        }
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
}

void StandardEditActions::selectionCursorDown()
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


void StandardEditActions::selectionCursorUp()
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


void StandardEditActions::selectionLineCursorDown()
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


void StandardEditActions::selectionLineCursorUp()
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


void StandardEditActions::selectionCursorWordLeft()
{
    if (!e->areCursorChangesDisabled())
    {
        long oldCursorPos = e->getCursorTextPosition();

        long newCursorPos = oldCursorPos;
        while (newCursorPos > 0 && !e->isWordCharacter(e->getTextData()->getChar(newCursorPos - 1))) {
            --newCursorPos;
        }
        while (newCursorPos > 0 && e->isWordCharacter(e->getTextData()->getChar(newCursorPos - 1))) {
            --newCursorPos;
        }
        e->moveCursorToTextPosition(newCursorPos);

        if (e->hasPrimarySelection()) {
            e->extendSelectionTo(newCursorPos);
        } else {
            e->setPrimarySelection(oldCursorPos, newCursorPos);
        }
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
}


void StandardEditActions::selectionCursorWordRight()
{
    if (!e->areCursorChangesDisabled())
    {
        long oldCursorPos = e->getCursorTextPosition();

        long pos = oldCursorPos;
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

        long newCursorPos = pos;
        
        e->moveCursorToTextPosition(newCursorPos);

        if (e->hasPrimarySelection()) {
            e->extendSelectionTo(newCursorPos);
        } else {
            e->setPrimarySelection(oldCursorPos, newCursorPos);
        }
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
}


void StandardEditActions::shiftBlockLeft()
{
    if (!e->areCursorChangesDisabled() && e->hasSelection() && !e->isReadOnly())
    {
        TextData* textData                                = e->getTextData();
        EditingHistory::SectionHolder::Ptr historySection = textData->createHistorySection();
        
        TextData::TextMark mark   = e->getNewMarkToBeginOfSelection();
        long               endPos = e->getEndSelectionPos();
        
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
    if (!e->areCursorChangesDisabled() && e->hasSelection() && !e->isReadOnly())
    {
        TextData* textData                                = e->getTextData();
        EditingHistory::SectionHolder::Ptr historySection = textData->createHistorySection();
        
        TextData::TextMark mark   = e->getNewMarkToBeginOfSelection();
        long               endPos = e->getEndSelectionPos();
        
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


void StandardEditActions::cursorPageUp()
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

void StandardEditActions::selectionCursorPageDown()
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


void StandardEditActions::selectionCursorPageUp()
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

void StandardEditActions::cursorBeginOfLine()
{
    if (!e->areCursorChangesDisabled())
    {
        e->releaseSelection();
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
        e->releaseSelection();
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
        long oldCursorPos = e->getCursorTextPosition();

        TextData::TextMark mark = e->createNewMarkFromCursor();
        mark.moveToBeginOfLine();
        e->moveCursorToTextMark(mark);

        long newCursorPos = mark.getPos();

        if (e->hasPrimarySelection()) {
            e->extendSelectionTo(newCursorPos);
        } else {
            e->setPrimarySelection(oldCursorPos, newCursorPos);
        }
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
}


void StandardEditActions::selectionCursorEndOfLine()
{
    if (!e->areCursorChangesDisabled())
    {
        long oldCursorPos = e->getCursorTextPosition();

        TextData::TextMark mark = e->createNewMarkFromCursor();
        mark.moveToEndOfLine();
        e->moveCursorToTextMark(mark);

        long newCursorPos = mark.getPos();

        if (e->hasPrimarySelection()) {
            e->extendSelectionTo(newCursorPos);
        } else {
            e->setPrimarySelection(oldCursorPos, newCursorPos);
        }
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
}


void StandardEditActions::cursorBeginOfText()
{
    if (!e->areCursorChangesDisabled())
    {
        e->releaseSelection();
        e->moveCursorToTextPosition(0);
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
}


void StandardEditActions::cursorEndOfText()
{
    if (!e->areCursorChangesDisabled())
    {
        e->releaseSelection();
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


void StandardEditActions::insertNewLineAutoIndent()
{
    newLineAutoIndent(true);
}
void StandardEditActions::appendNewLineAutoIndent()
{
    newLineAutoIndent(false);
}

void StandardEditActions::newLineAutoIndent(bool insert)
{
    if (!e->areCursorChangesDisabled() && !e->isReadOnly())
    {
        e->setCurrentAction(TextEditorWidget::ACTION_NEWLINE);
        
        EditingHistory::SectionHolder::Ptr historySectionHolder = e->getTextData()->createHistorySection();

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

void StandardEditActions::newLineFixedColumnIndent(bool forward)
{
    if (!e->areCursorChangesDisabled() && !e->isReadOnly())
    {
        e->setCurrentAction(TextEditorWidget::ACTION_NEWLINE);
        
        EditingHistory::SectionHolder::Ptr historySectionHolder = e->getTextData()->createHistorySection();

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

void StandardEditActions::newLineFixedColumnIndentForward()
{
    newLineFixedColumnIndent(true);
}

void StandardEditActions::newLineFixedColumnIndentBackward()
{
    newLineFixedColumnIndent(false);
}


void StandardEditActions::tabForward()
{
    if (!e->areCursorChangesDisabled() && !e->isReadOnly())
    {
        e->getTextData()->setMergableHistorySeparator();
        EditingHistory::SectionHolder::Ptr historySection = e->getTextData()->getHistorySectionHolder();
        
        if (e->hasPrimarySelection()) {
            long selBegin  = e->getBeginSelectionPos();
            long selLength = e->getEndSelectionPos() - selBegin;
            e->moveCursorToTextPosition(selBegin);
            e->removeAtCursor(selLength);
            e->releaseSelection();
        } else if (e->hasPseudoSelection()) {
            if (   (   e->getLastAction() != TextEditorWidget::ACTION_KEYBOARD_INPUT 
                    && e->getLastAction() != TextEditorWidget::ACTION_TABULATOR)
                || e->getCursorTextPosition() != e->getEndSelectionPos())
            {
                e->releaseSelection();
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
        if (e->hasPseudoSelection()) {
            e->extendSelectionTo(e->getCursorTextPosition());
        }
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
    e->showCursor();
}



void StandardEditActions::backSpace()
{
    if (!e->areCursorChangesDisabled() && !e->isReadOnly())
    {

        e->hideCursor();
        if (e->hasPrimarySelection())
        {
            {
                EditingHistory::SectionHolder::Ptr historySectionHolder = e->getTextData()->createHistorySection();

                long selBegin = e->getBeginSelectionPos();
                long selLength = e->getEndSelectionPos() - selBegin;
                e->moveCursorToTextPosition(selBegin);
                e->removeAtCursor(selLength);
                e->releaseSelection();
            }
            e->getTextData()->setHistorySeparator();
        }
        else
        {
            e->getTextData()->setMergableHistorySeparator();
            EditingHistory::SectionHolder::Ptr historySection = e->getTextData()->getHistorySectionHolder();

            const long cursorPos = e->getCursorTextPosition(); 

            if (e->hasPseudoSelection())
            {
                if (  (e->getLastAction() != TextEditorWidget::ACTION_KEYBOARD_INPUT && e->getLastAction() != TextEditorWidget::ACTION_TABULATOR)
                    || e->getCursorTextPosition() != e->getEndSelectionPos())
                {
                    e->releaseSelection();
                }
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
    if (!e->areCursorChangesDisabled() && !e->isReadOnly())
    {
        e->hideCursor();
        if (e->hasPrimarySelection()) 
        {
            {
                EditingHistory::SectionHolder::Ptr historySectionHolder = e->getTextData()->createHistorySection();

                long selBegin  = e->getBeginSelectionPos();
                long selLength = e->getEndSelectionPos() - selBegin;
                e->moveCursorToTextPosition(selBegin);
                e->removeAtCursor(selLength);
                e->releaseSelection();
            }
            e->getTextData()->setHistorySeparator();
        }
        else
        {
            if (e->hasPseudoSelection())
            {
                if (   (e->getLastAction() != TextEditorWidget::ACTION_KEYBOARD_INPUT && e->getLastAction() != TextEditorWidget::ACTION_TABULATOR)
                    || e->getCursorTextPosition() != e->getBeginSelectionPos())
                {
                    e->releaseSelection();
                }
            }
            e->getTextData()->setMergableHistorySeparator();
            EditingHistory::SectionHolder::Ptr historySection = e->getTextData()->getHistorySectionHolder();

            e->setCurrentAction(TextEditorWidget::ACTION_KEYBOARD_INPUT);
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

        if (e->hasPrimarySelection()) {
            long selBegin  = e->getBeginSelectionPos();
            long selLength = e->getEndSelectionPos() - selBegin;
            Clipboard::getInstance()->copyToClipboard(e->getTextData()->getAmount(selBegin, selLength), selLength);
        } else {
            Clipboard::getInstance()->copyActiveSelectionToClipboard();
        }
    }
}

void StandardEditActions::cutToClipboard()
{
    if (!e->areCursorChangesDisabled() && !e->isReadOnly())
    {
        if (e->hasPrimarySelection())
        {
            {
                EditingHistory::SectionHolder::Ptr historySectionHolder = e->getTextData()->createHistorySection();

                long selBegin  = e->getBeginSelectionPos();
                long selLength = e->getEndSelectionPos() - selBegin;
                Clipboard::getInstance()->copyToClipboard(e->getTextData()->getAmount(selBegin, selLength), selLength);
                e->moveCursorToTextPosition(selBegin);
                e->removeAtCursor(selLength);
                e->releaseSelection();
            }
            e->getTextData()->setHistorySeparator();
        }
    }
}

void StandardEditActions::selectAll()
{
    if (!e->areCursorChangesDisabled() && e->getTextData()->getLength() > 0)
    {
        if (e->hasPseudoSelection()) {
            e->makePseudoSelectionToPrimary();
        } else {
            e->setPrimarySelection(0, e->getTextData()->getLength());
        }
    }
}


void StandardEditActions::pasteFromClipboardForward()
{
    if (!e->areCursorChangesDisabled() && !e->isReadOnly())
    {
        EditingHistory::SectionHolder::Ptr historySection = e->getTextData()->createHistorySection();

        TextData::TextMark m;

        if (e->hasPrimarySelection()) {
            m = e->getNewMarkToBeginOfSelection();
            long selLength = e->getEndSelectionPos() - m.getPos();
            e->getTextData()->removeAtMark(m, selLength);
        } else {
            m = e->createNewMarkFromCursor();
        }
        e->requestClipboardPasting(m, TextEditorWidget::CURSOR_TO_END_OF_PASTED_DATA);
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
}

void StandardEditActions::pasteFromClipboardBackward()
{
    if (!e->areCursorChangesDisabled() && !e->isReadOnly())
    {
        EditingHistory::SectionHolder::Ptr historySection = e->getTextData()->createHistorySection();

        TextData::TextMark m;

        if (e->hasPrimarySelection()) {
            m = e->getNewMarkToBeginOfSelection();
            long selLength = e->getEndSelectionPos() - m.getPos();
            e->getTextData()->removeAtMark(m, selLength);
        } else {
            m = e->createNewMarkFromCursor();
        }
        e->requestClipboardPasting(m, TextEditorWidget::CURSOR_TO_BEGIN_OF_PASTED_DATA);
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
}

void StandardEditActions::undo()
{
    if (!e->areCursorChangesDisabled() && !e->isReadOnly())
    {
        e->hideCursor();
        TextData::TextMark mark = e->createNewMarkFromCursor();
        long length = e->getTextData()->undo(mark);
        e->moveCursorToTextMark(mark);
        
        if (length > 0)
        {
            long spos = e->getCursorTextPosition();
            e->moveCursorToTextPosition(spos + length);
            e->setPrimarySelection(spos, spos + length);
        }
        else {
            e->releaseSelection();
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
    if (!e->areCursorChangesDisabled() && !e->isReadOnly())
    {
        TextData::TextMark mark = e->createNewMarkFromCursor();
        long length = e->getTextData()->redo(mark);
        e->moveCursorToTextMark(mark);

        if (length > 0)
        {
            long spos = e->getCursorTextPosition();
            e->moveCursorToTextPosition(spos + length);
            e->setPrimarySelection(spos, spos + length);
        } else {
            e->releaseSelection();
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
        
        long len  = textData->getLength();
        long cursorPos = e->getCursorTextPosition();
        long spos = cursorPos;
        long epos = cursorPos;
        
        if (e->hasPrimarySelection())
        {
            spos = e->getBeginSelectionPos();
            epos = e->getEndSelectionPos();
            
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

            if (e->getBeginSelectionPos() != spos)
            {
                e->moveSelectionBeginTo(spos);
            }
            e->setAnchorToBeginOfSelection();
            e->moveCursorToTextPosition(epos);
        }
        else
        {
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
            e->setPrimarySelection(spos, epos);
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

        long len  = textData->getLength();
        long cursorPos = e->getCursorTextPosition();
        long spos = cursorPos;
        long epos = cursorPos;

        if (e->hasPrimarySelection())
        {
            e->setAnchorToBeginOfSelection();
            spos = e->getBeginSelectionPos();
            epos = e->getEndSelectionPos();
        
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
                e->extendSelectionTo(epos);
                e->moveCursorToTextPosition(epos);
            }
        }
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
}

void StandardEditActions::spaceBackward()
{
    if (!e->areCursorChangesDisabled())
    {
        e->hideCursor();
        if (e->hasPrimarySelection())
        {
            long selBegin  = e->getBeginSelectionPos();
            long selLength = e->getEndSelectionPos() - selBegin;
            e->moveCursorToTextPosition(selBegin);
            e->removeAtCursor(selLength);
            e->releaseSelection();
        }
        else if (e->hasPseudoSelection())
        {
            if (   (e->getLastAction() != TextEditorWidget::ACTION_KEYBOARD_INPUT && e->getLastAction() != TextEditorWidget::ACTION_TABULATOR)
                || e->getCursorTextPosition() != e->getBeginSelectionPos()
                || e->getBeginSelectionPos() == e->getEndSelectionPos())
            {
                e->releaseSelection();
            }
        }
        e->insertAtCursor(" ");
        if (!e->hasSelection())
        {
            e->setPseudoSelection(e->getCursorTextPosition(),
                                  e->getCursorTextPosition() + 1);
        }
        e->setCurrentAction(TextEditorWidget::ACTION_KEYBOARD_INPUT);
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
    e->showCursor();
}




void StandardEditActions::findNextLuaStructureElement()
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



void StandardEditActions::findPrevLuaStructureElement()
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
    e->setEditAction(          ControlMask, XK_v,         this, &StandardEditActions::pasteFromClipboardForward);
    e->setEditAction(ShiftMask|ControlMask, XK_v,         this, &StandardEditActions::pasteFromClipboardBackward);
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
    e->setEditAction(                    0, XK_Insert,    this, &StandardEditActions::spaceBackward);
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

    e->setEditAction(                    0, XK_Return,    this, &StandardEditActions::insertNewLineAutoIndent);
    e->setEditAction(                    0, XK_KP_Enter,  this, &StandardEditActions::insertNewLineAutoIndent);

    e->setEditAction(             Mod1Mask, XK_Return,    this, &StandardEditActions::appendNewLineAutoIndent);
    e->setEditAction(             Mod1Mask, XK_KP_Enter,  this, &StandardEditActions::appendNewLineAutoIndent);

    e->setEditAction(          ControlMask, XK_Return,    this, &StandardEditActions::newLineFixedColumnIndentForward);
    e->setEditAction(          ControlMask, XK_KP_Enter,  this, &StandardEditActions::newLineFixedColumnIndentForward);

    e->setEditAction(ControlMask|ShiftMask, XK_Return,    this, &StandardEditActions::newLineFixedColumnIndentBackward);
    e->setEditAction(ControlMask|ShiftMask, XK_KP_Enter,  this, &StandardEditActions::newLineFixedColumnIndentBackward);

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

    e->setEditAction(             Mod1Mask, XK_m,         this, &StandardEditActions::findNextLuaStructureElement);
    e->setEditAction(   ShiftMask|Mod1Mask, XK_m,         this, &StandardEditActions::findPrevLuaStructureElement);
}

