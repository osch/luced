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

#include <ctype.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>

#include "TextEditorWidget.h"
#include "util.h"
#include "EventDispatcher.h"
#include "GlobalConfig.h"
#include "Clipboard.h"

using namespace LucED;

static inline bool isWordCharacter(unsigned char c) {
    return c == '_' || isalnum(c);
}


TextEditorWidget::TextEditorWidget(GuiWidget *parent, 
            TextData::Ptr textData, TextStyles::Ptr textStyles, HilitingBuffer::Ptr hilitingBuffer,
            int x, int y, unsigned int width, unsigned int height)
      : TextWidget(parent, textData, textStyles, hilitingBuffer, x, y, width, height),
        SelectionOwner(this),
        PasteDataReceiver(this),
        keyMapping(this),
        rememberedCursorPixX(0),
        slotForScrollStepV(this, &TextEditorWidget::handleScrollStepV),
        slotForScrollStepH(this, &TextEditorWidget::handleScrollStepH),
        slotForScrollRepeating(this, &TextEditorWidget::handleScrollRepeating),
        hasMovingSelection(false),
        cursorChangesDisabled(false),
        buttonPressedCounter(0)
{
    hasFocusFlag = false;
    addToXEventMask(ButtonPressMask|ButtonReleaseMask|ButtonMotionMask);

    keyMapping.add(                    0, XK_Left,      &TextEditorWidget::cursorLeft);
    keyMapping.add(                    0, XK_Right,     &TextEditorWidget::cursorRight);
    keyMapping.add(                    0, XK_KP_Left,   &TextEditorWidget::cursorLeft);
    keyMapping.add(                    0, XK_KP_Right,  &TextEditorWidget::cursorRight);

    keyMapping.add(                    0, XK_Down,      &TextEditorWidget::cursorDown);
    keyMapping.add(                    0, XK_Up,        &TextEditorWidget::cursorUp);
    keyMapping.add(                    0, XK_KP_Down,   &TextEditorWidget::cursorDown);
    keyMapping.add(                    0, XK_KP_Up,     &TextEditorWidget::cursorUp);
    
    keyMapping.add(                    0, XK_Page_Down, &TextEditorWidget::cursorPageDown);
    keyMapping.add(                    0, XK_Page_Up,   &TextEditorWidget::cursorPageUp);
    keyMapping.add(             Mod1Mask, XK_Down,      &TextEditorWidget::cursorPageDown);
    keyMapping.add(             Mod1Mask, XK_Up,        &TextEditorWidget::cursorPageUp);
    
    keyMapping.add(             Mod1Mask, XK_Left,      &TextEditorWidget::cursorBeginOfLine);
    keyMapping.add(             Mod1Mask, XK_Right,     &TextEditorWidget::cursorEndOfLine);
    keyMapping.add(             Mod1Mask, XK_KP_Left,   &TextEditorWidget::cursorBeginOfLine);
    keyMapping.add(             Mod1Mask, XK_KP_Right,  &TextEditorWidget::cursorEndOfLine);

    keyMapping.add(                    0, XK_Home,      &TextEditorWidget::cursorBeginOfLine);
    keyMapping.add(                    0, XK_Begin,     &TextEditorWidget::cursorBeginOfLine);
    keyMapping.add(                    0, XK_End,       &TextEditorWidget::cursorEndOfLine);

    keyMapping.add( ControlMask|Mod1Mask, XK_Down,      &TextEditorWidget::scrollDown);
    keyMapping.add( ControlMask|Mod1Mask, XK_Up,        &TextEditorWidget::scrollUp);
    keyMapping.add( ControlMask|Mod1Mask, XK_KP_Down,   &TextEditorWidget::scrollDown);
    keyMapping.add( ControlMask|Mod1Mask, XK_KP_Up,     &TextEditorWidget::scrollUp);

    keyMapping.add( ControlMask|Mod1Mask, XK_Left,      &TextEditorWidget::scrollLeft);
    keyMapping.add( ControlMask|Mod1Mask, XK_Right,     &TextEditorWidget::scrollRight);
    keyMapping.add( ControlMask|Mod1Mask, XK_KP_Left,   &TextEditorWidget::scrollLeft);
    keyMapping.add( ControlMask|Mod1Mask, XK_KP_Right,  &TextEditorWidget::scrollRight);

    keyMapping.add(          ControlMask, XK_Home,      &TextEditorWidget::cursorBeginOfText);
    keyMapping.add(          ControlMask, XK_Begin,     &TextEditorWidget::cursorBeginOfText);
    keyMapping.add(          ControlMask, XK_End,       &TextEditorWidget::cursorEndOfText);

    keyMapping.add(                    0, XK_Return,    &TextEditorWidget::newLine);
    keyMapping.add(                    0, XK_KP_Enter,  &TextEditorWidget::newLine);

    keyMapping.add(                    0, XK_BackSpace, &TextEditorWidget::backSpace);
    keyMapping.add(                    0, XK_Delete,    &TextEditorWidget::deleteKey);

    keyMapping.add(          ControlMask, XK_c,         &TextEditorWidget::copyToClipboard);
    keyMapping.add(          ControlMask, XK_v,         &TextEditorWidget::pasteFromClipboard);
    keyMapping.add(          ControlMask, XK_a,         &TextEditorWidget::selectAll);

    keyMapping.add(            ShiftMask, XK_Left,      &TextEditorWidget::selectionCursorLeft);
    keyMapping.add(            ShiftMask, XK_Right,     &TextEditorWidget::selectionCursorRight);
    keyMapping.add(            ShiftMask, XK_KP_Left,   &TextEditorWidget::selectionCursorLeft);
    keyMapping.add(            ShiftMask, XK_KP_Right,  &TextEditorWidget::selectionCursorRight);

    keyMapping.add(            ShiftMask, XK_Down,      &TextEditorWidget::selectionCursorDown);
    keyMapping.add(            ShiftMask, XK_Up,        &TextEditorWidget::selectionCursorUp);
    keyMapping.add(            ShiftMask, XK_KP_Down,   &TextEditorWidget::selectionCursorDown);
    keyMapping.add(            ShiftMask, XK_KP_Up,     &TextEditorWidget::selectionCursorUp);
    
    keyMapping.add(          ControlMask, XK_Left,      &TextEditorWidget::cursorWordLeft);
    keyMapping.add(          ControlMask, XK_Right,     &TextEditorWidget::cursorWordRight);
    keyMapping.add(ShiftMask|ControlMask, XK_Left,      &TextEditorWidget::selectionCursorWordLeft);
    keyMapping.add(ShiftMask|ControlMask, XK_Right,     &TextEditorWidget::selectionCursorWordRight);

    keyMapping.add(            ShiftMask, XK_Home,      &TextEditorWidget::selectionCursorBeginOfLine);
    keyMapping.add(            ShiftMask, XK_Begin,     &TextEditorWidget::selectionCursorBeginOfLine);
    keyMapping.add(            ShiftMask, XK_End,       &TextEditorWidget::selectionCursorEndOfLine);

    keyMapping.add(            ShiftMask, XK_Page_Down, &TextEditorWidget::selectionCursorPageDown);
    keyMapping.add(            ShiftMask, XK_Page_Up,   &TextEditorWidget::selectionCursorPageUp);
}


