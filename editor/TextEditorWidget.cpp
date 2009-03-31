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

#include <ctype.h>

#include "TextEditorWidget.hpp"
#include "util.hpp"
#include "EventDispatcher.hpp"
#include "GlobalConfig.hpp"
#include "Clipboard.hpp"

using namespace LucED;

const int TextEditorWidget::BORDER_WIDTH;

class TextEditorWidget::SelectionContentHandler : public SelectionOwner::ContentHandler
{
public:
    typedef OwningPtr<ContentHandler> Ptr;

    static Ptr create(RawPtr<TextEditorWidget> textEditorWidget) {
        return Ptr(new SelectionContentHandler(textEditorWidget));
    }
    virtual long initSelectionDataRequest()
    {
        if (textEditorWidget->getBackliteBuffer()->hasActiveSelection())
        {
            long selBegin  = textEditorWidget->getBackliteBuffer()->getBeginSelectionPos();
            long selLength = textEditorWidget->getBackliteBuffer()->getEndSelectionPos() - selBegin;
            textEditorWidget->disableCursorChanges();
            return selLength;
        }
        else {
            return 0;
        }
    }
    virtual const byte* getSelectionDataChunk(long pos, long length)
    {
        long selBegin = textEditorWidget->getBackliteBuffer()->getBeginSelectionPos();
        return textEditorWidget->textData->getAmount(selBegin + pos, length);
    }
    virtual void endSelectionDataRequest()
    {
        textEditorWidget->enableCursorChanges();
    }
    virtual void notifyAboutObtainedSelectionOwnership() {
    }
    virtual void notifyAboutLostSelectionOwnership()
    {
        if (textEditorWidget->getBackliteBuffer()->isSelectionPrimary())
        {
            if (textEditorWidget->isSelectionPersistent) {
                textEditorWidget->getBackliteBuffer()->makeSelectionToSecondarySelection();
            } else {
                textEditorWidget->getBackliteBuffer()->deactivateSelection();
            }
        }
    }
private:
    SelectionContentHandler(RawPtr<TextEditorWidget> textEditorWidget)
        : textEditorWidget(textEditorWidget)
    {}
    RawPtr<TextEditorWidget> textEditorWidget;
};


class TextEditorWidget::PasteDataContentHandler : public PasteDataReceiver::ContentHandler
{
public:
    typedef OwningPtr<ContentHandler> Ptr;

    static Ptr create(RawPtr<TextEditorWidget> textEditorWidget) {
        return Ptr(new PasteDataContentHandler(textEditorWidget));
    }
    
    virtual void notifyAboutBeginOfPastingData() {
        textEditorWidget->notifyAboutBeginOfPastingData();
    }
    virtual void notifyAboutReceivedPasteData(const byte* data, long length) {
        textEditorWidget->notifyAboutReceivedPasteData(data, length);
    }
    virtual void notifyAboutEndOfPastingData() {
        textEditorWidget->notifyAboutEndOfPastingData();
    }
private:
    PasteDataContentHandler(RawPtr<TextEditorWidget> textEditorWidget)
        : textEditorWidget(textEditorWidget)
    {}
    RawPtr<TextEditorWidget> textEditorWidget;
};

class TextEditorWidget::MyKeyActionHandler : public KeyActionHandler
{
public:
    typedef OwningPtr<MyKeyActionHandler> Ptr;
    
    static Ptr create(RawPtr<TextEditorWidget> w) {
        return Ptr(new MyKeyActionHandler(w));
    }
    
    virtual bool invokeActionMethod(ActionId actionId)
    {
        ActionCategory oldLastActionCategory    = w->lastActionCategory;
        ActionCategory oldCurrentActionCategory = w->currentActionCategory;
        
        w->lastActionCategory    = w->currentActionCategory;
        w->currentActionCategory = ACTION_UNSPECIFIED;
    
        bool rslt = KeyActionHandler::invokeActionMethod(actionId);
    
        if (!rslt) {
            w->lastActionCategory    = oldLastActionCategory;
            w->currentActionCategory = oldCurrentActionCategory;
        }
        return rslt;
    }
    
