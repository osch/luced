
#include <X11/keysym.h>
#include <X11/Xatom.h>

#include "Clipboard.h"
#include "StandardEditActions.h"

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

void StandardEditActions::cursorWordLeft()
{
    if (!e->areCursorChangesDisabled())
    {
        e->releaseSelectionOwnership();

        long pos = e->getCursorTextPosition();
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

        long pos = e->getCursorTextPosition();
        long len = e->getTextData()->getLength();
        while (pos < len && e->isWordCharacter(e->getTextData()->getChar(pos))) {
            ++pos;
        }
        while (pos < len && !e->isWordCharacter(e->getTextData()->getChar(pos))) {
            ++pos;
        }
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
        while (pos < len && e->isWordCharacter(e->getTextData()->getChar(pos))) {
            ++pos;
        }
        while (pos < len && !e->isWordCharacter(e->getTextData()->getChar(pos))) {
            ++pos;
        }
        e->moveCursorToTextPosition(pos);
        e->getBackliteBuffer()->extendSelectionTo(e->getCursorTextPosition());
    }
    e->assureCursorVisible();
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


void StandardEditActions::scrollUp()
{
    e->scrollUp();
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


void StandardEditActions::backSpace()
{
    if (!e->areCursorChangesDisabled())
    {
        e->hideCursor();
        if (e->hasSelectionOwnership()) {
            long selBegin = e->getBackliteBuffer()->getBeginSelectionPos();
            long selLength = e->getBackliteBuffer()->getEndSelectionPos() - selBegin;
            e->moveCursorToTextPosition(selBegin);
            e->removeAtCursor(selLength);
            e->releaseSelectionOwnership();
        } else {
            long pos = e->getCursorTextPosition();
            if (pos > 0) {
                e->moveCursorToTextPosition(pos - 1);
                e->removeAtCursor(1);
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
        if (e->hasSelectionOwnership()) {
            long selBegin = e->getBackliteBuffer()->getBeginSelectionPos();
            long selLength = e->getBackliteBuffer()->getEndSelectionPos() - selBegin;
            e->moveCursorToTextPosition(selBegin);
            e->removeAtCursor(selLength);
            e->releaseSelectionOwnership();
        } else {
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
        if (e->getBackliteBuffer()->hasActiveSelection()) {
            long selBegin = e->getBackliteBuffer()->getBeginSelectionPos();
            long selLength = e->getBackliteBuffer()->getEndSelectionPos() - selBegin;
            Clipboard::getInstance()->copyToClipboard(e->getTextData()->getAmount(selBegin, selLength), selLength);
        }
    }
}

void StandardEditActions::selectAll()
{
    if (!e->areCursorChangesDisabled() && e->getTextData()->getLength() > 0)
    {
        e->requestSelectionOwnership();
        if (e->hasSelectionOwnership()) {
            e->getBackliteBuffer()->activateSelection(0);
            e->getBackliteBuffer()->extendSelectionTo(e->getTextData()->getLength());
        }
    }
}



void StandardEditActions::pasteFromClipboard()
{
    if (!e->areCursorChangesDisabled())
    {
        if (e->hasSelectionOwnership()) {
            long selBegin = e->getBackliteBuffer()->getBeginSelectionPos();
            long selLength = e->getBackliteBuffer()->getEndSelectionPos() - selBegin;
            e->moveCursorToTextPosition(selBegin);
            e->removeAtCursor(selLength);
        }
        e->requestClipboardPasting();
        if (e->hasSelectionOwnership()) {
            e->releaseSelectionOwnership();
        }
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
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
    e->setEditAction(             Mod1Mask, XK_Down,      this, &StandardEditActions::cursorPageDown);
    e->setEditAction(             Mod1Mask, XK_Up,        this, &StandardEditActions::cursorPageUp);
    
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
    
    e->setEditAction(            ShiftMask, XK_Page_Down, this, &StandardEditActions::selectionCursorPageDown);
    e->setEditAction(            ShiftMask, XK_Page_Up,   this, &StandardEditActions::selectionCursorPageUp);
}