void TextEditorWidget::assureCursorVisible()
{
    if (getCursorLineNumber() < getTopLineNumber()) {
        setTopLineNumber(getCursorLineNumber());
    } else if (getCursorLineNumber() >= getTopLineNumber() + getNumberOfVisibleLines()) {
        setTopLineNumber(getCursorLineNumber() - getNumberOfVisibleLines() + 1);
    }
    long pixX = getCursorPixX();
    int spaceWidth = getTextStyles()->get(0)->getSpaceWidth();
    
    if (pixX < getLeftPix() + spaceWidth) {
        setLeftPix(pixX - spaceWidth);
    } else if (pixX > getRightPix() - spaceWidth) {
        long newLeftPix = spaceWidth 
                * util::roundedUpDiv(pixX - (getRightPix() - getLeftPix()) + spaceWidth, spaceWidth);
        setLeftPix(newLeftPix);
    }
}

void TextEditorWidget::cursorLeft()
{
    if (!cursorChangesDisabled)
    {
        releaseSelectionOwnership();
        long cursorPos = getCursorTextPosition();
        if (cursorPos > 0) {
            moveCursorToTextPosition(cursorPos - 1);
        }
    }
    assureCursorVisible();
    rememberedCursorPixX = getCursorPixX();
}


void TextEditorWidget::cursorRight()
{
    if (!cursorChangesDisabled)
    {
        releaseSelectionOwnership();
        long cursorPos = getCursorTextPosition();
        if (cursorPos < getTextData()->getLength()) {
            moveCursorToTextPosition(cursorPos + 1);
        }
    }
    assureCursorVisible();
    rememberedCursorPixX = getCursorPixX();
}

void TextEditorWidget::cursorDown()
{
    if (!cursorChangesDisabled)
    {
        releaseSelectionOwnership();
        long pos = getCursorTextPosition();
        pos = getTextData()->getNextLineBegin(pos);
        if (getTextData()->isBeginOfLine(pos)) {
            pos = getTextPosForPixX(rememberedCursorPixX, pos);
            moveCursorToTextPosition(pos);
        } else {
            // Cursor is in last line
        }
    }
    assureCursorVisible();
}


void TextEditorWidget::cursorUp()
{
    if (!cursorChangesDisabled)
    {
        releaseSelectionOwnership();
        long pos = getCursorTextPosition();
        pos = getTextData()->getPrevLineBegin(pos);
        pos = getTextPosForPixX(rememberedCursorPixX, pos);
        moveCursorToTextPosition(pos);
    }
    assureCursorVisible();
}

void TextEditorWidget::cursorWordLeft()
{
    if (!cursorChangesDisabled)
    {
        releaseSelectionOwnership();

        long pos = getCursorTextPosition();
        while (pos > 0 && !isWordCharacter(getTextData()->getChar(pos - 1))) {
            --pos;
        }
        while (pos > 0 && isWordCharacter(getTextData()->getChar(pos - 1))) {
            --pos;
        }
        moveCursorToTextPosition(pos);
    }
    assureCursorVisible();
    rememberedCursorPixX = getCursorPixX();
}


void TextEditorWidget::cursorWordRight()
{
    if (!cursorChangesDisabled)
    {
        releaseSelectionOwnership();

        long pos = getCursorTextPosition();
        long len = getTextData()->getLength();
        while (pos < len && isWordCharacter(getTextData()->getChar(pos))) {
            ++pos;
        }
        while (pos < len && !isWordCharacter(getTextData()->getChar(pos))) {
            ++pos;
        }
        moveCursorToTextPosition(pos);
    }
    assureCursorVisible();
    rememberedCursorPixX = getCursorPixX();
}