    virtual bool handleLowPriorityKeyPress(const KeyPressEvent& keyPressEvent)
    {
        return w->handleLowPriorityKeyPress(keyPressEvent);
    }
    
private:
    MyKeyActionHandler(RawPtr<TextEditorWidget> w)
        : w(w)
    {}
    
    RawPtr<TextEditorWidget> w;
};

TextEditorWidget::TextEditorWidget(GuiWidget*       parent, 
                                   HilitedText::Ptr hilitedText, 
                                   CreateOptions    options,
                                   int              borderWidth)
                                   
      : TextWidget(parent, hilitedText, borderWidth, options),

        selectionOwner(SelectionOwner::create(this, 
                                              SelectionOwner::TYPE_PRIMARY,
                                              SelectionContentHandler::create(this))),

        rememberedCursorPixX(0),
        scrollRepeatCallback(newCallback(this, &TextEditorWidget::handleScrollRepeating)),
        hasMovingSelection(false),
        cursorChangesDisabled(false),
        buttonPressedCounter(0),
        lastActionCategory(ACTION_UNSPECIFIED),
        currentActionCategory(ACTION_UNSPECIFIED),
        pasteParameter(CURSOR_TO_END_OF_PASTED_DATA),
        isSelectionPersistent(false),
        textData(getTextData()),
        readOnlyFlag(options.isSet(READ_ONLY))
{
    setKeyActionHandler(MyKeyActionHandler::create(this));
    
    if (!options.isSet(READ_ONLY)) {
        pasteDataReceiver = PasteDataReceiver::create(this,
                                                      PasteDataContentHandler::create(this));
    }
    addToXEventMask(ButtonPressMask|ButtonReleaseMask|ButtonMotionMask);
    textData->activateHistory();
}


bool TextEditorWidget::isWordCharacter(unsigned char c)
{
    return c == '_' || isalnum(c);
}

bool TextEditorWidget::isCursorVisible()
{
    if (  (getCursorLineNumber() < getTopLineNumber())
       || (getCursorLineNumber() >= getTopLineNumber() + getNumberOfVisibleLines()))
    {
        return false;
    }
    long pixX = getCursorPixX();
    int spaceWidth = getDefaultTextStyle()->getSpaceWidth();
    
    if (  pixX > 0
       && (   (pixX < getLeftPix() + spaceWidth)
           || (pixX > getRightPix() - spaceWidth)))
    {
        return false;
    }
    return true;
}

void TextEditorWidget::assureCursorVisible()
{
    if (getCursorLineNumber() < getTopLineNumber()) {
        setTopLineNumber(getCursorLineNumber());
    } else if (getCursorLineNumber() >= getTopLineNumber() + getNumberOfVisibleLines()) {
        setTopLineNumber(getCursorLineNumber() - getNumberOfVisibleLines() + 1);
    }
    long pixX = getCursorPixX();
    int spaceWidth = getDefaultTextStyle()->getSpaceWidth();
    
    if (pixX < getLeftPix() + spaceWidth) {
        setLeftPix(pixX - spaceWidth);
    } else if (pixX > getRightPix() - spaceWidth) {
        long newLeftPix = spaceWidth 
                * util::roundedUpDiv(pixX - (getRightPix() - getLeftPix()) + spaceWidth, spaceWidth);
        setLeftPix(newLeftPix);
    }
}

void TextEditorWidget::assureSelectionVisible()
{
    if (getBackliteBuffer()->getEndSelectionLine() <= getTopLineNumber() + 1)
    {
        int newTopLineNumber = getBackliteBuffer()->getEndSelectionLine() - getNumberOfVisibleLines() / 3;
        if (newTopLineNumber < 0) {
            newTopLineNumber = 0;
        }
        setTopLineNumber(newTopLineNumber);
    }
    else if (getBackliteBuffer()->getBeginSelectionLine() >= getTopLineNumber() + getNumberOfVisibleLines()  - 1)
    {
        int newTopLineNumber = getBackliteBuffer()->getBeginSelectionLine() + getNumberOfVisibleLines() / 3 - getNumberOfVisibleLines();
        if (newTopLineNumber > getNumberOfLines() - getNumberOfVisibleLines()) {
            newTopLineNumber = getNumberOfLines() - getNumberOfVisibleLines();
        }
        setTopLineNumber(newTopLineNumber);
    }
}

