
#include <X11/keysym.h>
#include <X11/Xatom.h>

#include "Clipboard.h"
#include "StandardEditActions.h"

using namespace LucED;


void StandardEditActions::cursorLeft(TextEditorWidget *e)
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


void StandardEditActions::cursorRight(TextEditorWidget *e)
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

void StandardEditActions::cursorDown(TextEditorWidget *e)
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


void StandardEditActions::cursorUp(TextEditorWidget *e)
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

void StandardEditActions::cursorWordLeft(TextEditorWidget *e)
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


void StandardEditActions::cursorWordRight(TextEditorWidget *e)
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



void StandardEditActions::selectionCursorLeft(TextEditorWidget *e)
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


void StandardEditActions::selectionCursorRight(TextEditorWidget *e)
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

void StandardEditActions::selectionCursorDown(TextEditorWidget *e)
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


void StandardEditActions::selectionCursorUp(TextEditorWidget *e)
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


void StandardEditActions::selectionCursorWordLeft(TextEditorWidget *e)
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


void StandardEditActions::selectionCursorWordRight(TextEditorWidget *e)
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


void StandardEditActions::cursorPageDown(TextEditorWidget *e)
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


void StandardEditActions::cursorPageUp(TextEditorWidget *e)
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

void StandardEditActions::selectionCursorPageDown(TextEditorWidget *e)
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


void StandardEditActions::selectionCursorPageUp(TextEditorWidget *e)
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

void StandardEditActions::cursorBeginOfLine(TextEditorWidget *e)
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


void StandardEditActions::cursorEndOfLine(TextEditorWidget *e)
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

void StandardEditActions::selectionCursorBeginOfLine(TextEditorWidget *e)
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


void StandardEditActions::selectionCursorEndOfLine(TextEditorWidget *e)
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