void TextEditorWidget::selectionCursorLeft()
{
    if (!cursorChangesDisabled)
    {
        long cursorPos = getCursorTextPosition();
        if (!hasSelectionOwnership()) {
            requestSelectionOwnership();
            getBackliteBuffer()->activateSelection(cursorPos);
        }
        if (cursorPos > 0) {
            moveCursorToTextPosition(cursorPos - 1);
        }
        getBackliteBuffer()->extendSelectionTo(getCursorTextPosition());
    }
    assureCursorVisible();
    rememberedCursorPixX = getCursorPixX();
}


void TextEditorWidget::selectionCursorRight()
{
    if (!cursorChangesDisabled)
    {
        long cursorPos = getCursorTextPosition();
        if (!hasSelectionOwnership()) {
            requestSelectionOwnership();
            getBackliteBuffer()->activateSelection(cursorPos);
        }
        if (cursorPos < getTextData()->getLength()) {
            moveCursorToTextPosition(cursorPos + 1);
        }
        getBackliteBuffer()->extendSelectionTo(getCursorTextPosition());
    }
    assureCursorVisible();
    rememberedCursorPixX = getCursorPixX();
}

void TextEditorWidget::selectionCursorDown()
{
    if (!cursorChangesDisabled)
    {
        long pos = getCursorTextPosition();
        if (!hasSelectionOwnership()) {
            requestSelectionOwnership();
            getBackliteBuffer()->activateSelection(pos);
        }
        pos = getTextData()->getNextLineBegin(pos);
        if (getTextData()->isBeginOfLine(pos)) {
            pos = getTextPosForPixX(rememberedCursorPixX, pos);
            moveCursorToTextPosition(pos);
        } else {
            // Cursor is in last line
        }
        getBackliteBuffer()->extendSelectionTo(getCursorTextPosition());
    }
    assureCursorVisible();
}


void TextEditorWidget::selectionCursorUp()
{
    if (!cursorChangesDisabled)
    {
        long pos = getCursorTextPosition();
        if (!hasSelectionOwnership()) {
            requestSelectionOwnership();
            getBackliteBuffer()->activateSelection(pos);
        }
        pos = getTextData()->getPrevLineBegin(pos);
        pos = getTextPosForPixX(rememberedCursorPixX, pos);
        moveCursorToTextPosition(pos);
        getBackliteBuffer()->extendSelectionTo(getCursorTextPosition());
    }
    assureCursorVisible();
}


void TextEditorWidget::selectionCursorWordLeft()
{
    if (!cursorChangesDisabled)
    {
        long cursorPos = getCursorTextPosition();
        if (!hasSelectionOwnership()) {
            requestSelectionOwnership();
            getBackliteBuffer()->activateSelection(cursorPos);
        }
        long pos = cursorPos;
        while (pos > 0 && !isWordCharacter(getTextData()->getChar(pos - 1))) {
            --pos;
        }
        while (pos > 0 && isWordCharacter(getTextData()->getChar(pos - 1))) {
            --pos;
        }
        moveCursorToTextPosition(pos);
        getBackliteBuffer()->extendSelectionTo(getCursorTextPosition());
    }
    assureCursorVisible();
    rememberedCursorPixX = getCursorPixX();
}


void TextEditorWidget::selectionCursorWordRight()
{
    if (!cursorChangesDisabled)
    {
        long cursorPos = getCursorTextPosition();
        if (!hasSelectionOwnership()) {
            requestSelectionOwnership();
            getBackliteBuffer()->activateSelection(cursorPos);
        }
        long pos = cursorPos;
        long len = getTextData()->getLength();
        while (pos < len && isWordCharacter(getTextData()->getChar(pos))) {
            ++pos;
        }
        while (pos < len && !isWordCharacter(getTextData()->getChar(pos))) {
            ++pos;
        }
        moveCursorToTextPosition(pos);
        getBackliteBuffer()->extendSelectionTo(getCursorTextPosition());
    }
    assureCursorVisible();
    rememberedCursorPixX = getCursorPixX();
}


void TextEditorWidget::cursorPageDown()
{
    if (!cursorChangesDisabled)
    {
        releaseSelectionOwnership();
        long targetLine = getCursorLineNumber() + getNumberOfVisibleLines() - 1;
        long targetTopLine = getTopLineNumber() + getNumberOfVisibleLines() - 1;

        if (targetLine > getTextData()->getNumberOfLines()) {
            targetLine = getCursorLineNumber();
        }
        if (targetTopLine > getTextData()->getNumberOfLines() - getNumberOfVisibleLines()) {
            targetTopLine = getTextData()->getNumberOfLines() - getNumberOfVisibleLines();
        }
        TextData::TextMark mark = createNewMarkFromCursor();
        mark.moveToLineAndColumn(targetLine, 0);

        long newPos = getTextPosForPixX(rememberedCursorPixX, mark.getPos());
        mark.moveToPos(newPos);

        setTopLineNumber(targetTopLine);
        moveCursorToTextMark(mark);
    }
    assureCursorVisible();
}


