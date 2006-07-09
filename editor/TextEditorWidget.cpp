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

#include "TextEditorWidget.h"
#include "util.h"
#include "EventDispatcher.h"
#include "GlobalConfig.h"
#include "Clipboard.h"

using namespace LucED;


TextEditorWidget::TextEditorWidget(GuiWidget *parent, 
            TextData::Ptr textData, TextStyles::Ptr textStyles, Hiliting::Ptr hiliting)
      : TextWidget(parent, textData, textStyles, hiliting),
        SelectionOwner(this),
        PasteDataReceiver(this),
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

}


bool TextEditorWidget::isWordCharacter(unsigned char c)
{
    return c == '_' || isalnum(c);
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

void TextEditorWidget::notifyAboutReceivedPasteData(const byte* data, long length)
{
    if (length > 0) {
        length = insertAtCursor(data, length);
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
                        } else if (hasSelectionOwnership()) {
                            releaseSelectionOwnership();
                        }

                        if (wasDoubleClick)
                        {
                            long doubleClickPos = getTextPosFromPixXY(x, y, false);
                            long p1 = doubleClickPos;
                            long p2 = p1;
                            TextData *textData = getTextData();
                            if (doubleClickPos < textData->getLength())
                            {
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
                                    if (p1 != p2) {
                                        requestSelectionOwnership();
                                        getBackliteBuffer()->activateSelection(p1);
                                        getBackliteBuffer()->extendSelectionTo(p2);
                                    }
                                    moveCursorToTextPosition(p2);
                                }
                            }
                        } else {
                            if (extendingSelection) {
                                getBackliteBuffer()->extendSelectionTo(newCursorPos);
                            } else {
                                // not here, activate Selection if mouse cursor move
                                // getBackliteBuffer()->activateSelection(newCursorPos);
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

                    if (wasDoubleClick) {
                        long doubleClickPos = getTextPosFromPixXY(x, y, false);
                        long p1 = doubleClickPos;
                        long p2 = p1;
                        TextData *textData = getTextData();

                        if (doubleClickPos < textData->getLength())
                        {
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
                            if (p1 != getCursorTextPosition() || p2 != getCursorTextPosition())
                            {
                                if (!hasSelectionOwnership()) {
                                    requestSelectionOwnership();
                                    getBackliteBuffer()->activateSelection(getCursorTextPosition());
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
                            }
                        }
                    } else {
                        if (newCursorPos != getCursorTextPosition())
                        {
                            if (!hasSelectionOwnership()) {
                                requestSelectionOwnership();
                                getBackliteBuffer()->activateSelection(getCursorTextPosition());
                            }
                            getBackliteBuffer()->extendSelectionTo(newCursorPos);
                        }
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
        if (newCursorPos != getCursorTextPosition())
        {
            if (!hasSelectionOwnership()) {
                requestSelectionOwnership();
                getBackliteBuffer()->activateSelection(getCursorTextPosition());
            }
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
}


bool TextEditorWidget::processKeyboardEvent(const XEvent *event)
{
    unsigned int buttonState = event->xkey.state & (ControlMask|Mod1Mask|ShiftMask);
    KeyMapping<TextEditorWidget>::FunctionPtr m = 
            keyMapping.find(buttonState, XLookupKeysym((XKeyEvent*)&event->xkey, 0));
    if (m != NULL) {
        m(this);
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
                long insertedLength = insertAtCursor(buffer[0]);
                moveCursorToTextPosition(getCursorTextPosition() + insertedLength);
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

void TextEditorWidget::scrollDown()
{
    if (this->getTopLineNumber() < this->getTextData()->getNumberOfLines() - this->getNumberOfVisibleLines()) {
        this->setTopLineNumber(this->getTopLineNumber() + 1);
    }
}


void TextEditorWidget::scrollUp()
{
    this->setTopLineNumber(this->getTopLineNumber() - 1);
}


void TextEditorWidget::scrollLeft()
{
    long newLeft = this->getLeftPix() - this->getTextStyles()->get(0)->getSpaceWidth();
    this->setLeftPix(newLeft);
}


void TextEditorWidget::scrollRight()
{
    long newLeft = this->getLeftPix() + this->getTextStyles()->get(0)->getSpaceWidth();
    this->setLeftPix(newLeft);
}


void TextEditorWidget::scrollPageUp()
{
    long targetTopLine = this->getTopLineNumber() - (this->getNumberOfVisibleLines() - 1);
    
    if (targetTopLine < 0) {
        targetTopLine = 0;
    }
    this->setTopLineNumber(targetTopLine);
}


void TextEditorWidget::scrollPageDown()
{
    long targetTopLine = this->getTopLineNumber() + this->getNumberOfVisibleLines() - 1;
    
    if (targetTopLine > this->getTextData()->getNumberOfLines() - this->getNumberOfVisibleLines()) {
        targetTopLine = this->getTextData()->getNumberOfLines() - this->getNumberOfVisibleLines();
    }
    this->setTopLineNumber(targetTopLine);
}


void TextEditorWidget::scrollPageLeft()
{
    int columns = this->getPixWidth() / this->getTextStyles()->get(0)->getSpaceWidth();
    long newLeft = this->getLeftPix() - this->getTextStyles()->get(0)->getSpaceWidth() * (columns/2);
    this->setLeftPix(newLeft);
}


void TextEditorWidget::scrollPageRight()
{
    int columns = this->getPixWidth() / this->getTextStyles()->get(0)->getSpaceWidth();
    long newLeft = this->getLeftPix() + this->getTextStyles()->get(0)->getSpaceWidth() * (columns/2);
    this->setLeftPix(newLeft);
}