void TextEditorWidget::adjustCursorVisibility()
{
    if (getCursorLineNumber() < getTopLineNumber()) {
        int newTopLineNumber = getCursorLineNumber() - getNumberOfVisibleLines() / 4;
        if (newTopLineNumber < 0) {
            newTopLineNumber = 0;
        }
        setTopLineNumber(newTopLineNumber);
    } else if (getCursorLineNumber() >= getTopLineNumber() + getNumberOfVisibleLines()) {
        int newTopLineNumber = getCursorLineNumber() - getNumberOfVisibleLines() + getNumberOfVisibleLines() / 4;
        if (newTopLineNumber + getNumberOfVisibleLines() > getNumberOfLines()) {
            newTopLineNumber = getNumberOfLines() - getNumberOfVisibleLines();
        }
        setTopLineNumber(newTopLineNumber);
    }
    long pixX = getCursorPixX();
    int spaceWidth = getDefaultTextStyle()->getSpaceWidth();
    if (pixX < getLeftPix() + spaceWidth) {
        setLeftPix(pixX - spaceWidth);
    } else {
        long rightPix = getRightPix();
        if (rightPix - spaceWidth > 0 && pixX > rightPix - spaceWidth) {
            long newLeftPix = spaceWidth 
                    * util::roundedUpDiv(pixX - (rightPix - getLeftPix()) + spaceWidth, spaceWidth);
            setLeftPix(newLeftPix);
        }
    }
}

void TextEditorWidget::moveCursorToTextMarkAndAdjustVisibility(TextData::MarkHandle m)
{
    moveCursorToTextMark(m);
    adjustCursorVisibility();
}

void TextEditorWidget::moveCursorToTextPositionAndAdjustVisibility(int position)
{
    moveCursorToTextPosition(position);
    adjustCursorVisibility();
}

void TextEditorWidget::notifyAboutBeginOfPastingData()
{
    disableCursorChanges();
    
    pastingDataHistorySectionHolder = textData->createHistorySection();
    
    if (!beginPastingTextMark.isValid()) {
        beginPastingTextMark = createNewMarkFromCursor();
        pastingTextMark      = createNewMarkFromCursor();
    } else {
        pastingTextMark      = textData->createNewMark(beginPastingTextMark);
    }
//    getBackliteBuffer()->activateSelection(pastingTextMark.getPos());
//    getBackliteBuffer()->makeSelectionToSecondarySelection();
}

void TextEditorWidget::notifyAboutReceivedPasteData(const byte* data, long length)
{
    if (length > 0) {
        bool first =  (pastingTextMark.getPos() == beginPastingTextMark.getPos());
        
        textData->insertAtMark(pastingTextMark, data, length);
        pastingTextMark.moveToPos(pastingTextMark.getPos() + length);
        
        //if (!first) {
            if (!getBackliteBuffer()->hasActiveSelection()
              || getBackliteBuffer()->getBeginSelectionPos() != beginPastingTextMark.getPos())
            {
                getBackliteBuffer()->activateSelection(beginPastingTextMark.getPos());
            }
            getBackliteBuffer()->makeSelectionToSecondarySelection();
            getBackliteBuffer()->extendSelectionTo(pastingTextMark.getPos());
        //}
    }
}

void TextEditorWidget::notifyAboutEndOfPastingData()
{
    if (cursorChangesDisabled) {
        enableCursorChanges();
    }

    releaseSelectionButKeepPseudoSelection();

    if (!getBackliteBuffer()->hasActiveSelection()
      || getBackliteBuffer()->getBeginSelectionPos() != beginPastingTextMark.getPos())
    {
        getBackliteBuffer()->activateSelection(beginPastingTextMark.getPos());
    }
    getBackliteBuffer()->makeSelectionToSecondarySelection();
    getBackliteBuffer()->extendSelectionTo(pastingTextMark.getPos());

    if (pasteParameter == CURSOR_TO_END_OF_PASTED_DATA) {
        moveCursorToTextMark(pastingTextMark);
    }
    assureCursorVisible();

    rememberedCursorPixX = getCursorPixX();

    pastingTextMark = TextData::TextMark();

    pastingDataHistorySectionHolder.invalidate();
    textData->setHistorySeparator();
    
    pasteParameter = CURSOR_TO_END_OF_PASTED_DATA;
}