void TextEditorWidget::cursorPageUp()
{
    if (!cursorChangesDisabled)
    {
        releaseSelectionOwnership();
        long targetLine = getCursorLineNumber() - (getNumberOfVisibleLines() - 1);
        long targetTopLine = getTopLineNumber() - (getNumberOfVisibleLines() - 1);

        if (targetLine < 0) {
            targetLine = getCursorLineNumber();
        }
        if (targetTopLine < 0) {
            targetTopLine = 0;
        }
        TextData::TextMark mark = createNewMarkFromCursor();
        mark.moveToLineAndColumn(targetLine, 0);

        long newPos = getTextPosForPixX(rememberedCursorPixX, mark.getPos());
        mark.moveToPos(newPos);

        setTopLineNumber(targetTopLine);
        moveCursorToTextMark(mark);
    }
    assureCursorVisible();
}

void TextEditorWidget::selectionCursorPageDown()
{
    if (!cursorChangesDisabled)
    {
        long cursorPos = getCursorTextPosition();
        if (!hasSelectionOwnership()) {
            requestSelectionOwnership();
            getBackliteBuffer()->activateSelection(cursorPos);
        }

        long targetLine = getCursorLineNumber() + getNumberOfVisibleLines() - 1;
        long targetTopLine = getTopLineNumber() + getNumberOfVisibleLines() - 1;

        if (targetLine > getTextData()->getNumberOfLines()) {
            targetLine = getCursorLineNumber();
        }
        if (targetTopLine > getTextData()->getNumberOfLines() - getNumberOfVisibleLines()) {
            targetTopLine = getTextData()->getNumberOfLines() - getNumberOfVisibleLines();
        }
        TextData::TextMark mark = createNewMarkFromCursor();
        mark.moveToLineAndColumn(targetLine, 0);

        long newPos = getTextPosForPixX(rememberedCursorPixX, mark.getPos());
        mark.moveToPos(newPos);

        setTopLineNumber(targetTopLine);
        moveCursorToTextMark(mark);
        getBackliteBuffer()->extendSelectionTo(getCursorTextPosition());
    }
    assureCursorVisible();
}


void TextEditorWidget::selectionCursorPageUp()
{
    if (!cursorChangesDisabled)
    {
        long cursorPos = getCursorTextPosition();
        if (!hasSelectionOwnership()) {
            requestSelectionOwnership();
            getBackliteBuffer()->activateSelection(cursorPos);
        }

        long targetLine = getCursorLineNumber() - (getNumberOfVisibleLines() - 1);
        long targetTopLine = getTopLineNumber() - (getNumberOfVisibleLines() - 1);

        if (targetLine < 0) {
            targetLine = getCursorLineNumber();
        }
        if (targetTopLine < 0) {
            targetTopLine = 0;
        }
        TextData::TextMark mark = createNewMarkFromCursor();
        mark.moveToLineAndColumn(targetLine, 0);

        long newPos = getTextPosForPixX(rememberedCursorPixX, mark.getPos());
        mark.moveToPos(newPos);

        setTopLineNumber(targetTopLine);
        moveCursorToTextMark(mark);
        getBackliteBuffer()->extendSelectionTo(getCursorTextPosition());
    }
    assureCursorVisible();
}

void TextEditorWidget::cursorBeginOfLine()
{
    if (!cursorChangesDisabled)
    {
        releaseSelectionOwnership();
        TextData::TextMark mark = createNewMarkFromCursor();
        mark.moveToBeginOfLine();
        moveCursorToTextMark(mark);
    }
    assureCursorVisible();
    rememberedCursorPixX = getCursorPixX();
}


void TextEditorWidget::cursorEndOfLine()
{
    if (!cursorChangesDisabled)
    {
        releaseSelectionOwnership();
        TextData::TextMark mark = createNewMarkFromCursor();
        mark.moveToEndOfLine();
        moveCursorToTextMark(mark);
    }
    assureCursorVisible();
    rememberedCursorPixX = getCursorPixX();
}

void TextEditorWidget::selectionCursorBeginOfLine()
{
    if (!cursorChangesDisabled)
    {
        long cursorPos = getCursorTextPosition();
        if (!hasSelectionOwnership()) {
            requestSelectionOwnership();
            getBackliteBuffer()->activateSelection(cursorPos);
        }

        TextData::TextMark mark = createNewMarkFromCursor();
        mark.moveToBeginOfLine();
        moveCursorToTextMark(mark);
        getBackliteBuffer()->extendSelectionTo(getCursorTextPosition());
    }
    assureCursorVisible();
    rememberedCursorPixX = getCursorPixX();
}


void TextEditorWidget::selectionCursorEndOfLine()
{
    if (!cursorChangesDisabled)
    {
        long cursorPos = getCursorTextPosition();
        if (!hasSelectionOwnership()) {
            requestSelectionOwnership();
            getBackliteBuffer()->activateSelection(cursorPos);
        }

        TextData::TextMark mark = createNewMarkFromCursor();
        mark.moveToEndOfLine();
        moveCursorToTextMark(mark);
        getBackliteBuffer()->extendSelectionTo(getCursorTextPosition());
    }
    assureCursorVisible();
    rememberedCursorPixX = getCursorPixX();
}


void TextEditorWidget::cursorBeginOfText()
{
    if (!cursorChangesDisabled)
    {
        releaseSelectionOwnership();
        moveCursorToTextPosition(0);
    }
    assureCursorVisible();
    rememberedCursorPixX = getCursorPixX();
}


