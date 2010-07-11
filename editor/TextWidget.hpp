/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2010 Oliver Schmidt, oliver at luced dot de
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

#ifndef TEXT_WIDGET_HPP
#define TEXT_WIDGET_HPP

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
#include "CursorPositionData.hpp"
#include "RawPtr.hpp"
#include "Flags.hpp"
#include "ScrollBar.hpp"
#include "FocusableWidget.hpp"
                    
namespace LucED
{

class TextWidget : public FocusableWidget
{
public:
    typedef OwningPtr<TextWidget> Ptr;
    
    enum CreateOption
    {
        READ_ONLY,
        NEVER_SHOW_CURSOR
    };
    typedef Flags<CreateOption> CreateOptions;
    
    virtual ~TextWidget();
    
protected:
    TextWidget(HilitedText::Ptr hilitedText, int border,
               CreateOptions options);

    void setScrollBarVerticalValueRangeChangedCallback(Callback<long,long,long>::Ptr callback) {
        scrollBarVerticalValueRangeChangedCallback = callback;
        scrollBarVerticalValueRangeChangedCallback->call(getNumberOfLines(), visibleLines, getTopLineNumber());
    }
    void setScrollBarHorizontalValueRangeChangedCallback(Callback<long,long,long>::Ptr callback) {
        scrollBarHorizontalValueRangeChangedCallback = callback;
        scrollBarHorizontalValueRangeChangedCallback->call(totalPixWidth, getWidth(), leftPix);
    }

    BackliteBuffer* getBackliteBuffer() {
        return backliteBuffer.getRawPtr();
    }

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
    struct FreePos
    {
        FreePos(long pos, long extraColumns)
            : pos(pos),
              extraColumns(extraColumns)
        {}
        
        long pos;
        long extraColumns;
    };

    virtual void setPosition(const Position& p);
                                                                 
    enum VerticalAdjustmentStrategy {
           STRICT_TOP_LINE_ANCHOR,
       NOT_STRICT_TOP_LINE_ANCHOR,
               BOTTOM_LINE_ANCHOR
    };
    
    enum HorizontalAdjustmentStrategy {
           STRICT_LEFT_COLUMN_ANCHOR,
       NOT_STRICT_LEFT_COLUMN_ANCHOR,
                 RIGHT_COLUMN_ANCHOR
    };
    
    void setVerticalAdjustmentStrategy(VerticalAdjustmentStrategy verticalAdjustmentStrategy);
    void setHorizontalAdjustmentStrategy(HorizontalAdjustmentStrategy horizontalAdjustmentStrategy);
    
    enum LastEmptyLineStrategy {
        DO_NOT_IGNORE_EMPTY_LAST_LINE,
        IGNORE_EMPTY_LAST_LINE
    };
    
    void setLastEmptyLineStrategy(LastEmptyLineStrategy lastEmptyLineStrategy);
    
    void setDesiredMeasuresInChars(int minWidth, int minHeight, 
            int bestWidth, int bestHeight, int maxWidth, int maxHeight);

    void setDesiredMeasuresInChars(int bestWidth, int bestHeight);
    
    virtual void setBackgroundColor(GuiColor color);
    
    TextData::Ptr getTextData() {
        return textData;
    }
    
