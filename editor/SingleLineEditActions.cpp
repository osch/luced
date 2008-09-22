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
#include "SingleLineEditActions.hpp"

using namespace LucED;

void SingleLineEditActions::cursorLeft()
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


void SingleLineEditActions::cursorRight()
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


void SingleLineEditActions::gotoMatchingBracket()
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
            
            RawPtr<TextData> textData = e->getTextData();
            
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


void SingleLineEditActions::cursorWordLeft()
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


void SingleLineEditActions::cursorWordRight()
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


void SingleLineEditActions::selectionCursorLeft()
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


void SingleLineEditActions::selectionCursorRight()
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

void SingleLineEditActions::selectionCursorWordLeft()
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


void SingleLineEditActions::selectionCursorWordRight()
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

void SingleLineEditActions::cursorBeginOfLine()
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


void SingleLineEditActions::cursorEndOfLine()
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

void SingleLineEditActions::selectionCursorBeginOfLine()
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


void SingleLineEditActions::selectionCursorEndOfLine()
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


void SingleLineEditActions::tabForward()
{
    if (!e->areCursorChangesDisabled() && !e->isReadOnly())
    {
        e->getTextData()->setMergableHistorySeparator();
        TextData::HistorySection::Ptr historySection = e->getTextData()->getHistorySectionHolder();
        
        if (e->hasPrimarySelection()) {
            long selBegin  = e->getBeginSelectionPos();
            long selLength = e->getEndSelectionPos() - selBegin;
            e->moveCursorToTextPosition(selBegin);
            e->removeAtCursor(selLength);
            e->releaseSelection();
        } else if (e->hasPseudoSelection()) {
            if (   (   e->getLastActionCategory() != TextEditorWidget::ACTION_KEYBOARD_INPUT 
                    && e->getLastActionCategory() != TextEditorWidget::ACTION_TABULATOR)
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
        e->setCurrentActionCategory(TextEditorWidget::ACTION_TABULATOR);
        if (e->hasPseudoSelection()) {
            e->extendSelectionTo(e->getCursorTextPosition());
        }
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
    e->showCursor();
}



void SingleLineEditActions::backSpace()
{
    if (!e->areCursorChangesDisabled() && !e->isReadOnly())
    {

        e->hideCursor();
        if (e->hasPrimarySelection())
        {
            {
                TextData::HistorySection::Ptr historySectionHolder = e->getTextData()->createHistorySection();

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
            TextData::HistorySection::Ptr historySection = e->getTextData()->getHistorySectionHolder();

            const long cursorPos = e->getCursorTextPosition(); 

            if (e->hasPseudoSelection())
            {
                if (  (e->getLastActionCategory() != TextEditorWidget::ACTION_KEYBOARD_INPUT && e->getLastActionCategory() != TextEditorWidget::ACTION_TABULATOR)
                    || e->getCursorTextPosition() != e->getEndSelectionPos())
                {
                    e->releaseSelection();
                }
            }
            e->setCurrentActionCategory(TextEditorWidget::ACTION_KEYBOARD_INPUT);

            
            if (cursorPos > 0)
            {
                long softTabWidth = e->getLanguageMode()->getSoftTabWidth();
                long amountToBeRemoved;
                
                if (softTabWidth <= 0 || (   e->getLastActionCategory() != TextEditorWidget::ACTION_TABULATOR
                                          && e->getLastActionCategory() != TextEditorWidget::ACTION_NEWLINE))
                {
                    amountToBeRemoved = 1;
                }
                else 
                {
#if 0
                    RawPtr<TextData> textData = e->getTextData();
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
                    RawPtr<TextData> textData = e->getTextData();
                    long p                      = cursorPos;
                    long opticalColumn          = e->getOpticalCursorColumn();
                    
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
                    e->setCurrentActionCategory(TextEditorWidget::ACTION_TABULATOR);
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

void SingleLineEditActions::deleteKey()
{
    if (!e->areCursorChangesDisabled() && !e->isReadOnly())
    {
        e->hideCursor();
        if (e->hasPrimarySelection()) 
        {
            {
                TextData::HistorySection::Ptr historySectionHolder = e->getTextData()->createHistorySection();

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
                if (   (e->getLastActionCategory() != TextEditorWidget::ACTION_KEYBOARD_INPUT && e->getLastActionCategory() != TextEditorWidget::ACTION_TABULATOR)
                    || e->getCursorTextPosition() != e->getBeginSelectionPos())
                {
                    e->releaseSelection();
                }
            }
            e->getTextData()->setMergableHistorySeparator();
            TextData::HistorySection::Ptr historySection = e->getTextData()->getHistorySectionHolder();

            e->setCurrentActionCategory(TextEditorWidget::ACTION_KEYBOARD_INPUT);
            e->removeAtCursor(1);
        }
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
    e->showCursor();
}


void SingleLineEditActions::cutToClipboard()
{
    if (!e->areCursorChangesDisabled() && !e->isReadOnly())
    {
        if (e->hasPrimarySelection())
        {
            {
                TextData::HistorySection::Ptr historySectionHolder = e->getTextData()->createHistorySection();

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

void SingleLineEditActions::pasteFromClipboardForward()
{
    if (!e->areCursorChangesDisabled() && !e->isReadOnly())
    {
        {
            TextData::HistorySection::Ptr historySection = e->getTextData()->createHistorySection();
    
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
        //e->getTextData()->setHistorySeparator();
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
}

void SingleLineEditActions::pasteFromClipboardBackward()
{
    if (!e->areCursorChangesDisabled() && !e->isReadOnly())
    {
        TextData::HistorySection::Ptr historySection = e->getTextData()->createHistorySection();

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

void SingleLineEditActions::undo()
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

void SingleLineEditActions::redo()
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


void SingleLineEditActions::selectWordForward()
{
    if (!e->areCursorChangesDisabled())
    {
        RawPtr<TextData>  textData = e->getTextData();
        
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
            e->extendSelectionTo(epos);
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

void SingleLineEditActions::selectWordBackward()
{
    if (!e->areCursorChangesDisabled())
    {
        RawPtr<TextData> textData = e->getTextData();

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

void SingleLineEditActions::spaceBackward()
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
            if (   (e->getLastActionCategory() != TextEditorWidget::ACTION_KEYBOARD_INPUT && e->getLastActionCategory() != TextEditorWidget::ACTION_TABULATOR)
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
        e->setCurrentActionCategory(TextEditorWidget::ACTION_KEYBOARD_INPUT);
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
    e->showCursor();
}