static inline MicroSeconds calculateScrollTime(int diffPix, int lineHeight)
{
    long microSecs = GlobalConfig::getInstance()->getScrollBarRepeatNextMicroSecs();
    long rslt = microSecs - (microSecs * diffPix)/(10*lineHeight);
    if (rslt < microSecs/10) 
        rslt = microSecs/10;
    return MicroSeconds(rslt);
}

GuiWidget::ProcessingResult TextEditorWidget::processEvent(const XEvent *event)
{
    if (   (selectionOwner.isValid()    && selectionOwner   ->processSelectionOwnerEvent   (event) == EVENT_PROCESSED) 
        || (pasteDataReceiver.isValid() && pasteDataReceiver->processPasteDataReceiverEvent(event) == EVENT_PROCESSED)
        || TextWidget::processEvent(event) == EVENT_PROCESSED)
    {
        return EVENT_PROCESSED;
    } else {
        switch (event->type)
        {
            case ButtonPress:
            {
                showMousePointer();
                if (!hasFocus()) {
                    reportMouseClick();
                    requestFocus();
                }
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
                        if (extendingSelection && !selectionOwner->hasSelectionOwnership()) {
                            selectionOwner->requestSelectionOwnership();
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
                            releaseSelection();
                        }

                        if (wasDoubleClick)
                        {
                            long doubleClickPos = getTextPosFromPixXY(x, y, false);
                            long p1 = doubleClickPos;
                            long p2 = p1;
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
                                        selectionOwner->requestSelectionOwnership();
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
                        currentActionCategory = ACTION_UNSPECIFIED;
                    }
                    return EVENT_PROCESSED;
                }
                else if (event->xbutton.button == Button2)
                {
                    if (!cursorChangesDisabled && !isReadOnly())
                    {
                        int x = event->xbutton.x;
                        int y = event->xbutton.y;

                        long newCursorPos = getTextPosFromPixXY(x, y);
                        moveCursorToTextPosition(newCursorPos);
                        pastingDataHistorySectionHolder = textData->createHistorySection();
                        
                        requestSelectionPasting(createNewMarkFromCursor());
                        currentActionCategory = ACTION_UNSPECIFIED;
                    }
                    assureCursorVisible();
                    rememberedCursorPixX = getCursorPixX();
                }
                else if (event->xbutton.button == Button4)
                {
                    setTopLineNumber(getTopLineNumber() - 5);
                    if (hasMovingSelection) {
                        setNewMousePositionForMovingSelection(event->xbutton.x, event->xbutton.y);
                    }
                    return EVENT_PROCESSED;
                }
                else if (event->xbutton.button == Button5)
                {
                    if (getTopLineNumber() + 5 >= getNumberOfLines() - getNumberOfVisibleLines()) {
                        setTopLineNumber(getNumberOfLines() - getNumberOfVisibleLines());
                    } else {
                        setTopLineNumber(getTopLineNumber() + 5);
                    }
                    if (hasMovingSelection) {
                        setNewMousePositionForMovingSelection(event->xbutton.x, event->xbutton.y);
                    }
                    return EVENT_PROCESSED;
                }
                break;
            }
            case ButtonRelease:
            {
                showMousePointer();
                if (event->xbutton.button == Button1) {
                    if (hasMovingSelection) {
                        hasMovingSelection = false;
                        isMovingSelectionScrolling = false;
                        currentActionCategory = ACTION_UNSPECIFIED;
                        return EVENT_PROCESSED;
                    }
                }
                break;
            }
            case MotionNotify:
            {
                showMousePointer();
                if (hasMovingSelection) {
                    XEvent newEvent;
                    XSync(getDisplay(), False);
                    if (XCheckWindowEvent(getDisplay(), getWid(), ButtonMotionMask, &newEvent) == True) {
                        event = &newEvent;
                        while (XCheckWindowEvent(getDisplay(), getWid(), ButtonMotionMask, &newEvent) == True);
                    }

                    int x = event->xmotion.x;
                    int y = event->xmotion.y;
                    setNewMousePositionForMovingSelection(x, y);
                    currentActionCategory = ACTION_UNSPECIFIED;
                    return EVENT_PROCESSED;
                } else {
                    removeFromXEventMask(PointerMotionMask);
                }
                break;
            }
        }
        return propagateEventToParentWidget(event);
    }
}