    TextStyle::Ptr getDefaultTextStyle() const {
        return defaultTextStyle;
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
    FreePos getCursorFreePos() const {
        return FreePos(getCursorTextPosition(), cursorColumnsBehindEndOfLine);
    }
    long getCursorLineNumber() const {
        return textData->getLineNumberOfMark(cursorMarkId);
    }
    long getCursorWCharColumn() const {
        return textData->getWCharColumnNumberOfMark(cursorMarkId) + cursorColumnsBehindEndOfLine;
    }
    long getCursorByteColumn() const {
        return textData->getByteColumnNumberOfMark(cursorMarkId) + cursorColumnsBehindEndOfLine;
    }
    long getOpticalCursorColumn() const;

    long getOpticalColumn(long pos) const;
    
    int getNumberOfVisibleLines() const {
        return visibleLines;
    }
    
    long getCursorPixX();
    long getLeftPix() const {
        return leftPix;
    }
    long getRightPix() const {
        return leftPix + getWidth();
    }
    long getPixWidth() const {
        return getWidth();
    }
    unsigned int getHeightPix() const {
        return getHeight();
    }
    int getLineHeight() const {
        return lineHeight;
    }
    int getSpaceCharWidth() const {
        return defaultTextStyle->getFontInfo()->getSpaceWidth();
    }
    long getMaximalVisiblePixWidth() const {
        return totalPixWidth;
    }
    
    TextData::TextMark createNewMarkFromCursor();

    void moveCursorToTextPosition(long pos) {
        moveCursorToFreePos(FreePos(pos, 0));
    }
    void moveCursorToFreePos(FreePos freePos);
    void moveCursorToTextMark(TextData::MarkHandle m);
    void moveCursorToWCharColumn(long newColumn);
    void moveCursorRelativeWCharColumns(long columns);
    
    long getTopLineNumber() const {
        return textData->getLineNumberOfMark(topMarkId);
    }
    long getNumberOfLines() const {
        if (    lastEmptyLineStrategy == DO_NOT_IGNORE_EMPTY_LAST_LINE
            || !textData->isBeginOfLine(textData->getLength()))
        {
            return textData->getNumberOfLines();
        } else {
            return textData->getNumberOfLines() - 1;
        }
    }

    void setTopLineNumber(long n);
    void setLeftPix(long leftPix);

    FreePos getFreePosFromPixXY(int pixX, int pixY, bool optimizeForThinCursor = true);
    long getTextPosFromPixXY(int pixX, int pixY, bool optimizeForThinCursor = true) {
        return getFreePosFromPixXY(pixX, pixY, optimizeForThinCursor).pos;
    }

    FreePos getFreePosForPixX(long pixX, long beginOfLinePos);
    long getTextPosForPixX(long pixX, long beginOfLinePos) {
        return getFreePosForPixX(pixX, beginOfLinePos).pos;
    }

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
    
    bool isCursorBlinking() const {
        return cursorIsBlinking;
    }    
    void startCursorBlinking();
    void stopCursorBlinking();
    void setCursorActive();
    void setCursorInactive();
    void hideCursor();
    void showCursor();

    void registerCursorPositionDataListener(Callback<CursorPositionData>::Ptr listener);

    void treatNotificationOfHotKeyEventForOtherWidget();
        
protected:
    virtual void processGuiWidgetCreatedEvent();

protected: // GuiWidget::EventListener interface implementation
    virtual GuiWidget::ProcessingResult processGuiWidgetEvent(const XEvent* event);
    virtual void                        processGuiWidgetNewPositionEvent(const Position& newPosition);

    virtual Measures internalGetDesiredMeasures();

private:
    
    GuiColor getColorForBackground(byte background);
    
    void internalShowMousePointer();
    void internalHideMousePointer();
    
    void unclip();
    void clip(int x, int y, int w, int h);
    void calcTotalPixWidth();
    long calcLongestVisiblePixWidth();
    void fillLineInfo(long beginOfLinePos, RawPtr<LineInfo> li);
    RawPtr<LineInfo> getValidLineInfo(long line);
    void redrawChanged(long spos, long epos);
    void redraw();
    void drawPartialArea(int minY, int maxY, int x1, int x2);
    void drawArea(int minY, int maxY);
    int  calcVisiblePixXForPosInLine(RawPtr<LineInfo> li, FreePos freePos);
    void printPartialLine(RawPtr<LineInfo> li, int y, int x1, int x2);
    void printPartialLine(RawPtr<LineInfo> li, int y, int x1, int x2, RawPtr<LineInfo> prevLi, RawPtr<LineInfo> nextLi);
    void printPartialLineWithoutCursor(RawPtr<LineInfo> li, int y, int x1, int x2);
    void drawCursorInPartialLine(RawPtr<LineInfo> li, int y, int x1, int x2);
    void printLine(RawPtr<LineInfo> li, int y);
    void printLine(RawPtr<LineInfo> li, int y, RawPtr<LineInfo> prevLi);
    void printChangedPartOfLine(RawPtr<LineInfo> newLi, int y, RawPtr<LineInfo> oldLi, RawPtr<LineInfo> prevLi, RawPtr<LineInfo> nextLi);
    void clearLine(RawPtr<LineInfo> li, int y);
    void clearPartialLine(RawPtr<LineInfo> li, int y, int x1, int x2);
    void applyTextStyle(int styleIndex);
    void internSetLeftPix(long leftPix);

    Callback<>::Ptr cursorBlinkCallback;
    void blinkCursor();

    void treatTextDataUpdate(TextData::UpdateInfo update);
    void treatTextStylesChanged(const ObjectArray<TextStyle::Ptr>& newTextStyles);

    void flushPendingUpdates();
    
    void treatHilitingUpdate(HilitingBuffer::UpdateInfo update);
    
    void drawCursor(long cursorPos);
    
    void processAllExposureEvents();
    
    long fillCursorColumnsBehindEndOfLine();
    
    TimeVal cursorNextBlinkTime;
    
    bool hasPosition;
    HilitedText::Ptr hilitedText;
    TextData::Ptr    textData;

    TextStyle::Ptr                   defaultTextStyle;
    ObjectArray< TextStyle::Ptr>     textStyles;
    ObjectArray< RawPtr<TextStyle> > rawTextStylePtrs;
    
    HilitingBuffer::Ptr hilitingBuffer;
    BackliteBuffer::Ptr backliteBuffer;

    TextData::TextMark topMarkId; // first column of the first displayed textline
    TextData::TextMark cursorMarkId;
    long cursorColumnsBehindEndOfLine;
    long lastDrawnCursorPixX;

    long visibleLines;
    int lineHeight;
    int lineAscent;
    int lineDescent;
    LineInfos lineInfos;
    Callback<long,long,long>::Ptr scrollBarVerticalValueRangeChangedCallback;
    Callback<long,long,long>::Ptr scrollBarHorizontalValueRangeChangedCallback;
    long totalPixWidth;
    long leftPix;
    long endPos;
    bool cursorIsBlinking;
    bool cursorIsActive;
    
    LineInfo tempLineInfo;
    LineInfo tempLineInfo2;

    bool updateVerticalScrollBar;
    bool updateHorizontalScrollBar;

    Region redrawRegion; // collects Rectangles for redraw events
    
    CallbackContainer<CursorPositionData> lineAndColumnListeners;
    long                         lastLineOfLineAndColumnListeners;
    long                       lastColumnOfLineAndColumnListeners;
    long                          lastPosOfLineAndColumnListeners;
    long                     lastLengthOfSelectionLengthListeners;
    
    int minWidthChars;
    int minHeightChars;
    int bestWidthChars;
    int bestHeightChars;
    int maxWidthChars;
    int maxHeightChars;
    int border;
    VerticalAdjustmentStrategy verticalAdjustmentStrategy;
    HorizontalAdjustmentStrategy horizontalAdjustmentStrategy;
    LastEmptyLineStrategy lastEmptyLineStrategy;
    bool isMousePointerHidden;
    GuiColor primarySelectionColor;
    GuiColor secondarySelectionColor;
    GuiColor backgroundColor;
    GC textWidget_gcid;

    class CursorVisibleFlag
    {
    public:
        CursorVisibleFlag(CreateOptions options)
            : cursorVisibleFlag(false),
              neverShowCursorFlag(options.isSet(NEVER_SHOW_CURSOR))
        {}
        
        operator bool() const {
            return cursorVisibleFlag;
        }
        
        CursorVisibleFlag& operator=(bool newValue) {
            if (!neverShowCursorFlag) {
                cursorVisibleFlag = newValue;
            }
            return *this;
        }
    private:
        bool cursorVisibleFlag;
        bool neverShowCursorFlag;
    };

    CursorVisibleFlag cursorVisible;
    bool neverShowCursorFlag;
    
    bool exposureNeedsSync;
};

} // namespace LucED

#endif // TEXT_WIDGET_HPP

