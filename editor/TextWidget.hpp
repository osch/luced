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

namespace LucED {

class TextWidget : public GuiWidget
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
    TextWidget(GuiWidget* parent, HilitedText::Ptr hilitedText, int border,
               CreateOptions options);

    void setScrollBarVerticalValueRangeChangedCallback(Callback<long,long,long>::Ptr callback) {
        scrollBarVerticalValueRangeChangedCallback = callback;
        scrollBarVerticalValueRangeChangedCallback->call(getNumberOfLines(), visibleLines, getTopLineNumber());
    }
    void setScrollBarHorizontalValueRangeChangedCallback(Callback<long,long,long>::Ptr callback) {
        scrollBarHorizontalValueRangeChangedCallback = callback;
        scrollBarHorizontalValueRangeChangedCallback->call(totalPixWidth, position.w, leftPix);
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
    virtual void setPosition(Position newPosition);
    
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
    
    virtual Measures getDesiredMeasures();
    
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
    long getCursorLineNumber() const {
        return textData->getLineNumberOfMark(cursorMarkId);
    }
    long getCursorColumn() const {
        return textData->getColumnNumberOfMark(cursorMarkId);
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
        return defaultTextStyle->getSpaceWidth();
    }
    long getMaximalVisiblePixWidth() const {
        return totalPixWidth;
    }
    
    TextData::TextMark createNewMarkFromCursor();

    void moveCursorToTextPosition(long pos);
    void moveCursorToTextMark(TextData::MarkHandle m);
    
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

    void registerCursorPositionDataListener(Callback<CursorPositionData>::Ptr listener);

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

    Callback<>::Ptr cursorBlinkCallback;
    void blinkCursor();

    void treatTextDataUpdate(TextData::UpdateInfo update);
    void treatTextStylesChanged();
    void treatSyntaxPatternsChanged(SyntaxPatterns::Ptr newSyntaxPatterns);

    void flushPendingUpdates();
    
    void treatHilitingUpdate(HilitingBuffer::UpdateInfo update);
    
    void drawCursor(long cursorPos);
    
    void processAllExposureEvents();
    
    void treatConfigUpdate();
    
    TimeVal cursorNextBlinkTime;
    
    Position position;
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
    long opticalCursorColumn;

    long visibleLines;
    int lineHeight;
    int lineAscent;
    LineInfos lineInfos;
    Callback<long,long,long>::Ptr scrollBarVerticalValueRangeChangedCallback;
    Callback<long,long,long>::Ptr scrollBarHorizontalValueRangeChangedCallback;
    long totalPixWidth;
    long leftPix;
    long endPos;
    bool cursorIsBlinking;
    bool cursorIsActive;
    
    LineInfo tempLineInfo;

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