void TextEditorWidget::cursorEndOfText()
{
    if (!cursorChangesDisabled)
    {
        releaseSelectionOwnership();
        moveCursorToTextPosition(getTextData()->getLength());
    }
    assureCursorVisible();
    rememberedCursorPixX = getCursorPixX();
}


void TextEditorWidget::scrollDown()
{
    if (getTopLineNumber() < getTextData()->getNumberOfLines() - getNumberOfVisibleLines()) {
        setTopLineNumber(getTopLineNumber() + 1);
    }
}


void TextEditorWidget::scrollUp()
{
    setTopLineNumber(getTopLineNumber() - 1);
}


void TextEditorWidget::scrollLeft()
{
    long newLeft = getLeftPix() - getTextStyles()->get(0)->getSpaceWidth();
    setLeftPix(newLeft);
}


void TextEditorWidget::scrollRight()
{
    long newLeft = getLeftPix() + getTextStyles()->get(0)->getSpaceWidth();
    setLeftPix(newLeft);
}


void TextEditorWidget::scrollPageUp()
{
    long targetTopLine = getTopLineNumber() - (getNumberOfVisibleLines() - 1);
    
    if (targetTopLine < 0) {
        targetTopLine = 0;
    }
    setTopLineNumber(targetTopLine);
}


void TextEditorWidget::scrollPageDown()
{
    long targetTopLine = getTopLineNumber() + getNumberOfVisibleLines() - 1;
    
    if (targetTopLine > getTextData()->getNumberOfLines() - getNumberOfVisibleLines()) {
        targetTopLine = getTextData()->getNumberOfLines() - getNumberOfVisibleLines();
    }
    setTopLineNumber(targetTopLine);
}


void TextEditorWidget::scrollPageLeft()
{
    int columns = getPixWidth() / getTextStyles()->get(0)->getSpaceWidth();
    long newLeft = getLeftPix() - getTextStyles()->get(0)->getSpaceWidth() * (columns/2);
    setLeftPix(newLeft);
}


void TextEditorWidget::scrollPageRight()
{
    int columns = getPixWidth() / getTextStyles()->get(0)->getSpaceWidth();
    long newLeft = getLeftPix() + getTextStyles()->get(0)->getSpaceWidth() * (columns/2);
    setLeftPix(newLeft);
}


void TextEditorWidget::newLine()
{
    if (!cursorChangesDisabled)
    {
        TextData::TextMark mark = createNewMarkFromCursor();
        ByteArray whiteSpace;
        whiteSpace.append('\n');

        mark.moveToBeginOfLine();
        while (!mark.isEndOfText() && mark.getPos() < getCursorTextPosition()) {
            byte c = mark.getChar();
            if (c == ' ' || c == '\t') {
                whiteSpace.append(c);
            } else {
                break;
            }
            mark.inc();
        }

        hideCursor();
        insertAtCursor(whiteSpace);
        moveCursorToTextPosition(getCursorTextPosition() + whiteSpace.getLength());
    }
    assureCursorVisible();
    rememberedCursorPixX = getCursorPixX();
    showCursor();
}


void TextEditorWidget::backSpace()
{
    if (!cursorChangesDisabled)
    {
        hideCursor();
        if (hasSelectionOwnership()) {
            long selBegin = getBackliteBuffer()->getBeginSelectionPos();
            long selLength = getBackliteBuffer()->getEndSelectionPos() - selBegin;
            moveCursorToTextPosition(selBegin);
            removeAtCursor(selLength);
            releaseSelectionOwnership();
        } else {
            long pos = getCursorTextPosition();
            if (pos > 0) {
                moveCursorToTextPosition(pos - 1);
                removeAtCursor(1);
            }
        }
    }
    assureCursorVisible();
    rememberedCursorPixX = getCursorPixX();
    showCursor();
}

void TextEditorWidget::deleteKey()
{
    if (!cursorChangesDisabled)
    {
        hideCursor();
        if (hasSelectionOwnership()) {
            long selBegin = getBackliteBuffer()->getBeginSelectionPos();
            long selLength = getBackliteBuffer()->getEndSelectionPos() - selBegin;
            moveCursorToTextPosition(selBegin);
            removeAtCursor(selLength);
            releaseSelectionOwnership();
        } else {
            removeAtCursor(1);
        }
    }
    assureCursorVisible();
    rememberedCursorPixX = getCursorPixX();
    showCursor();
}


void TextEditorWidget::copyToClipboard()
{
    if (!cursorChangesDisabled)
    {
        if (getBackliteBuffer()->hasActiveSelection()) {
            long selBegin = getBackliteBuffer()->getBeginSelectionPos();
            long selLength = getBackliteBuffer()->getEndSelectionPos() - selBegin;
            Clipboard::getInstance()->copyToClipboard(getTextData()->getAmount(selBegin, selLength), selLength);
        }
    }
}

void TextEditorWidget::selectAll()
{
    if (!cursorChangesDisabled && getTextData()->getLength() > 0)
    {
        requestSelectionOwnership();
        if (hasSelectionOwnership()) {
            getBackliteBuffer()->activateSelection(0);
            getBackliteBuffer()->extendSelectionTo(getTextData()->getLength());
        }
    }
}