void TextEditorWidget::replaceTextWithPrimarySelection()
{
    if (!cursorChangesDisabled && !isReadOnly())
    {
        pastingDataHistorySectionHolder = textData->createHistorySection();
        //textData->setHistorySeparator();
        
        textData->clear();
        
        requestSelectionPasting(createNewMarkFromCursor());
        currentActionCategory = ACTION_UNSPECIFIED;
    }
    assureCursorVisible();
    rememberedCursorPixX = getCursorPixX();
}


void TextEditorWidget::setNewMousePositionForMovingSelection(int x, int y)
{
    if (hasMovingSelection)
    {
        this->movingSelectionY = y;
        this->movingSelectionX = x;
        long newCursorPos = getTextPosFromPixXY(x, y);

        if (wasDoubleClick) {
            long doubleClickPos = getTextPosFromPixXY(x, y, false);
            long p1 = doubleClickPos;
            long p2 = p1;

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
                    if (!selectionOwner->hasSelectionOwnership()) {
                        selectionOwner->requestSelectionOwnership();
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
            long newCursorPos2 = newCursorPos;
            
            if (   this->getTopLineNumber() >= getNumberOfLines() - this->getNumberOfVisibleLines()
                && y >= 0 && y > getHeightPix())
            {
                newCursorPos2 = textData->getLength();  // special handling if last empty line is not displayed
            }
//            if (newCursorPos2 != getCursorTextPosition() || newCursorPos2 != getBackliteBuffer()->)
            {
                if (!selectionOwner->hasSelectionOwnership()) {
                    selectionOwner->requestSelectionOwnership();
                    getBackliteBuffer()->activateSelection(getCursorTextPosition());
                }
                getBackliteBuffer()->extendSelectionTo(newCursorPos2);
            }
        }
        moveCursorToTextPosition(newCursorPos);
        assureCursorVisible();
        rememberedCursorPixX = getCursorPixX();

        if (y < 0 ) {
            if (!isMovingSelectionScrolling) {
                scrollUp();
                isMovingSelectionScrolling = true;
                EventDispatcher::getInstance()->registerTimerCallback(Seconds(0), 
                        calculateScrollTime(-y, getLineHeight()),
                        scrollRepeatCallback);
            }
        } else if (y > getHeightPix()) {
            if (!isMovingSelectionScrolling) {
                scrollDown();
                isMovingSelectionScrolling = true;
                EventDispatcher::getInstance()->registerTimerCallback(Seconds(0), 
                        calculateScrollTime(y - getHeightPix(), getLineHeight()),
                        scrollRepeatCallback);
            }
        } else if (x < 0 ) {
            if (!isMovingSelectionScrolling) {
                scrollLeft();
                isMovingSelectionScrolling = true;
                EventDispatcher::getInstance()->registerTimerCallback(Seconds(0), 
                        calculateScrollTime(-x, getLineHeight()),
                        scrollRepeatCallback);
            }
        } else if (x > getPixWidth() && !textData->isEndOfLine(newCursorPos)) {
            if (!isMovingSelectionScrolling) {
                scrollRight();
                isMovingSelectionScrolling = true;
                EventDispatcher::getInstance()->registerTimerCallback(Seconds(0), 
                        calculateScrollTime(x - getPixWidth(), getLineHeight()),
                        scrollRepeatCallback);
            }
        } else {
            isMovingSelectionScrolling = false;
        }
    }
}



void TextEditorWidget::releaseSelection()
{
    if (getBackliteBuffer()->hasActiveSelection()) {
        getBackliteBuffer()->deactivateSelection();
    }
    selectionOwner->releaseSelectionOwnership();
}

void TextEditorWidget::releaseSelectionButKeepPseudoSelection()
{
    if (getBackliteBuffer()->hasActiveSelection()) {
        getBackliteBuffer()->makeSelectionToSecondarySelection();
    }
    if (selectionOwner->hasSelectionOwnership()) {
        selectionOwner->releaseSelectionOwnership();
    }
}



void TextEditorWidget::handleScrollRepeating()
{
    if (isMovingSelectionScrolling) {
        long newCursorPos = getTextPosFromPixXY(movingSelectionX, movingSelectionY);
        if (newCursorPos != getCursorTextPosition())
        {
            if (!selectionOwner->hasSelectionOwnership()) {
                selectionOwner->requestSelectionOwnership();
                getBackliteBuffer()->activateSelection(getCursorTextPosition());
            }
            moveCursorToTextPosition(newCursorPos);
            assureCursorVisible();
            rememberedCursorPixX = getCursorPixX();
            getBackliteBuffer()->extendSelectionTo(getCursorTextPosition());
        }
        if (movingSelectionY < 0 ) {
            scrollUp();
            EventDispatcher::getInstance()->registerTimerCallback(Seconds(0), 
                    calculateScrollTime(-movingSelectionY, getLineHeight()),
                    scrollRepeatCallback);
        } else if (movingSelectionY > getHeightPix()) {
            scrollDown();
            EventDispatcher::getInstance()->registerTimerCallback(Seconds(0), 
                    calculateScrollTime(movingSelectionY - getHeightPix(), getLineHeight()),
                    scrollRepeatCallback);
        } else if (movingSelectionX < 0 ) {
            scrollLeft();
            EventDispatcher::getInstance()->registerTimerCallback(Seconds(0), 
                    calculateScrollTime(-movingSelectionX, getLineHeight()),
                    scrollRepeatCallback);
        } else if (movingSelectionX > getPixWidth() && !textData->isEndOfLine(newCursorPos)) {
            scrollRight();
            EventDispatcher::getInstance()->registerTimerCallback(Seconds(0), 
                    calculateScrollTime(movingSelectionX - getPixWidth(), getLineHeight()),
                    scrollRepeatCallback);
        } else {
            isMovingSelectionScrolling = false;
        }
    }    
}

#if 0
GuiElement::ProcessingResult TextEditorWidget::processKeyboardEvent(const KeyPressEvent& keyPressEvent)
{
    if (handleLowPriorityKeyPress(keyPressEvent)) {
        return EVENT_PROCESSED;
    } else {
        return NOT_PROCESSED;
    }
}
#endif


bool TextEditorWidget::handleLowPriorityKeyPress(const KeyPressEvent& keyPressEvent)
{
    
    if (hasFocus() && !keyPressEvent.getKeyId().isModifierKey())
    {
        hideMousePointer();
    }
    
    //unsigned int buttonState = event->xkey.state & (ControlMask|Mod1Mask|ShiftMask);

    {
        if (keyPressEvent.hasInputString()) {
            if (!cursorChangesDisabled && !isReadOnly())
            {
                lastActionCategory = currentActionCategory;
                currentActionCategory = ACTION_KEYBOARD_INPUT;
                
                textData->setMergableHistorySeparator();
                TextData::HistorySection::Ptr historySectionHolder = textData->getHistorySectionHolder();
                
                hideCursor();
                if (hasPrimarySelection())
                {
                    long selBegin  = getBeginSelectionPos();
                    long selLength = getEndSelectionPos() - selBegin;
                    moveCursorToTextPosition(selBegin);
                    removeAtCursor(selLength);
                    releaseSelection();
                }
                else if (hasPseudoSelection())
                {
                    if (   (lastActionCategory != ACTION_KEYBOARD_INPUT && lastActionCategory != ACTION_TABULATOR)
                        || getCursorTextPosition() != getBackliteBuffer()->getEndSelectionPos())
                    {
                        releaseSelection();
                    }
                }
                if (!getBackliteBuffer()->hasActiveSelection())
                {
                    getBackliteBuffer()->activateSelection(getCursorTextPosition());
                    getBackliteBuffer()->makeSelectionToSecondarySelection();
                }
                long insertedLength = insertAtCursor(keyPressEvent.getInputString()[0]);
                moveCursorToTextPosition(getCursorTextPosition() + insertedLength);

                getBackliteBuffer()->extendSelectionTo(getCursorTextPosition());

                if (getCursorLineNumber() < getTopLineNumber()) {
                    setTopLineNumber(getCursorLineNumber());
                } else if ((getCursorLineNumber() - getTopLineNumber() + 1) * getLineHeight() > getHeightPix()) {
                    setTopLineNumber(getCursorLineNumber() - getNumberOfVisibleLines() + 1);
                }
                long cursorPixX = getCursorPixX();
                int spaceWidth = getDefaultTextStyle()->getSpaceWidth();

                if (cursorPixX < getLeftPix()) {
                    setLeftPix(cursorPixX - spaceWidth);
                } else if (cursorPixX >= getRightPix() - spaceWidth) {
                    setLeftPix(cursorPixX - (getRightPix() - getLeftPix()) + 2 * spaceWidth);
                }
                showCursor();
            }
            assureCursorVisible();
            rememberedCursorPixX = getCursorPixX();
            return EVENT_PROCESSED;
        } else {
            return NOT_PROCESSED;
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
    if (hasFocus()) {
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
    showMousePointer();
    setFocusFlag(true);

    isSelectionPersistent = false;
}


void TextEditorWidget::treatFocusOut()
{
    setCursorInactive();
    stopCursorBlinking();
    showMousePointer();
    setFocusFlag(false);

    if (getBackliteBuffer()->hasActiveSelection()) {
        isSelectionPersistent = true;
    }

}

void TextEditorWidget::showCursor()
{
    if (hasFocus()) {
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

bool TextEditorWidget::scrollDown()
{
    if (this->getTopLineNumber() < getNumberOfLines() - this->getNumberOfVisibleLines()) {
        this->setTopLineNumber(this->getTopLineNumber() + 1);
        return true;
    } else {
        return false;
    }
}


bool TextEditorWidget::scrollUp()
{
    long topLine = this->getTopLineNumber();
    if (topLine > 0) {
        this->setTopLineNumber(this->getTopLineNumber() - 1);
        return true;
    } else {
        return false;
    }
}


void TextEditorWidget::scrollLeft()
{
    long newLeft = this->getLeftPix() - this->getDefaultTextStyle()->getSpaceWidth();
    this->setLeftPix(newLeft);
}


void TextEditorWidget::scrollRight()
{
    long newLeft = this->getLeftPix() + this->getDefaultTextStyle()->getSpaceWidth();
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
    
    if (targetTopLine > getNumberOfLines() - this->getNumberOfVisibleLines()) {
        targetTopLine = getNumberOfLines() - this->getNumberOfVisibleLines();
    }
    this->setTopLineNumber(targetTopLine);
}


void TextEditorWidget::scrollPageLeft()
{
    int columns = this->getPixWidth() / this->getDefaultTextStyle()->getSpaceWidth();
    long newLeft = this->getLeftPix() - this->getDefaultTextStyle()->getSpaceWidth() * (columns/2);
    this->setLeftPix(newLeft);
}


void TextEditorWidget::scrollPageRight()
{
    int columns = this->getPixWidth() / this->getDefaultTextStyle()->getSpaceWidth();
    long newLeft = this->getLeftPix() + this->getDefaultTextStyle()->getSpaceWidth() * (columns/2);
    this->setLeftPix(newLeft);
}

TextEditorWidget::ActionCategory TextEditorWidget::getLastActionCategory() const
{
    return lastActionCategory;
}

void TextEditorWidget::setCurrentActionCategory(TextEditorWidget::ActionCategory actionCategory)
{
    this->currentActionCategory = actionCategory;
}

