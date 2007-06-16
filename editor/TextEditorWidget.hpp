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

#ifndef TEXTEDITORWIDGET_H
#define TEXTEDITORWIDGET_H

#include "TextWidget.hpp"
#include "KeyMapping.hpp"
#include "SelectionOwner.hpp"
#include "PasteDataReceiver.hpp"
#include "OwningPtr.hpp"
#include "WeakPtr.hpp"
#include "Callback.hpp"

namespace LucED {

class TextEditorWidget : public TextWidget, public SelectionOwner, public PasteDataReceiver
{
public:
    typedef void (EditActionFunction)(TextEditorWidget *);

    enum ActionId
    {
        ACTION_UNSPECIFIED,
        ACTION_TABULATOR,
        ACTION_NEWLINE,
        ACTION_KEYBOARD_INPUT,
    };

    ActionId getLastAction() const;
    void setCurrentAction(ActionId action);
    
    bool isCursorVisible();
    void assureCursorVisible();
    void moveCursorToTextMarkAndAdjustVisibility(TextData::MarkHandle m);
    void moveCursorToTextPositionAndAdjustVisibility(int position);
    void adjustCursorVisibility();
    
    void showCursor();
    void hideCursor();

    virtual ProcessingResult processEvent(const XEvent *event);
    virtual ProcessingResult processKeyboardEvent(const XEvent *event);

    void treatFocusIn();
    void treatFocusOut();
    void disableCursorChanges();
    void enableCursorChanges();
    bool areCursorChangesDisabled()  { return cursorChangesDisabled; }
    void rememberCursorPixX()        { rememberedCursorPixX = getCursorPixX(); }
    int  getRememberedCursorPixX()   { return rememberedCursorPixX; }
    bool isWordCharacter(unsigned char c);
    
    template<class T> void setEditAction(int keyState, KeySym keySym, T* object, void (T::*method)()) {
        keyMapping.set(keyState, keySym, Callback0(object, method));
    }
    
    bool scrollUp();
    bool scrollDown();
    void scrollLeft();
    void scrollRight();
    void scrollPageUp();
    void scrollPageDown();
    void scrollPageLeft();
    void scrollPageRight();

    void handleScrollStepV(ScrollStep::Type scrollStep);
    void handleScrollStepH(ScrollStep::Type scrollStep);

    void showMousePointer() {
        TextWidget::showMousePointer();
    }
    void hideMousePointer() {
        TextWidget::hideMousePointer();
        addToXEventMask(PointerMotionMask);
    }

    virtual void releaseSelectionOwnership();
    void releaseSelectionOwnershipButKeepPseudoSelection();
    
    void requestClipboardPasting(const TextData::TextMark& m) {
        beginPastingTextMark = getTextData()->createNewMark(m);
        PasteDataReceiver::requestClipboardPasting();
    }

    void requestSelectionPasting(const TextData::TextMark& m) {
        beginPastingTextMark = getTextData()->createNewMark(m);
        PasteDataReceiver::requestSelectionPasting();
    }

protected:
    TextEditorWidget(GuiWidget *parent, TextStyles::Ptr textStyles, HilitedText::Ptr hilitedText, int borderWidth);

    virtual void notifyAboutReceivedPasteData(const byte* data, long length);
    virtual void notifyAboutEndOfPastingData();
    virtual void notifyAboutBeginOfPastingData();

private:
    virtual long  initSelectionDataRequest();
    virtual const byte* getSelectionDataChunk(long pos, long length);
    virtual void  endSelectionDataRequest();
    virtual void notifyAboutLostSelectionOwnership();

    void setNewMousePositionForMovingSelection(int x, int y);
    
    bool hasFocusFlag;
    KeyMapping keyMapping;
    long rememberedCursorPixX;
    
    bool hasMovingSelection;
    int movingSelectionX;
    int movingSelectionY;
    bool isMovingSelectionScrolling;
    Callback0 scrollRepeatCallback;
    void handleScrollRepeating();
    
    bool cursorChangesDisabled;
    
    Time lastButtonPressedTime;
    int buttonPressedCounter;
    bool wasDoubleClick;
    ActionId lastActionId;
    ActionId currentActionId;
    TextData::TextMark beginPastingTextMark;
    TextData::TextMark pastingTextMark;
};

} // namespace LucED

#endif // TEXTEDITORWIDGET_H