void TextEditorWidget::pasteFromClipboard()
{
    if (!cursorChangesDisabled)
    {
        if (hasSelectionOwnership()) {
            long selBegin = getBackliteBuffer()->getBeginSelectionPos();
            long selLength = getBackliteBuffer()->getEndSelectionPos() - selBegin;
            moveCursorToTextPosition(selBegin);
            removeAtCursor(selLength);
        }
        requestClipboardPasting();
        if (hasSelectionOwnership()) {
            releaseSelectionOwnership();
        }
    }
    assureCursorVisible();
    rememberedCursorPixX = getCursorPixX();
}

void TextEditorWidget::notifyAboutReceivedPasteData(const byte* data, long length)
{
    if (length > 0) {
        insertAtCursor(data, length);
        moveCursorToTextPosition(getCursorTextPosition() + length);
    }
}

void TextEditorWidget::notifyAboutLostSelectionOwnership()
{
    getBackliteBuffer()->deactivateSelection();
}

void TextEditorWidget::notifyAboutBeginOfPastingData()
{
    disableCursorChanges();
}

void TextEditorWidget::notifyAboutEndOfPastingData()
{
    if (cursorChangesDisabled) {
        enableCursorChanges();
    }    
    assureCursorVisible();
    rememberedCursorPixX = getCursorPixX();
}


static inline long calculateScrollTime(int diffPix, int lineHeight)
{
    long microSecs = GlobalConfig::getInstance()->getScrollBarRepeatNextMicroSecs();
    long rslt = microSecs - (microSecs * diffPix)/(10*lineHeight);
    if (rslt < microSecs/10) 
        rslt = microSecs/10;
    return rslt;
}