void StandardEditActions::cursorBeginOfText(TextEditorWidget *e)
{
    if (!e->areCursorChangesDisabled())
    {
        e->releaseSelectionOwnership();
        e->moveCursorToTextPosition(0);
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
}


void StandardEditActions::cursorEndOfText(TextEditorWidget *e)
{
    if (!e->areCursorChangesDisabled())
    {
        e->releaseSelectionOwnership();
        e->moveCursorToTextPosition(e->getTextData()->getLength());
    }
    e->assureCursorVisible();
    e->rememberCursorPixX();
}


void StandardEditActions::scrollDown(TextEditorWidget *e)
{
    e->scrollDown();
}


void StandardEditActions::scrollUp(TextEditorWidget *e)
{
    e->scrollUp();
}


void StandardEditActions::scrollLeft(TextEditorWidget *e)
{
    e->scrollLeft();
}


void StandardEditActions::scrollRight(TextEditorWidget *e)
{
    e->scrollRight();
}


void StandardEditActions::scrollPageUp(TextEditorWidget *e)
{
    e->scrollPageUp();
}


void StandardEditActions::scrollPageDown(TextEditorWidget *e)
{
    e->scrollPageDown();
}


void StandardEditActions::scrollPageLeft(TextEditorWidget *e)
{
    e->scrollPageLeft();
}


void StandardEditActions::scrollPageRight(TextEditorWidget *e)
{
    e->scrollPageRight();
}


void StandardEditActions::newLine(TextEditorWidget *e)
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


void StandardEditActions::backSpace(TextEditorWidget *e)
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

void StandardEditActions::deleteKey(TextEditorWidget *e)
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


void StandardEditActions::copyToClipboard(TextEditorWidget *e)
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

void StandardEditActions::selectAll(TextEditorWidget *e)
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



void StandardEditActions::pasteFromClipboard(TextEditorWidget *e)
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

void StandardEditActions::addSingleLineEditActions(TextEditorWidget* e)
{
    e->setEditAction(                    0, XK_Left,      &cursorLeft);
    e->setEditAction(                    0, XK_Right,     &cursorRight);
    e->setEditAction(                    0, XK_KP_Left,   &cursorLeft);
    e->setEditAction(                    0, XK_KP_Right,  &cursorRight);

    e->setEditAction(             Mod1Mask, XK_Left,      &cursorBeginOfLine);
    e->setEditAction(             Mod1Mask, XK_Right,     &cursorEndOfLine);
    e->setEditAction(             Mod1Mask, XK_KP_Left,   &cursorBeginOfLine);
    e->setEditAction(             Mod1Mask, XK_KP_Right,  &cursorEndOfLine);

    e->setEditAction(                    0, XK_Home,      &cursorBeginOfLine);
    e->setEditAction(                    0, XK_Begin,     &cursorBeginOfLine);
    e->setEditAction(                    0, XK_End,       &cursorEndOfLine);

    e->setEditAction( ControlMask|Mod1Mask, XK_Left,      &scrollLeft);
    e->setEditAction( ControlMask|Mod1Mask, XK_Right,     &scrollRight);
    e->setEditAction( ControlMask|Mod1Mask, XK_KP_Left,   &scrollLeft);
    e->setEditAction( ControlMask|Mod1Mask, XK_KP_Right,  &scrollRight);

    e->setEditAction(          ControlMask, XK_Home,      &cursorBeginOfText);
    e->setEditAction(          ControlMask, XK_Begin,     &cursorBeginOfText);
    e->setEditAction(          ControlMask, XK_End,       &cursorEndOfText);

    e->setEditAction(                    0, XK_BackSpace, &backSpace);
    e->setEditAction(                    0, XK_Delete,    &deleteKey);

    e->setEditAction(          ControlMask, XK_c,         &copyToClipboard);
    e->setEditAction(          ControlMask, XK_v,         &pasteFromClipboard);
    e->setEditAction(          ControlMask, XK_a,         &selectAll);

    e->setEditAction(            ShiftMask, XK_Left,      &selectionCursorLeft);
    e->setEditAction(            ShiftMask, XK_Right,     &selectionCursorRight);
    e->setEditAction(            ShiftMask, XK_KP_Left,   &selectionCursorLeft);
    e->setEditAction(            ShiftMask, XK_KP_Right,  &selectionCursorRight);
    
    e->setEditAction(          ControlMask, XK_Left,      &cursorWordLeft);
    e->setEditAction(          ControlMask, XK_Right,     &cursorWordRight);
    e->setEditAction(ShiftMask|ControlMask, XK_Left,      &selectionCursorWordLeft);
    e->setEditAction(ShiftMask|ControlMask, XK_Right,     &selectionCursorWordRight);

    e->setEditAction(            ShiftMask, XK_Home,      &selectionCursorBeginOfLine);
    e->setEditAction(            ShiftMask, XK_Begin,     &selectionCursorBeginOfLine);
    e->setEditAction(            ShiftMask, XK_End,       &selectionCursorEndOfLine);
}


void StandardEditActions::addMultiLineEditActions(TextEditorWidget* e)
{
    StandardEditActions::addSingleLineEditActions(e);

    e->setEditAction(                    0, XK_Down,      &cursorDown);
    e->setEditAction(                    0, XK_Up,        &cursorUp);
    e->setEditAction(                    0, XK_KP_Down,   &cursorDown);
    e->setEditAction(                    0, XK_KP_Up,     &cursorUp);
    
    e->setEditAction(                    0, XK_Page_Down, &cursorPageDown);
    e->setEditAction(                    0, XK_Page_Up,   &cursorPageUp);
    e->setEditAction(             Mod1Mask, XK_Down,      &cursorPageDown);
    e->setEditAction(             Mod1Mask, XK_Up,        &cursorPageUp);
    
    e->setEditAction( ControlMask|Mod1Mask, XK_Down,      &scrollDown);
    e->setEditAction( ControlMask|Mod1Mask, XK_Up,        &scrollUp);
    e->setEditAction( ControlMask|Mod1Mask, XK_KP_Down,   &scrollDown);
    e->setEditAction( ControlMask|Mod1Mask, XK_KP_Up,     &scrollUp);

    e->setEditAction(                    0, XK_Return,    &newLine);
    e->setEditAction(                    0, XK_KP_Enter,  &newLine);

    e->setEditAction(            ShiftMask, XK_Down,      &selectionCursorDown);
    e->setEditAction(            ShiftMask, XK_Up,        &selectionCursorUp);
    e->setEditAction(            ShiftMask, XK_KP_Down,   &selectionCursorDown);
    e->setEditAction(            ShiftMask, XK_KP_Up,     &selectionCursorUp);
    
    e->setEditAction(            ShiftMask, XK_Page_Down, &selectionCursorPageDown);
    e->setEditAction(            ShiftMask, XK_Page_Up,   &selectionCursorPageUp);
}

