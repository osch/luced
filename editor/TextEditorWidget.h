/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, oliver at luced dot de
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

#include "TextWidget.h"
#include "KeyMapping.h"
#include "SelectionOwner.h"
#include "PasteDataReceiver.h"
#include "OwningPtr.h"
#include "WeakPtr.h"
#include "Callback.h"

namespace LucED {

class TextEditorWidget : public TextWidget, public SelectionOwner, public PasteDataReceiver
{
public:
    typedef void (EditActionFunction)(TextEditorWidget *);

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
    
    
    Slot1<ScrollStep::Type> slotForScrollStepV;
    Slot1<ScrollStep::Type> slotForScrollStepH;

    void scrollUp();
    void scrollDown();
    void scrollLeft();
    void scrollRight();
    void scrollPageUp();
    void scrollPageDown();
    void scrollPageLeft();
    void scrollPageRight();

    void showMousePointer() {
        TextWidget::showMousePointer();
    }
    void hideMousePointer() {
        TextWidget::hideMousePointer();
        addToXEventMask(PointerMotionMask);
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
    
    void handleScrollStepV(ScrollStep::Type scrollStep);
    void handleScrollStepH(ScrollStep::Type scrollStep);

    bool hasFocusFlag;
    KeyMapping keyMapping;
    long rememberedCursorPixX;
    
    bool hasMovingSelection;
    int movingSelectionX;
    int movingSelectionY;
    bool isMovingSelectionScrolling;
    Slot0 slotForScrollRepeating;
    void handleScrollRepeating();
    
    bool cursorChangesDisabled;
    
    Time lastButtonPressedTime;
    int buttonPressedCounter;
    bool wasDoubleClick;
};

} // namespace LucED

#endif // TEXTEDITORWIDGET_H