bool TextEditorWidget::processEvent(const XEvent *event)
{
    if (processSelectionOwnerEvent(event) || processPasteDataReceiverEvent(event) 
            || TextWidget::processEvent(event)) {
        return true;
    } else {
        switch (event->type)
        {
            case ButtonPress:
            {
                if (event->xbutton.button == Button1)
                {
                    if (!cursorChangesDisabled)
                    {
                        if (buttonPressedCounter == 1 && event->xbutton.time - lastButtonPressedTime
                                < GlobalConfig::getInstance()->getDoubleClickMilliSecs()) {
                            wasDoubleClick = true;
                        } else {
                            buttonPressedCounter = 0;
                            wasDoubleClick = false;
                        }
                        ++buttonPressedCounter;
                        
                        int x = event->xbutton.x;
                        int y = event->xbutton.y;

                        long newCursorPos = getTextPosFromPixXY(x, y);

                        bool extendingSelection = (event->xbutton.state & ShiftMask != 0);
                        if (extendingSelection && !hasSelectionOwnership()) {
                            requestSelectionOwnership();
                            getBackliteBuffer()->activateSelection(getCursorTextPosition());
                        }

                        if (extendingSelection) {
                            bool toStart = 
                                abs(newCursorPos - getBackliteBuffer()->getBeginSelectionPos())
                              < abs(newCursorPos - getBackliteBuffer()->getEndSelectionPos());
                            if (toStart) {
                                getBackliteBuffer()->setAnchorToEndOfSelection();
                            } else {
                                getBackliteBuffer()->setAnchorToBeginOfSelection();
                            }
                        } else {
                            releaseSelectionOwnership();
                        }

                        if (wasDoubleClick) {
                            long doubleClickPos = getTextPosFromPixXY(x, y, false);
                            long p1 = doubleClickPos;
                            long p2 = p1;
                            TextData *textData = getTextData();

                            if (isWordCharacter(textData->getChar(p1))) {
                                while (p1 > 0 && isWordCharacter(textData->getChar(p1 - 1))) {
                                    --p1;
                                }
                                while (p2 < textData->getLength() && isWordCharacter(textData->getChar(p2))) {
                                    ++p2;
                                }
                            } else if (ispunct(textData->getChar(p1))) {
                                while (p1 > 0 && ispunct(textData->getChar(p1 - 1))) {
                                    --p1;
                                }
                                while (p2 < textData->getLength() && ispunct(textData->getChar(p2))) {
                                    ++p2;
                                }
                            }
                            if (extendingSelection) {
                                if (getBackliteBuffer()->isAnchorAtBegin()) {
                                    getBackliteBuffer()->extendSelectionTo(p2);
                                    moveCursorToTextPosition(p2);
                                } else {
                                    getBackliteBuffer()->extendSelectionTo(p1);
                                    moveCursorToTextPosition(p1);
                                }
                            } else {
                                requestSelectionOwnership();
                                getBackliteBuffer()->activateSelection(p1);
                                getBackliteBuffer()->extendSelectionTo(p2);
                                moveCursorToTextPosition(p2);
                            }
                        } else {
                            if (extendingSelection) {
                                getBackliteBuffer()->extendSelectionTo(newCursorPos);
                            } else {
                                getBackliteBuffer()->activateSelection(newCursorPos);
                            }
                            moveCursorToTextPosition(newCursorPos);
                        }
                        assureCursorVisible();
                        rememberedCursorPixX = getCursorPixX();

                        this->movingSelectionY = y;
                        this->movingSelectionX = x;
                        this->hasMovingSelection = true;
                        this->isMovingSelectionScrolling = false;
                        lastButtonPressedTime = event->xbutton.time;
                    }
                    return true;
                }
                else if (event->xbutton.button == Button2)
                {
                    if (!cursorChangesDisabled)
                    {
                        int x = event->xbutton.x;
                        int y = event->xbutton.y;

                        long newCursorPos = getTextPosFromPixXY(x, y);
                        moveCursorToTextPosition(newCursorPos);
                        requestSelectionPasting();
                        if (hasSelectionOwnership()) {
                            releaseSelectionOwnership();
                        }
                    }
                    assureCursorVisible();
                    rememberedCursorPixX = getCursorPixX();
                }
                else if (event->xbutton.button == Button4)
                {
                    setTopLineNumber(getTopLineNumber() - 5);
                    return true;
                }
                else if (event->xbutton.button == Button5)
                {
                    if (getTopLineNumber() + 5 >= getTextData()->getNumberOfLines() - getNumberOfVisibleLines()) {
                        setTopLineNumber(getTextData()->getNumberOfLines() - getNumberOfVisibleLines());
                    } else {
                        setTopLineNumber(getTopLineNumber() + 5);
                    }
                    return true;
                }
                break;
            }
            case ButtonRelease:
            {
                if (hasMovingSelection) {
                    hasMovingSelection = false;
                    isMovingSelectionScrolling = false;
                    return true;
                }
                break;
            }
            case MotionNotify:
            {
                if (hasMovingSelection) {
                    XEvent newEvent;
                    XFlush(getDisplay());
                    if (XCheckWindowEvent(getDisplay(), getWid(), ButtonMotionMask, &newEvent) == True) {
                        event = &newEvent;
                        while (XCheckWindowEvent(getDisplay(), getWid(), ButtonMotionMask, &newEvent) == True);
                    }

                    int x = event->xmotion.x;
                    int y = event->xmotion.y;
                    this->movingSelectionY = y;
                    this->movingSelectionX = x;
                    long newCursorPos = getTextPosFromPixXY(x, y);

                    if (!hasSelectionOwnership() && newCursorPos != getCursorTextPosition()) {
                        requestSelectionOwnership();
                    }

                    if (wasDoubleClick) {
                        long doubleClickPos = getTextPosFromPixXY(x, y, false);
                        long p1 = doubleClickPos;
                        long p2 = p1;
                        TextData *textData = getTextData();
                        
                        if (isWordCharacter(textData->getChar(p1))) {
                            while (p1 > 0 && isWordCharacter(textData->getChar(p1 - 1))) {
                                --p1;
                            }
                            while (p2 < textData->getLength() && isWordCharacter(textData->getChar(p2))) {
                                ++p2;
                            }
                        } else if (ispunct(textData->getChar(p1))) {
                            while (p1 > 0 && ispunct(textData->getChar(p1 - 1))) {
                                --p1;
                            }
                            while (p2 < textData->getLength() && ispunct(textData->getChar(p2))) {
                                ++p2;
                            }
                        }
                        if (p1 < getBackliteBuffer()->getSelectionAnchorPos()) {
                            if (getBackliteBuffer()->isAnchorAtBegin()) {
                                long p = getBackliteBuffer()->getSelectionAnchorPos();
                                if (isWordCharacter(textData->getChar(p))) {
                                    while (p < textData->getLength() && isWordCharacter(textData->getChar(p))) {
                                        ++p;
                                    }
                                } else if (ispunct(textData->getChar(p))) {
                                    while (p < textData->getLength() && ispunct(textData->getChar(p))) {
                                        ++p;
                                    }
                                }
                                getBackliteBuffer()->extendSelectionTo(p);
                                getBackliteBuffer()->setAnchorToEndOfSelection();
                            }
                            getBackliteBuffer()->extendSelectionTo(p1);
                            newCursorPos = p1;
                        } else {
                            if (!getBackliteBuffer()->isAnchorAtBegin()) {
                                long p = getBackliteBuffer()->getSelectionAnchorPos();
                                if (p > 0 && isWordCharacter(textData->getChar(p - 1))) {
                                    while (p > 0 && isWordCharacter(textData->getChar(p - 1))) {
                                        --p;
                                    }
                                } else if (p > 0 && ispunct(textData->getChar(p - 1))) {
                                    while (p > 0 && ispunct(textData->getChar(p - 1))) {
                                       --p;
                                    }
                                }
                                getBackliteBuffer()->extendSelectionTo(p);
                                getBackliteBuffer()->setAnchorToBeginOfSelection();
                            }
                            getBackliteBuffer()->extendSelectionTo(p2);
                            newCursorPos = p2;
                        }
                    } else {
                        getBackliteBuffer()->extendSelectionTo(newCursorPos);
                    }
                    moveCursorToTextPosition(newCursorPos);
                    assureCursorVisible();
                    rememberedCursorPixX = getCursorPixX();

                    if (y < 0 ) {
                        if (!isMovingSelectionScrolling) {
                            scrollUp();
                            isMovingSelectionScrolling = true;
                            EventDispatcher::getInstance()->registerTimerCallback(0, 
                                    calculateScrollTime(-y, getLineHeight()),
                                    slotForScrollRepeating);
                        }
                    } else if (y > getHeightPix()) {
                        if (!isMovingSelectionScrolling) {
                            scrollDown();
                            isMovingSelectionScrolling = true;
                            EventDispatcher::getInstance()->registerTimerCallback(0, 
                                    calculateScrollTime(y - getHeightPix(), getLineHeight()),
                                    slotForScrollRepeating);
                        }
                    } else {
                        isMovingSelectionScrolling = false;
                    }  
                    return true;
                }
                break;
            }
        }
        return propagateEventToParentWidget(event);
    }
}

