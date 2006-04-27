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

#ifndef TEXTWIDGET_H
#define TEXTWIDGET_H

#include "GuiWidget.h"
#include "TextData.h"
#include "Slot.h"
#include "LineInfo.h"
#include "TextStyle.h"
#include "TimeVal.h"
#include "HilitingBuffer.h"
#include "BackliteBuffer.h"
#include "CallbackContainer.h"

namespace LucED {

class TextWidget : public GuiWidget
{
public:
    typedef HeapObjectPtr<TextWidget> Ptr;
    typedef Callback1<long> ChangedValueCallback;
    
    virtual ~TextWidget();
    
protected:
    TextWidget(GuiWidget *parent, 
            TextData::Ptr textData, TextStyles::Ptr textStyles, 
            HilitingBuffer::Ptr hilitingBuffer);

    BackliteBuffer* getBackliteBuffer() {
        return backliteBuffer.getRawPtr();
    }

public:
    virtual void setPosition(Position newPosition);
    virtual Measures getDesiredMeasures();
    
    TextData* getTextData() {
        return textData.getRawPtr();
    }
    
    TextStyles* getTextStyles() {
        return textStyles.getRawPtr();
    }

    long getTopLeftTextPosition() const {
        return textData->getTextPositionOfMark(topMarkId);
    }
    long getCursorTextPosition() const {
        return textData->getTextPositionOfMark(cursorMarkId);
    }
    long getCursorLineNumber() const {
        return textData->getLineNumberOfMark(cursorMarkId);
    }
    long getCursorColumn() const {
        return textData->getColumnNumberOfMark(cursorMarkId);
    }
    int getNumberOfVisibleLines() const {
        return visibleLines;
    }
    
    long getCursorPixX();
    long getLeftPix() const {
        return leftPix;
    }
    long getRightPix() const {
        return leftPix + position.w;
    }
    long getPixWidth() const {
        return position.w;
    }
    unsigned int getHeightPix() const {
        return position.h;
    }
    int getLineHeight() const {
        return lineHeight;
    }
    
    TextData::TextMark createNewMarkFromCursor();

    void moveCursorToTextPosition(long pos);
    void moveCursorToTextMark(TextData::MarkHandle m);

    long getTopLineNumber() const {
        return textData->getLineNumberOfMark(topMarkId);
    }
    void setScrollBarVerticalValueRangeChangedCallback(const Callback3<long,long,long>& callback) {
        scrollBarVerticalValueRangeChangedCallback = callback;
        scrollBarVerticalValueRangeChangedCallback.call(textData->getNumberOfLines(), visibleLines, getTopLineNumber());
    }
    void setScrollBarHorizontalValueRangeChangedCallback(const Callback3<long,long,long>& callback) {
        scrollBarHorizontalValueRangeChangedCallback = callback;
        scrollBarHorizontalValueRangeChangedCallback.call(totalPixWidth, position.w, leftPix);
    }
    void setTopLineNumber(long n);
    void setLeftPix(long leftPix);
    long getTextPosFromPixXY(int pixX, int pixY, bool optimizeForThinCursor = true);
    long getTextPosForPixX(long pixX, long beginOfLinePos);
    void insertAtCursor(char c);
    void insertAtCursor(const byte* buffer, long length);
    void insertAtCursor(const ByteArray& buffer);
    void removeAtCursor(long amount);
    
    virtual bool processEvent(const XEvent *event);
    
    void startCursorBlinking();
    void stopCursorBlinking();
    void setCursorActive();
    void setCursorInactive();
    void hideCursor();
    void showCursor();

    Slot1<long> slotForVerticalScrollBarChangedValue;
    Slot1<long> slotForHorizontalScrollBarChangedValue;
    
    void registerLineAndColumnListener(const Callback2<long,long>& listener);
    
private:
    
    void unclip();
    void clip(int x, int y, int w, int h);
    void calcTotalPixWidth();
    void fillLineInfo(long beginOfLinePos, LineInfo* li);
    LineInfo* getValidLineInfo(long line);
    void appendToOutBuf(LineInfo *li, long pos,
            int c, int background, int style, int *print, long *pixelPos, int* lastBackground, int* lastStyle, 
            int* lastStyleBegin, long* lastStylePixBegin);
    void redrawChanged();
    void redraw();
    void drawPartialArea(int minY, int maxY, int x1, int x2);
    void drawArea(int minY, int maxY);
    int  calcVisiblePixX(LineInfo *li, long pos);
    void printPartialLine(LineInfo *li, int y, int x1, int x2);
    void printPartialLineWithoutCursor(LineInfo *li, int y, int x1, int x2);
    void printLine(LineInfo *li, int y);
    void printLineWithoutCursor(LineInfo *li, int y);
    void clearLine(LineInfo *li, int y);
    void clearPartialLine(LineInfo *li, int y, int x1, int x2);
    void applyTextStyle(int styleIndex);
    void internSetLeftPix(long leftPix);

    Slot0 slotForCursorBlinking;
    void blinkCursor();

    Slot1<TextData::UpdateInfo> slotForTextDataUpdateTreatment;
    void treatTextDataUpdate(TextData::UpdateInfo update);

    Slot0 slotForFlushPendingUpdates;
    void flushPendingUpdates();
    
    Slot1<HilitingBuffer::UpdateInfo> slotForHilitingUpdateTreatment;
    void treatHilitingUpdate(HilitingBuffer::UpdateInfo update);
    
    void drawCursor(long cursorPos);
    
    TimeVal cursorNextBlinkTime;
    
    Position position;
    TextData::Ptr textData;

    TextStyles::Ptr textStyles;
    HilitingBuffer::Ptr hilitingBuffer;
    BackliteBuffer::Ptr backliteBuffer;

    TextData::TextMark topMarkId; // first column of the first displayed textline
    TextData::TextMark cursorMarkId;

    long visibleLines;
    int lineHeight;
    int lineAscent;
    LineInfos lineInfos;
    Callback3<long,long,long> scrollBarVerticalValueRangeChangedCallback;
    Callback3<long,long,long> scrollBarHorizontalValueRangeChangedCallback;
    long totalPixWidth;
    long leftPix;
    long endPos;
    bool cursorVisible;
    bool cursorIsBlinking;
    bool cursorIsActive;
    
    LineInfo tempLineInfo;

    bool updateVerticalScrollBar;
    bool updateHorizontalScrollBar;

    Region redrawRegion; // collects Rectangles for redraw events
    
    Callback2Container<long,long> lineAndColumnListeners;
};

} // namespace LucED

#endif // TEXTWIDGET_H

