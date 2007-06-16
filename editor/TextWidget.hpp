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

#ifndef TEXTWIDGET_H
#define TEXTWIDGET_H

#include "GuiWidget.hpp"
#include "TextData.hpp"
#include "LineInfo.hpp"
#include "TextStyle.hpp"
#include "TimeVal.hpp"
#include "HilitingBuffer.hpp"
#include "BackliteBuffer.hpp"
#include "CallbackContainer.hpp"
#include "OwningPtr.hpp"
#include "GuiColor.hpp"

namespace LucED {

class TextWidget : public GuiWidget
{
public:
    typedef OwningPtr<TextWidget> Ptr;
    typedef Callback1<long> ChangedValueCallback;
    
    virtual ~TextWidget();
    
    BackliteBuffer* getBackliteBuffer() {
        return backliteBuffer.getRawPtr();
    }

protected:
    TextWidget(GuiWidget *parent, TextStyles::Ptr textStyles, HilitedText::Ptr hilitedText, int border);

    void showMousePointer() {
        if (isMousePointerHidden) {
            internalShowMousePointer();
        }
    }
    void hideMousePointer() {
        if (!isMousePointerHidden) {
            internalHideMousePointer();
        }
    }
    
public:
    virtual void setPosition(Position newPosition);
    
    void setResizeAdjustment(VerticalAdjustment::Type adjustment);
    
    virtual Measures getDesiredMeasures();
    
    void setDesiredMeasuresInChars(int minWidth, int minHeight, 
            int bestWidth, int bestHeight, int maxWidth, int maxHeight);

    void setDesiredMeasuresInChars(int bestWidth, int bestHeight);
    
    TextData* getTextData() {
        return textData.getRawPtr();
    }
    
    TextStyles::Ptr getTextStyles() const {
        return textStyles;
    }
    
    LanguageMode::Ptr getLanguageMode() const {
        return hilitingBuffer->getLanguageMode();
    }
    
    HilitedText::Ptr getHilitedText() const {
        return hilitingBuffer->getHilitedText();
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
    long getOpticalCursorColumn() const;
    
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
    int getSpaceCharWidth() const {
        return textStyles->get(0)->getSpaceWidth();
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
    long insertAtCursor(char c);
    long insertAtCursor(const byte* buffer, long length);
    long insertAtCursor(const String& s) {
        return insertAtCursor((const byte*) s.toCString(), s.getLength());
    }
    long insertAtCursor(TextData* d) {
        return insertAtCursor(d->getAmount(0, d->getLength()), d->getLength());
    }
    long insertAtCursor(const ByteArray& buffer);
    void removeAtCursor(long amount);
    
    virtual ProcessingResult processEvent(const XEvent *event);

    bool isCursorBlinking() const {
        return cursorIsBlinking;
    }    
    void startCursorBlinking();
    void stopCursorBlinking();
    void setCursorActive();
    void setCursorInactive();
    void hideCursor();
    void showCursor();

    void registerLineAndColumnListener(const Callback2<long,long>& listener);
    void notifyAboutHotKeyEventForOtherWidget();
        
private:
    
    GuiColor getColorForBackground(byte background);
    
    void internalShowMousePointer();
    void internalHideMousePointer();
    
    void unclip();
    void clip(int x, int y, int w, int h);
    void calcTotalPixWidth();
    long calcLongestVisiblePixWidth();
    void fillLineInfo(long beginOfLinePos, LineInfo* li);
    LineInfo* getValidLineInfo(long line);
    void appendToOutBuf(LineInfo *li, long pos,
            int c, int background, int style, int *print, long *pixelPos, int* lastBackground, int* lastStyle, 
            int* lastStyleBegin, long* lastStylePixBegin);
    void redrawChanged(long spos, long epos);
    void redraw();
    void drawPartialArea(int minY, int maxY, int x1, int x2);
    void drawArea(int minY, int maxY);
    int  calcVisiblePixX(LineInfo *li, long pos);
    void printPartialLine(LineInfo *li, int y, int x1, int x2);
    void printPartialLineWithoutCursor(LineInfo *li, int y, int x1, int x2);
    void printLine(LineInfo *li, int y);
    void printChangedPartOfLine(LineInfo* newLi, int y, LineInfo* oldLi);
    void clearLine(LineInfo *li, int y);
    void clearPartialLine(LineInfo *li, int y, int x1, int x2);
    void applyTextStyle(int styleIndex);
    void internSetLeftPix(long leftPix);

    Callback0 cursorBlinkCallback;
    void blinkCursor();

    void treatTextDataUpdate(TextData::UpdateInfo update);

    void flushPendingUpdates();
    
    void treatHilitingUpdate(HilitingBuffer::UpdateInfo update);
    
    void drawCursor(long cursorPos);
    
    void processAllExposureEvents();
    
    TimeVal cursorNextBlinkTime;
    
    Position position;
    TextData::Ptr textData;

    TextStyles::Ptr textStyles;
    HilitingBuffer::Ptr hilitingBuffer;
    BackliteBuffer::Ptr backliteBuffer;

    TextData::TextMark topMarkId; // first column of the first displayed textline
    TextData::TextMark cursorMarkId;
    long opticalCursorColumn;

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
    
    int minWidthChars;
    int minHeightChars;
    int bestWidthChars;
    int bestHeightChars;
    int maxWidthChars;
    int maxHeightChars;
    int border;
    VerticalAdjustment::Type adjustment;
    bool isMousePointerHidden;
    GuiColor primarySelectionColor;
    GuiColor secondarySelectionColor;
};

} // namespace LucED

#endif // TEXTWIDGET_H