long  TextEditorWidget::initSelectionDataRequest()
{
    long selBegin = getBackliteBuffer()->getBeginSelectionPos();
    long selLength = getBackliteBuffer()->getEndSelectionPos() - selBegin;
    disableCursorChanges();
    return selLength;
}

const byte* TextEditorWidget::getSelectionDataChunk(long pos, long length)
{
    long selBegin = getBackliteBuffer()->getBeginSelectionPos();
    return getTextData()->getAmount(selBegin + pos, length);
}

void  TextEditorWidget::endSelectionDataRequest()
{
    enableCursorChanges();
}


void TextEditorWidget::handleScrollRepeating()
{
    if (isMovingSelectionScrolling) {
        long newCursorPos = getTextPosFromPixXY(movingSelectionX, movingSelectionY);
        moveCursorToTextPosition(newCursorPos);
        assureCursorVisible();
        rememberedCursorPixX = getCursorPixX();
        getBackliteBuffer()->extendSelectionTo(getCursorTextPosition());
        if (movingSelectionY < 0 ) {
            scrollUp();
            EventDispatcher::getInstance()->registerTimerCallback(0, 
                    calculateScrollTime(-movingSelectionY, getLineHeight()),
                    slotForScrollRepeating);
        } else if (movingSelectionY > getHeightPix()) {
            scrollDown();
            EventDispatcher::getInstance()->registerTimerCallback(0, 
                    calculateScrollTime(movingSelectionY - getHeightPix(), getLineHeight()),
                    slotForScrollRepeating);
        }
    }    
}


bool TextEditorWidget::processKeyboardEvent(const XEvent *event)
{
    KeyMapping<TextEditorWidget>::MethodPtr m = 
            keyMapping.find(event->xkey.state, XLookupKeysym((XKeyEvent*)&event->xkey, 0));
    if (m != NULL) {
        (this->*m)();
        return true;
    } else {
        char buffer[100];
        int len = XLookupString(&((XEvent*)event)->xkey, buffer, 100, NULL, NULL);
        if (len > 0) {
            if (!cursorChangesDisabled)
            {
                hideCursor();
                if (hasSelectionOwnership()) {
                    long selBegin = getBackliteBuffer()->getBeginSelectionPos();
                    long selLength = getBackliteBuffer()->getEndSelectionPos() - selBegin;
                    moveCursorToTextPosition(selBegin);
                    removeAtCursor(selLength);
                    releaseSelectionOwnership();
                }
                insertAtCursor(buffer[0]);
                moveCursorToTextPosition(getCursorTextPosition() + 1);
                if (getCursorLineNumber() < getTopLineNumber()) {
                    setTopLineNumber(getCursorLineNumber());
                } else if ((getCursorLineNumber() - getTopLineNumber() + 1) * getLineHeight() > getHeightPix()) {
                    setTopLineNumber(getCursorLineNumber() - getNumberOfVisibleLines() + 1);
                }
                long cursorPixX = getCursorPixX();
                int spaceWidth = getTextStyles()->get(0)->getSpaceWidth();
                if (cursorPixX < getLeftPix()) {
                    setLeftPix(cursorPixX - spaceWidth);
                } else if (cursorPixX >= getRightPix() - spaceWidth) {
                    setLeftPix(cursorPixX - (getRightPix() - getLeftPix()) + 2 * spaceWidth);
                }
                showCursor();
            }
            assureCursorVisible();
            rememberedCursorPixX = getCursorPixX();
            return true;
        } else {
            return false;
        }
    }
}

void TextEditorWidget::disableCursorChanges()
{
    setCursorInactive();
    stopCursorBlinking();
    cursorChangesDisabled = true;
}

void TextEditorWidget::enableCursorChanges()
{
    if (hasFocusFlag) {
        setCursorActive();
        startCursorBlinking();
    }
    cursorChangesDisabled = false;
}

void TextEditorWidget::treatFocusIn()
{
    if (!cursorChangesDisabled) {
        setCursorActive();
        startCursorBlinking();
    }
    hasFocusFlag = true;
}


void TextEditorWidget::treatFocusOut()
{
    setCursorInactive();
    stopCursorBlinking();
    hasFocusFlag = false;
}

void TextEditorWidget::showCursor()
{
    if (hasFocusFlag) {
        TextWidget::startCursorBlinking();
    } else {
        TextWidget::showCursor();
    }
}


void TextEditorWidget::hideCursor()
{
    TextWidget::hideCursor();
}

void TextEditorWidget::handleScrollStepV(ScrollStep::Type scrollStep)
{
    if (scrollStep == ScrollStep::LINE_UP) {
        scrollUp();
    }
    else if (scrollStep == ScrollStep::LINE_DOWN) {
        scrollDown();
    }
    else if (scrollStep == ScrollStep::PAGE_UP) {
        scrollPageUp();
    }
    else if (scrollStep == ScrollStep::PAGE_DOWN) {
        scrollPageDown();
    }
}

void TextEditorWidget::handleScrollStepH(ScrollStep::Type scrollStep)
{
    if (scrollStep == ScrollStep::LINE_UP) {
        scrollLeft();
    }
    else if (scrollStep == ScrollStep::LINE_DOWN) {
        scrollRight();
    }
    else if (scrollStep == ScrollStep::PAGE_UP) {
        scrollPageLeft();
    }
    else if (scrollStep == ScrollStep::PAGE_DOWN) {
        scrollPageRight();
    }
}

