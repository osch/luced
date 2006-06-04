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

#ifndef TEXTEDITORWIDGET_H
#define TEXTEDITORWIDGET_H

#include "TextWidget.h"
#include "HilitingBuffer.h"
#include "KeyMapping.h"
#include "SelectionOwner.h"
#include "PasteDataReceiver.h"
#include "OwningPtr.h"

namespace LucED {

class TextEditorWidget : public TextWidget, SelectionOwner, PasteDataReceiver
{
public:
    typedef OwningPtr<TextEditorWidget> Ptr;
    typedef Callback1<long> ChangedValueCallback;

    static TextEditorWidget::Ptr create(GuiWidget *parent, 
            TextData::Ptr textData, TextStyles::Ptr textStyles, HilitingBuffer::Ptr hilitingBuffer) {
        return TextEditorWidget::Ptr(new TextEditorWidget(parent, 
                textData, textStyles, hilitingBuffer));
    }

    virtual ~TextEditorWidget() {}

    void assureCursorVisible();
    void cursorLeft();
    void cursorRight();
    void cursorDown();
    void cursorUp();
    void cursorPageDown();
    void cursorPageUp();
    void cursorBeginOfLine();
    void cursorEndOfLine();
    void scrollDown();
    void scrollUp();
    void scrollLeft();
    void scrollRight();
    void scrollPageUp();
    void scrollPageDown();
    void scrollPageLeft();
    void scrollPageRight();
    void cursorBeginOfText();
    void cursorEndOfText();
    void newLine();
    void backSpace();
    void deleteKey();
    void copyToClipboard();
    void pasteFromClipboard();
    void selectAll();
    void selectionCursorLeft();
    void selectionCursorRight();
    void selectionCursorDown();
    void selectionCursorUp();
    void cursorWordLeft();
    void cursorWordRight();
    void selectionCursorWordLeft();
    void selectionCursorWordRight();
    void selectionCursorBeginOfLine();
    void selectionCursorEndOfLine();
    void selectionCursorPageDown();
    void selectionCursorPageUp();
    
    void showCursor();
    void hideCursor();

    virtual bool processEvent(const XEvent *event);
    virtual bool processKeyboardEvent(const XEvent *event);

    void treatFocusIn();
    void treatFocusOut();
    void disableCursorChanges();
    void enableCursorChanges();
    
    Slot1<ScrollStep::Type> slotForScrollStepV;
    Slot1<ScrollStep::Type> slotForScrollStepH;

private:
    TextEditorWidget(GuiWidget *parent, 
            TextData::Ptr textData, TextStyles::Ptr textStyles, HilitingBuffer::Ptr hilitingBuffer);

    virtual long  initSelectionDataRequest();
    virtual const byte* getSelectionDataChunk(long pos, long length);
    virtual void  endSelectionDataRequest();
    virtual void notifyAboutLostSelectionOwnership();

    virtual void notifyAboutReceivedPasteData(const byte* data, long length);
    virtual void notifyAboutEndOfPastingData();
    virtual void notifyAboutBeginOfPastingData();
    
    void handleScrollStepV(ScrollStep::Type scrollStep);
    void handleScrollStepH(ScrollStep::Type scrollStep);

    bool hasFocusFlag;
    KeyMapping<TextEditorWidget> keyMapping;
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
