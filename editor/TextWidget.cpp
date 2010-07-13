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

#include <X11/cursorfont.h>

#include "TextWidget.hpp"
#include "util.hpp"
#include "TimeVal.hpp"
#include "EventDispatcher.hpp"
#include "GlobalConfig.hpp"
#include "RawPtr.hpp"

#define CURSOR_WIDTH 2

static const int TAB_CHARACTER = 0x09;

using namespace LucED;

static inline unsigned int calculateWidthOrHeightWithoutBorder(unsigned int totalWidth, int border)
{
    unsigned int rslt = totalWidth - 2 * border;
    if (rslt <= 0) rslt = 2 * border;
    return rslt;
}

namespace LucED
{

class TextWidgetSingletonData : public HeapObject
{
public:
    static TextWidgetSingletonData* getInstance() { return instance.getPtr(); }

    Cursor getEmptyMouseCursor() { return emptyMouseCursor; }
    Cursor getTextMouseCursor()  { return textMouseCursor; }
    GC     getGcId()             { return textWidget_gcid; }

private:
    friend class SingletonInstance<TextWidgetSingletonData>;
    
    TextWidgetSingletonData()
    {
        static const char emptyPixmapBytes[] = {0x00, 0x00, 0x00, 0x00};

        Pixmap emptyPixmap = XCreateBitmapFromData(GuiRoot::getInstance()->getDisplay(),
                                                   GuiRoot::getInstance()->getRootWid(), 
                                                   emptyPixmapBytes, 1, 1);

        XColor dummyForegoundColor  = {1, 0, 0, 0, DoRed|DoGreen|DoBlue, 0};
        XColor dummyBackgroundColor = {0, 0, 0, 0, DoRed|DoGreen|DoBlue, 0};
        
        emptyMouseCursor = XCreatePixmapCursor(GuiRoot::getInstance()->getDisplay(), emptyPixmap, emptyPixmap, 
                                               &dummyForegoundColor, &dummyBackgroundColor, 0, 0);

        XFreePixmap(GuiRoot::getInstance()->getDisplay(), emptyPixmap);

        textMouseCursor = XCreateFontCursor(GuiRoot::getInstance()->getDisplay(), XC_xterm);

        textWidget_gcid = XCreateGC(GuiRoot::getInstance()->getDisplay(), 
                                    GuiRoot::getInstance()->getRootWid(), 0, NULL);
                                    
        XSetGraphicsExposures(GuiRoot::getInstance()->getDisplay(), textWidget_gcid, True);
    }
    
    ~TextWidgetSingletonData()
    {
        Display* display = GuiRoot::getInstance()->getDisplay();
        XFreeCursor(display, emptyMouseCursor);
        XFreeCursor(display, textMouseCursor);
        XFreeGC    (display, textWidget_gcid);
    }
    
    static SingletonInstance<TextWidgetSingletonData> instance;

    Cursor emptyMouseCursor;
    Cursor textMouseCursor;
    GC textWidget_gcid;
};

} // namespace LucED


using namespace LucED;

SingletonInstance<TextWidgetSingletonData> TextWidgetSingletonData::instance;


/**
 * TextWidget-Constructor.
 */
TextWidget::TextWidget(HilitedText::Ptr hilitedText, int border,
                       CreateOptions options)

    : FocusableWidget(VISIBLE, border),

      totalPixWidth(0),
      leftPix(0),
      endPos(0),
      cursorIsBlinking(false),
      cursorIsActive(false),
      updateVerticalScrollBar(false),
      updateHorizontalScrollBar(false),
      
      hasPosition(false),
      hilitedText(hilitedText),
      textData(hilitedText->getTextData()),
      cursorBlinkCallback(newCallback(this, &TextWidget::blinkCursor)),
      defaultTextStyle(hilitedText->getSyntaxPatterns()->getDefaultTextStyle()),
      textStyles      (hilitedText->getSyntaxPatterns()->getTextStylesArray()),
      rawTextStylePtrs(textStyles),
      hilitingBuffer(HilitingBuffer::create(hilitedText)),
      backliteBuffer(BackliteBuffer::create(textData)),
      lineInfos(),
      topMarkId(textData->createNewMark()),
      cursorMarkId(textData->createNewMark()),
      cursorColumnsBehindEndOfLine(0),
      lastDrawnCursorPixX(0),
      lastLineOfLineAndColumnListeners(0),
      lastColumnOfLineAndColumnListeners(0),
      lastPosOfLineAndColumnListeners(0),
      lastLengthOfSelectionLengthListeners(0),
      minWidthChars(10),
      minHeightChars(1),
      bestWidthChars(80),
      bestHeightChars(25),
      maxWidthChars(INT_MAX),
      maxHeightChars(INT_MAX),
      border(border),
      verticalAdjustmentStrategy(STRICT_TOP_LINE_ANCHOR),
      horizontalAdjustmentStrategy(STRICT_LEFT_COLUMN_ANCHOR),
      lastEmptyLineStrategy(DO_NOT_IGNORE_EMPTY_LAST_LINE),
      isMousePointerHidden(false),
      
      primarySelectionColor(  getGuiRoot()->getGuiColor(GlobalConfig::getConfigData()->getGeneralConfig()->getPrimarySelectionColor())),
      secondarySelectionColor(getGuiRoot()->getGuiColor(GlobalConfig::getConfigData()->getGeneralConfig()->getPseudoSelectionColor())),
      backgroundColor(        getGuiRoot()->getWhiteColor()),
      textWidget_gcid(TextWidgetSingletonData::getInstance()->getGcId()),
      
      cursorVisible(options),
      neverShowCursorFlag(options.isSet(NEVER_SHOW_CURSOR)),
      exposureNeedsSync(false)
{
    lineHeight  = defaultTextStyle->getLineHeight();
    lineAscent  = defaultTextStyle->getLineAscent();
    lineDescent = defaultTextStyle->getLineDescent();

    visibleLines = getHeight() / lineHeight; // not rounded;

    lineInfos.setLength(util::maximum(1, ROUNDED_UP_DIV(getHeight()  - 2*border, lineHeight)));


#if 0
    XSetWindowAttributes at;
    at.background_pixmap = None;
//    at.backing_store = Always;
//    at.save_under = True;
//    at.backing_store = WhenMapped;
//    at.bit_gravity = StaticGravity; // NorthWestGravity; //StaticGravity;
//    XChangeWindowAttributes(getDisplay(), getWid(), 
//            //CWBackingStore|
//            CWBitGravity, &at);

    XChangeWindowAttributes(getDisplay(), getWid(), 
//            CWBackingStore|CWSaveUnder
              CWBackPixmap
            , &at);
#endif
    textData->flushPendingUpdates();
    textData->registerUpdateListener(newCallback(this, &TextWidget::treatTextDataUpdate));
    
    EventDispatcher::getInstance()->registerUpdateSource(newCallback(this, &TextWidget::flushPendingUpdates));

    hilitingBuffer->registerTextStylesChangedListeners (newCallback(this, &TextWidget::treatTextStylesChanged));
    hilitingBuffer->registerUpdateListener             (newCallback(this, &TextWidget::treatHilitingUpdate));
    backliteBuffer->registerUpdateListener             (newCallback(this, &TextWidget::treatHilitingUpdate));
    
    redrawRegion = XCreateRegion();
}

void TextWidget::processGuiWidgetCreatedEvent()
{
    getGuiWidget()->addToXEventMask(ExposureMask);

    setBackgroundColor(backgroundColor);
    
    getGuiWidget()->setBitGravity(NorthWestGravity);
    XDefineCursor(getDisplay(), getGuiWidget()->getWid(), TextWidgetSingletonData::getInstance()->getTextMouseCursor());

    processGuiWidgetNewPositionEvent(getPosition());
}

TextWidget::~TextWidget()
{
    XDestroyRegion(redrawRegion);
}


void TextWidget::treatTextStylesChanged(const ObjectArray<TextStyle::Ptr>& newTextStyles)
{
    lineInfos.setAllInvalid();
    
    defaultTextStyle = newTextStyles[0];
    textStyles       = newTextStyles;
    rawTextStylePtrs = newTextStyles;

    lineHeight  = defaultTextStyle->getLineHeight();
    lineAscent  = defaultTextStyle->getLineAscent();
    lineDescent = defaultTextStyle->getLineDescent();
    
    redrawChanged(getTopLeftTextPosition(), textData->getLength());
}


void TextWidget::registerCursorPositionDataListener(Callback<CursorPositionData>::Ptr listener)
{
    listener->call(CursorPositionData(lastLineOfLineAndColumnListeners, lastColumnOfLineAndColumnListeners,
                                      lastPosOfLineAndColumnListeners, lastLengthOfSelectionLengthListeners));
    lineAndColumnListeners.registerCallback(listener);
}


void TextWidget::treatHilitingUpdate(HilitingBuffer::UpdateInfo update)
{
    ASSERT(update.beginPos <= update.endPos);
#if 0
printf("treatHilitingUpdate %d ... %d\n", update.beginPos, update.endPos);
#endif
    if (update.endPos >= getTopLeftTextPosition() && update.beginPos < this->endPos)
    {
        redrawChanged(update.beginPos, update.endPos);
        totalPixWidth = 0;
        calcTotalPixWidth();
        updateHorizontalScrollBar = true;
    }
}


namespace LucED
{

class TextWidgetFillLineInfoIterator
{
public:
    TextWidgetFillLineInfoIterator(RawPtr<const TextData>                  textData, 
                                   RawPtr<HilitingBuffer>                  hilitingBuffer, 
                                   RawPtr<BackliteBuffer>                  backliteBuffer,
                                   RawPtr<const ObjectArray< RawPtr<TextStyle> > > 
                                                                           textStyles,
                                   RawPtr<TextStyle>                       defaultTextStyle,
                                   long                                    textPos)
        : textData(textData),
          hilitingBuffer(hilitingBuffer),
          backliteBuffer(backliteBuffer),
          textStyles(textStyles),
          defaultTextStyle(defaultTextStyle),
          pixelPos(0),
          textPos(textPos),
          isEndOfLineFlag(textData->isEndOfLine(textPos)),
          c(isEndOfLineFlag ? 0 : textData->getWChar(textPos)),
          styleIndex(hilitingBuffer->getTextStyle(textPos)),
          style((*textStyles)[styleIndex]),
          spaceWidth(defaultTextStyle->getSpaceWidth()),
          tabWidth(hilitingBuffer->getLanguageMode()->getHardTabWidth() * defaultTextStyle->getSpaceWidth()),
          charWidth(isEndOfLineFlag ? 0 : style->getCharWidth(c)),
          doBackgroundFlag(true),
          background(backliteBuffer->getBackground(textPos)),
          numberWChars(0),
          maxCharAscent (isEndOfLineFlag ? 0 : style->getCharAscent(c)),
          maxCharDescent(isEndOfLineFlag ? 0 : style->getCharDescent(c))
    {}
    bool isAtEndOfLine() const { return isEndOfLineFlag; }
    long getTextPos()    const { return textPos; }
    long getPixelPos()   const { return pixelPos; }
    int  getWChar()      const { return c; }
    int getCharWidth()   const { return charWidth; }
    int getTabWidth()    const { return tabWidth; }
    int getSpaceWidth()  const { return spaceWidth; }
    int getBackground()  const { ASSERT(doBackgroundFlag == true); return background; }

    long getNumberOfIteratedWChars() const { return numberWChars; }

    void setDoBackground(bool flag)
    {
        doBackgroundFlag = flag; 
        if (flag) {
            background = backliteBuffer->getBackground(textPos);
        }
    }

    int getCharRBearing() const
    { 
        if (c != TAB_CHARACTER) {
            return style->getCharRBearing(c);
        } else {
            return (((pixelPos / tabWidth) + 1) * tabWidth) - pixelPos;
        }
    }
    int getCharLBearing()    const { return style->getCharLBearing(c); }
    int getMaxCharAscent()   const { return maxCharAscent;  }
    int getMaxCharDescent()  const { return maxCharDescent; }
    
    int getStyleIndex()  const { return styleIndex; }
    RawPtr<TextStyle> getStyle() const { return style; }
    
    void increment()
    {
        ASSERT(isEndOfLineFlag == false);

        if (c== TAB_CHARACTER) {
            pixelPos  = ((pixelPos / tabWidth) + 1) * tabWidth;
        } else {
            pixelPos += charWidth;
        }
        textPos         = textData->getNextWCharPos(textPos); 
        numberWChars   += 1;
        
        if (!textData->isEndOfLine(textPos)) {
            c          = textData->getWChar(textPos);
            styleIndex = hilitingBuffer->getTextStyle(textPos);
            style      = (*textStyles)[styleIndex];
            charWidth  = style->getCharWidth(c);

            util::maximize(&maxCharAscent,  style->getCharAscent(c));
            util::maximize(&maxCharDescent, style->getCharDescent(c));
        } 
        else {
            isEndOfLineFlag = true;
            c          = 0;
            styleIndex = hilitingBuffer->getTextStyle(textPos);
            style      = (*textStyles)[styleIndex];
            charWidth  = 0;
        }
        if (doBackgroundFlag == true) {
            background = backliteBuffer->getBackground(textPos);
        }
    }

private:
    RawPtr<const TextData>             const textData;
    RawPtr<HilitingBuffer>             const hilitingBuffer;
    RawPtr<BackliteBuffer>             const backliteBuffer;
    RawPtr<TextStyle>                  const defaultTextStyle;
    RawPtr<const ObjectArray< RawPtr<TextStyle> > >  textStyles;
    long pixelPos;
    long textPos;
    bool isEndOfLineFlag;
    int  c;
    int  styleIndex;
    RawPtr<TextStyle> style;
    const int tabWidth;
    const int spaceWidth;
    int charWidth;
    bool doBackgroundFlag;
    int background;
    long numberWChars;
    int maxCharAscent;
    int maxCharDescent;
};

class TextWidgetFragmentFiller
{
public:
    TextWidgetFragmentFiller(RawPtr<MemArray<LineInfo::FragmentInfo> > fragments)
        : fragments(fragments),
          lastStyle(-1),
          lastBackground(-1),
          lastStylePixBegin(-1),
          lastStyleBeginNumberOfIteratedWChars(-1)
    {}

    bool hasStyleChanged(const TextWidgetFillLineInfoIterator& i) const
    {
        return lastStyle      != i.getStyleIndex() 
            || lastBackground != i.getBackground()
            || i.getWChar() == TAB_CHARACTER;
    }

    bool hasRememberedStyle() const
    {
        return lastStyle != -1;
    }

    void completeFragment(const TextWidgetFillLineInfoIterator& i) 
    {
        ASSERT(lastStyle                            != -1);
        ASSERT(lastStyleBeginNumberOfIteratedWChars != -1);
        ASSERT(lastStylePixBegin                    != -1);
        ASSERT(fragments->getLength()               >=  1);

        fragments->getLast().numberWChars = i.getNumberOfIteratedWChars() - lastStyleBeginNumberOfIteratedWChars;
        fragments->getLast().pixWidth     = i.getPixelPos() - lastStylePixBegin;
    }

    void beginNewFragment(const TextWidgetFillLineInfoIterator& i)
    {
        if (i.getWChar() == TAB_CHARACTER) {
            lastStyle = -2;
        } else {
            lastStyle = i.getStyleIndex();
        }
        lastBackground        = i.getBackground();
        lastStylePixBegin     = i.getPixelPos();
        lastStyleBeginNumberOfIteratedWChars = i.getNumberOfIteratedWChars();

        fragments->appendAmount(1);
        fragments->getLast().background = i.getBackground();
        fragments->getLast().styleIndex = i.getStyleIndex();
    #ifdef DEBUG
        fragments->getLast().numberWChars = -1;
        fragments->getLast().pixWidth     = -1;
    #endif
    }

private:
    RawPtr<MemArray<LineInfo::FragmentInfo> > fragments;
    int                                     lastStyle;
    int                                     lastBackground;
    long                                    lastStylePixBegin;
    long                                    lastStyleBeginNumberOfIteratedWChars;
};

} // namespace LucED


void TextWidget::fillLineInfo(long beginOfLinePos, RawPtr<LineInfo> li)
{
    TextWidgetFillLineInfoIterator i(textData, hilitingBuffer, backliteBuffer, &rawTextStylePtrs, defaultTextStyle, beginOfLinePos);
    TextWidgetFragmentFiller       f(&li->fragments);
    
    int  print = 0;
    
    li->beginOfLinePos = beginOfLinePos;
    li->leftPixOffset = 0;
    li->startPos = -1;
    li->endPos = -1;
    li->isEndOfText = false;

    li->fragments.clear();
    li->outBuf.clear();
    li->styles.clear();
    

    if (i.isAtEndOfLine())
    {
        if (this->leftPix <= i.getSpaceWidth()) {
            li->startPos = i.getTextPos();
            li->endPos   = i.getTextPos();
            li->leftPixOffset = this->leftPix;
        }
        li->backgroundToEnd = i.getBackground();
        li->spaceWidthAtEnd = i.getStyle()->getSpaceWidth();
        li->charRBearingAtEnd = 0;
    }
    else
    {
        i.setDoBackground(false);
        do
        {
            ASSERT(print == 0);
            
            if (i.getPixelPos() + i.getCharRBearing() >= this->leftPix) {
                print = 1;
                break;
            }
            i.increment();
        }
        while (!i.isAtEndOfLine());

        i.setDoBackground(true);
        const long startPosNumberOfIteratedWChars = i.getNumberOfIteratedWChars();
        
        if (print == 1)
        {
            li->leftPixOffset = leftPix - i.getPixelPos();
            li->startPos      = i.getTextPos();

            f.beginNewFragment(i);

            i.increment();
        }

        while (!i.isAtEndOfLine())
        {
            ASSERT(print == 1);
            
            if (i.getPixelPos() + i.getCharLBearing() >= this->leftPix + this->getWidth())
            {
                print = 2;
                break;
            }
            if (f.hasStyleChanged(i))
            {
                f.completeFragment(i);
                f.beginNewFragment(i);
            }
            i.increment();
        }

        if (print != 0)
        {
            f.completeFragment(i);

            li->endPos = i.getTextPos();

            // long numberWChars = i.getTextPos() - li->startPos;

            const long numberWChars = i.getNumberOfIteratedWChars() - startPosNumberOfIteratedWChars;
            
            {
                Char2b* outPtr = li->outBuf.appendAmount(numberWChars);
                long p    = li->startPos;
                long pend = i.getTextPos();

                while (p < pend) {
                    *(outPtr++) = textData->getWCharAndIncrementPos(&p);
                }
            }

            byte* stylesPtr = hilitingBuffer->getTextStyles(li->startPos, numberWChars);
            if (stylesPtr != NULL) {
                memcpy(li->styles.appendAmount(numberWChars), 
                       stylesPtr,
                       numberWChars);
            } else {
                memset(li->styles.appendAmount(numberWChars), 0, numberWChars);
            }
        }

        if (print == 2)
        {
            i.increment();

            i.setDoBackground(false);
            while (!i.isAtEndOfLine())
            {
                i.increment();
            }
            i.setDoBackground(true);
        }

        li->backgroundToEnd   = i.getBackground();
        li->spaceWidthAtEnd   = i.getStyle()->getSpaceWidth();
        li->charRBearingAtEnd = i.getCharRBearing();
    }

    li->valid = true;
    li->endOfLinePos = i.getTextPos();
    li->totalPixWidth = i.getPixelPos();
    li->pixWidth = i.getPixelPos() - (this->leftPix - li->leftPixOffset);
    li->isEndOfText = (i.getTextPos() == textData->getLength());
    li->maxCharAscent  = util::maximum(lineAscent,  i.getMaxCharAscent());
    li->maxCharDescent = util::maximum(lineDescent, i.getMaxCharDescent());
    
    ASSERT((print == 0 && (   (li->startPos == -1 && li->endPos == -1)
                           || (li->startPos >=  0 && li->endPos >= li->startPos)
                          )
           )
        || (print != 0 && li->startPos >=  0 && li->endPos >=  li->startPos));
        
}

inline void TextWidget::applyTextStyle(int styleIndex)
{
    const RawPtr<TextStyle> style = textStyles[styleIndex];
    
//    XSetBackground(getDisplay(), textWidget_gcid, getGuiRoot()->getWhiteColor());

    XSetForeground(getDisplay(), textWidget_gcid, style->getColor());
    XSetFont(      getDisplay(), textWidget_gcid, style->getFontHandle());
}

inline int TextWidget::calcVisiblePixXForPosInLine(RawPtr<LineInfo> li, FreePos freePos)
{
    long pos          = freePos.pos;
    long extraColumns = freePos.extraColumns;
    
    ASSERT(extraColumns == 0 || textData->isEndOfLine(pos));
    ASSERT(li->valid);
    ASSERT(li->isPosInLine(pos));

    int x;

    if (li->isPosInLineCache(pos))
    {
        x = -li->leftPixOffset;
        long p = li->startPos;
        for (int i = 0; p < pos; ++i) {
            int c = textData->getWChar(p);
            int style = li->styles[i];
            if (c == TAB_CHARACTER) {
                int tabWidth = hilitingBuffer->getLanguageMode()->getHardTabWidth() * defaultTextStyle->getSpaceWidth();
                long totalX = leftPix + x;
                totalX = (totalX / tabWidth + 1) * tabWidth;
                x = totalX - leftPix;
            } else {
                x += textStyles[style]->getCharWidth(c);
            }
            p = textData->getNextWCharPos(p);
        }
    }
    else if (pos == li->endOfLinePos)
    {
        x = li->totalPixWidth - leftPix;
    }
    else
    {
        x = -leftPix;
        for (long p = li->beginOfLinePos; p < pos; ) {
            int c = textData->getWChar(p);
            if (c == TAB_CHARACTER) {
                int tabWidth = hilitingBuffer->getLanguageMode()->getHardTabWidth() * defaultTextStyle->getSpaceWidth();
                x = (x / tabWidth + 1) * tabWidth;
            } else {
                int style = hilitingBuffer->getTextStyle(p);
                x += textStyles[style]->getCharWidth(c);
            }
            p = textData->getNextWCharPos(p);
        }
    }
    if (extraColumns > 0) {
        x += extraColumns * li->spaceWidthAtEnd;
    }
    return x;
}


inline GuiColor TextWidget::getColorForBackground(byte background)
{
    switch (background)
    {
        case 0:  return backgroundColor;
        case 1:  return primarySelectionColor;
        case 2:  return secondarySelectionColor;
        default: ASSERT(false);
                 return backgroundColor;
    }
}


inline void TextWidget::clearLine(RawPtr<LineInfo> li, int y)
{
/*{
    long x = 0;
    
    XSetForeground(getDisplay(), textWidget_gcid, getGuiRoot()->getWhiteColor());
    XFillRectangle(getDisplay(), getWid(), textWidget_gcid, 
            x, y, getWidth() - x, lineHeight);
}*/
///////////////////


}



inline void TextWidget::clearPartialLine(RawPtr<LineInfo> li, int y, int x1, int x2)
{
    MemArray<Char2b>&                         buf       = li->outBuf;
    RawPtr<MemArray<LineInfo::FragmentInfo> > fragments = &li->fragments;
    Char2b* ptr;
    Char2b* end;
    long x = -li->leftPixOffset;;
    long accX = -li->leftPixOffset;;
    int  accBackground = -1;
    
    if (buf.getLength() > 0) {

        ptr = buf.getPtr(0);
        end = ptr + buf.getLength();

        for (int i = 0, n = fragments->getLength(); i < n; ++i)
        {
            ASSERT(ptr < end);
            
            int background = (*fragments)[i].background;
            int styleIndex = (*fragments)[i].styleIndex;
            int len        = (*fragments)[i].numberWChars;
            int pixWidth   = (*fragments)[i].pixWidth;

            if (background != accBackground) {
                if (accBackground != -1) {
                    XSetForeground(getDisplay(), textWidget_gcid, getColorForBackground(accBackground));
                    int useX1 = accX;
                    int useX2 = x;
                    if (useX1 < x2 && useX2 >= x1) {
                        if (getGuiWidget().isValid()) {
                            XFillRectangle(getDisplay(), getGuiWidget()->getWid(), textWidget_gcid, 
                                    useX1, y, useX2 - useX1, lineHeight);
                        }
                    }
                }
                accBackground = background;
                accX = x;
            }
            ptr += len;
            x += pixWidth;
        }
    }
    if (accBackground != -1) {
        XSetForeground(getDisplay(), textWidget_gcid, getColorForBackground(accBackground));
        if (accBackground == li->backgroundToEnd) {
            int useX1 = accX;
            int useX2 = getWidth();
            if (useX1 < x2 && useX2 >= x1) {
                if (getGuiWidget().isValid()) {
                    XFillRectangle(getDisplay(), getGuiWidget()->getWid(), textWidget_gcid, 
                            useX1, y, useX2 - useX1, lineHeight);
                }
            }
        } else {
            int useX1 = accX;
            int useX2 = x;
            if (useX1 < x2 && useX2 >= x1) {
                if (getGuiWidget().isValid()) {
                    XFillRectangle(getDisplay(), getGuiWidget()->getWid(), textWidget_gcid, 
                            useX1, y, useX2 - useX1, lineHeight);
                }
            }
            XSetForeground(getDisplay(), textWidget_gcid, getColorForBackground(li->backgroundToEnd));
            useX1 = x;
            useX2 = getWidth();
            if (useX1 < x2 && useX2 >= x1) {
                if (getGuiWidget().isValid()) {
                    XFillRectangle(getDisplay(), getGuiWidget()->getWid(), textWidget_gcid, 
                            useX1, y, useX2 - useX1, lineHeight);
                }
            }
        }
    } else {
        XSetForeground(getDisplay(), textWidget_gcid, getColorForBackground(li->backgroundToEnd));
        int useX1 = x;
        int useX2 = getWidth();
        if (useX1 < x2 && useX2 >= x1) {
            if (getGuiWidget().isValid()) {
                XFillRectangle(getDisplay(), getGuiWidget()->getWid(), textWidget_gcid, 
                        useX1, y, useX2 - useX1, lineHeight);
            }
        }
    }

}

inline void TextWidget::printPartialLineWithoutCursor(RawPtr<LineInfo> li, int y, int x1, int x2)
{
    MemArray<Char2b>&                 buf       = li->outBuf;
    MemArray<LineInfo::FragmentInfo>& fragments = li->fragments;
    Char2b* ptr;
    Char2b* end;
    int x = -li->leftPixOffset;
    
    if (buf.getLength() > 0) {

        ptr = buf.getPtr(0);
        end = ptr + buf.getLength();

        for (int i = 0, n = fragments.getLength(); i < n; ++i)
        {
            ASSERT(ptr < end);
            
            int background = fragments[i].background;
            int styleIndex = fragments[i].styleIndex;
            int len        = fragments[i].numberWChars;
            int pixWidth   = fragments[i].pixWidth;

            const RawPtr<TextStyle> style = textStyles[styleIndex];
            
            if (x + style->getCharLBearing(*ptr) >= x2) {
                break;
            }
            
            if (len > 0 && *ptr != TAB_CHARACTER
                    && x + pixWidth 
                         - style->getCharWidth(ptr[len - 1])
                         + style->getCharRBearing(ptr[len - 1]) >= x1) {
                Char2b* ptrpend = ptr + len;
                Char2b* ptrp = ptr;
                Char2b* ptrp1;
                Char2b* ptrp2;
                int xp = x;
                int xp1;
                
                while (xp + style->getCharRBearing(*ptrp) < x1) {
                    xp   += style->getCharWidth(*ptrp);
                    ptrp += 1;
                }
                xp1 = xp;
                ptrp1 = ptrp;
                do {
                    xp   += style->getCharWidth(*ptrp);
                    ptrp += 1;
                } while (ptrp < ptrpend && xp + style->getCharLBearing(*ptrp) < x2);
                ptrp2 = ptrp;
                
                applyTextStyle(styleIndex);
                if (getGuiWidget().isValid()) {
                    XDrawString16(getDisplay(), getGuiWidget()->getWid(), 
                                  textWidget_gcid, xp1, y + lineAscent, ptrp1, ptrp2 - ptrp1);
                }
                //XDrawString(getDisplay(), tw->wid, 
                //        tw->gcid, x, y + tw->lineAscent, ptr, len);
                //printf("print <%.*s> \n", len, ptr);
            }
            ptr += len;
            x += pixWidth;
        }
    }
}

inline void TextWidget::drawCursorInPartialLine(RawPtr<LineInfo> li, int y, int x1, int x2)
{
    long cursorPos = getCursorTextPosition();

    if (cursorVisible && (li->isPosInLine(cursorPos)))
    {
        // cursor visible in this line
    
        int cursorX = calcVisiblePixXForPosInLine(li, FreePos(cursorPos, cursorColumnsBehindEndOfLine));
        
        if (cursorX < x2 && x1 <= cursorX + CURSOR_WIDTH - 1) {
        
            // cursor visible in this part
            
            if (cursorIsActive) {
                XSetForeground(getDisplay(), textWidget_gcid, getGuiRoot()->getBlackColor());
            } else {
                XSetForeground(getDisplay(), textWidget_gcid, getGuiRoot()->getGreyColor());
            }
            if (getGuiWidget().isValid()) {
                XFillRectangle(getDisplay(), getGuiWidget()->getWid(), textWidget_gcid, 
                        cursorX, y, CURSOR_WIDTH, lineHeight);
                lastDrawnCursorPixX = cursorX;
            }
        }
        
    }
}

inline void TextWidget::printPartialLine(RawPtr<LineInfo> li, int y, int x1, int x2)
{
    long cursorPos = getCursorTextPosition();
    
    clearPartialLine             (li, y, x1, x2);
    drawCursorInPartialLine      (li, y, x1, x2);
    printPartialLineWithoutCursor(li, y, x1, x2);
}


inline void TextWidget::printLine(RawPtr<LineInfo> li, int y)
{
    long cursorPos      = getCursorTextPosition();
    bool considerCursor = false;
    int  cursorX;
    
    if (cursorVisible && (li->isPosInLine(cursorPos))) {
        // cursor visible in this line
        considerCursor = true;
        cursorX = calcVisiblePixXForPosInLine(li, FreePos(cursorPos, cursorColumnsBehindEndOfLine));
    }

    MemArray<Char2b>&                 buf       = li->outBuf;
    MemArray<LineInfo::FragmentInfo>& fragments = li->fragments;
    Char2b* ptr;
    Char2b* end;
    long x = 0;
    long lastBackgroundX = 0;
    int leftPixOffset = li->leftPixOffset;
    
    if (buf.getLength() > 0)
    {
        ptr = buf.getPtr(0);
        end = ptr + buf.getLength();

        for (int i = 0, n = fragments.getLength(); i < n; ++i)
        {
            ASSERT(ptr < end);
            
            int background = fragments[i].background;
            int styleIndex = fragments[i].styleIndex;
            int len        = fragments[i].numberWChars;
            int pixWidth   = fragments[i].pixWidth;

            int tx1 = -leftPixOffset + x;
            int tx2 = tx1 + pixWidth;

            int bx1 = -leftPixOffset + lastBackgroundX;
            int bx2 = tx2;

            XSetForeground(getDisplay(), textWidget_gcid, getColorForBackground(background));
            if (getGuiWidget().isValid()) {
                XFillRectangle(getDisplay(), getGuiWidget()->getWid(), textWidget_gcid, 
                        bx1, y, bx2 - bx1, lineHeight);
            }
            lastBackgroundX = x + pixWidth;
            
            if (len > 0 && *ptr != TAB_CHARACTER ) 
            {
                const RawPtr<TextStyle> style = textStyles[styleIndex];
                int txCorrection = - style->getCharWidth(ptr[len - 1]) + style->getCharRBearing(ptr[len - 1]);
                
                if (txCorrection > 0)
                {
                    if (i + 1 < n) {
                        XSetForeground(getDisplay(), textWidget_gcid, getColorForBackground(fragments[i + 1].background));
                    } else {
                        XSetForeground(getDisplay(), textWidget_gcid, getColorForBackground(li->backgroundToEnd));
                    }
                    if (getGuiWidget().isValid()) {
                        XFillRectangle(getDisplay(), getGuiWidget()->getWid(), textWidget_gcid, 
                                tx2, y, txCorrection, lineHeight);
                    }
                    lastBackgroundX += txCorrection;
                    bx2 += txCorrection;
                }
            }

            if (considerCursor && cursorX < bx2 && cursorX + CURSOR_WIDTH > bx1) {
                if (cursorIsActive) {
                    XSetForeground(getDisplay(), textWidget_gcid, getGuiRoot()->getBlackColor());
                } else {
                    XSetForeground(getDisplay(), textWidget_gcid, getGuiRoot()->getGreyColor());
                }
                int cx1 = util::maximum(cursorX, bx1);
                int cx2 = util::minimum(cursorX + CURSOR_WIDTH, bx2);
                if (getGuiWidget().isValid()) {
                    XFillRectangle(getDisplay(), getGuiWidget()->getWid(), textWidget_gcid,     // DrawCursor
                            cx1, y, cx2 - cx1, lineHeight);
                    lastDrawnCursorPixX = cursorX;
                }
            }

            if (len > 0 && *ptr != TAB_CHARACTER) {
                applyTextStyle(styleIndex);
                if (getGuiWidget().isValid()) {
                    XDrawString16(getDisplay(), getGuiWidget()->getWid(), 
                                  textWidget_gcid, -leftPixOffset + x, y + lineAscent, ptr, len);
                }
                //printf("print <%.*s> \n", len, ptr);
            }
            ptr += len;
            x += pixWidth;
        }
    }
    {
        int bx1 = -leftPixOffset + lastBackgroundX;
        int bx2 = getWidth();
        XSetForeground(getDisplay(), textWidget_gcid, getColorForBackground(li->backgroundToEnd));
        if (getGuiWidget().isValid()) {
            XFillRectangle(getDisplay(), getGuiWidget()->getWid(), textWidget_gcid, 
                    bx1, y, bx2 - bx1, lineHeight);
        }
        if (considerCursor && cursorX < bx2 && cursorX + CURSOR_WIDTH > bx1) {
            if (cursorIsActive) {
                XSetForeground(getDisplay(), textWidget_gcid, getGuiRoot()->getBlackColor());
            } else {
                XSetForeground(getDisplay(), textWidget_gcid, getGuiRoot()->getGreyColor());
            }
            int cx1 = util::maximum(cursorX, bx1);
            int cx2 = util::minimum(cursorX + CURSOR_WIDTH, bx2);
            if (getGuiWidget().isValid()) {
                XFillRectangle(getDisplay(), getGuiWidget()->getWid(), textWidget_gcid,     // DrawCursor
                        cx1, y, cx2 - cx1, lineHeight);
                lastDrawnCursorPixX = cursorX;
            }
        }
    }
}

inline void TextWidget::printLine(RawPtr<LineInfo> li, int y, RawPtr<LineInfo> prevLi)
{
    printLine(li, y);
    if (prevLi.isValid())
    {
        if (li->maxCharAscent > lineAscent || prevLi->maxCharDescent > lineDescent)
        {
            int deltaY1 = util::maximum(0, li->maxCharAscent  - lineAscent);
            int w       = getWidth();
            
            clip(0, y - deltaY1, w, lineHeight + deltaY1);
            printPartialLineWithoutCursor(prevLi, y - lineHeight, 0, w);
            unclip();
        }
    }
}
void TextWidget::printChangedPartOfLine(RawPtr<LineInfo> newLi, int y, RawPtr<LineInfo> oldLi, RawPtr<LineInfo> prevLi, RawPtr<LineInfo> nextLi)
{
    MemArray<Char2b>& newBuf = newLi->outBuf;
    MemArray<Char2b>& oldBuf = oldLi->outBuf;
    
    long newBufLength = newBuf.getLength();
    long oldBufLength = oldBuf.getLength();
    
    MemArray<LineInfo::FragmentInfo>& newFragments = newLi->fragments;
    MemArray<LineInfo::FragmentInfo>& oldFragments = oldLi->fragments;

    int startX = -1;    
    int endX = -1;
    
    if (oldBufLength > 0 && newBufLength > 0)
    {
        long cursorPos             = getCursorTextPosition();
        bool considerCursorInNewLi = cursorVisible && newLi->isPosInLine(cursorPos);
        bool considerCursorInOldLi = cursorVisible && oldLi->isPosInLine(cursorPos);
        long cursorBufIndexInNewLi = 0;
        long cursorBufIndexInOldLi = 0;
        
        if (considerCursorInNewLi) {
            long p = newLi->startPos;
            while (cursorBufIndexInNewLi + 1 < newBufLength && p < cursorPos) {
                ++cursorBufIndexInNewLi;
                p = textData->getNextWCharPos(p);
            }
        }
        if (considerCursorInOldLi) {
            long p = oldLi->startPos;
            while (cursorBufIndexInOldLi + 1 < oldBufLength && p < cursorPos) {
                ++cursorBufIndexInOldLi;
                p = textData->getNextWCharPos(p);
            }
        }
        if (   util::maximum(newLi->maxCharAscent,
                             oldLi->maxCharAscent) <= lineAscent 
            && prevLi.isValid() 
            && prevLi->maxCharDescent <= lineDescent) 
        {
            prevLi = Null;
        }
        if (   util::maximum(newLi->maxCharDescent,
                             oldLi->maxCharDescent) <= lineDescent 
            && nextLi.isValid() 
            && nextLi->maxCharAscent <= lineAscent)
        {
            nextLi = Null;
        }
    
        bool newEnd = false;
        bool oldEnd = false;
    
        int newFragmentIndex = 0;
        int newFragmentCount = newFragments.getLength();
        
        int oldFragmentIndex = 0;
        int oldFragmentCount = oldFragments.getLength();
        
    
        int newLeftPixOffset = newLi->leftPixOffset;
        int oldLeftPixOffset = oldLi->leftPixOffset;
        
        long newX = -newLeftPixOffset;
        long oldX = -oldLeftPixOffset;
    
        long xMin = -1;
        long xMax = -1;
        
        long newBufIndex = 0;
        long oldBufIndex = 0;
        
        /////
        
        int newFragmentMaxBufIndex = newBufIndex + newFragments[newFragmentIndex].numberWChars;
        int oldFragmentMaxBufIndex = oldBufIndex + oldFragments[oldFragmentIndex].numberWChars;
    
        int newStyleIndex = newFragments[newFragmentIndex].styleIndex;
        int oldStyleIndex = oldFragments[oldFragmentIndex].styleIndex;
    
        int newBackground   = newFragments[newFragmentIndex].background;
        int oldBackground   = oldFragments[oldFragmentIndex].background;
    
        RawPtr<TextStyle> newStyle = textStyles[newStyleIndex];
        RawPtr<TextStyle> oldStyle = textStyles[oldStyleIndex];
    
        int tabWidth = hilitingBuffer->getLanguageMode()->getHardTabWidth() * defaultTextStyle->getSpaceWidth();
        
        do
        {
            Char2b newChar = newBuf[newBufIndex];
            Char2b oldChar = oldBuf[oldBufIndex];
            
            int  newCharLBearing = newStyle->getCharLBearing(newChar);
            int  newCharWidth;
            int  newCharRBearing;
            if (newChar == TAB_CHARACTER) {
                newCharWidth    = (((newX + leftPix) / tabWidth) + 1) * tabWidth - (newX + leftPix);
                newCharRBearing = newCharWidth;
            } else {
                newCharWidth    = newStyle->getCharWidth(newChar);
                newCharRBearing = newStyle->getCharRBearing(newChar);
            }
            
            int  oldCharLBearing = oldStyle->getCharLBearing(oldChar);
            int  oldCharWidth;
            int  oldCharRBearing;
            if (oldChar == TAB_CHARACTER) {
                oldCharWidth    = (((oldX + leftPix) / tabWidth) + 1) * tabWidth - (oldX + leftPix);
                oldCharRBearing = oldCharWidth;
            } else {
                oldCharWidth    = oldStyle->getCharWidth(oldChar);
                oldCharRBearing = oldStyle->getCharRBearing(oldChar);
            }

            bool isAtCursorInNewLiPos = (considerCursorInNewLi && cursorBufIndexInNewLi == newBufIndex);
            bool isAtCursorInOldLiPos = (considerCursorInOldLi && cursorBufIndexInOldLi == oldBufIndex);
            
            if (   newX != oldX
                || newChar != oldChar
                || newStyleIndex != oldStyleIndex
                || newBackground != oldBackground
                || isAtCursorInNewLiPos
                || isAtCursorInOldLiPos)
            {
                if (xMin == -1) {
                    xMin = util::minimum(newX + newCharLBearing, oldX + oldCharLBearing);
                    if (xMin < 0) xMin = 0;
                }
                xMax = util::maximum(newX + newCharRBearing, oldX + oldCharRBearing);
                
                if (isAtCursorInNewLiPos && cursorColumnsBehindEndOfLine > 0) {
                    util::maximize(&xMax, newX + newLi->spaceWidthAtEnd * cursorColumnsBehindEndOfLine + CURSOR_WIDTH);
                }
                if (isAtCursorInOldLiPos && cursorColumnsBehindEndOfLine > 0) {
                    util::maximize(&xMax, oldX + oldLi->spaceWidthAtEnd * cursorColumnsBehindEndOfLine + CURSOR_WIDTH);
                }
            }
    
            
            newBufIndex += 1;
            oldBufIndex += 1;
            
            if (newBufIndex >= newFragmentMaxBufIndex) {
                newFragmentIndex += 1;
                if (newFragmentIndex < newFragmentCount)
                {
                    newFragmentMaxBufIndex = newBufIndex + newFragments[newFragmentIndex].numberWChars;
                    newStyleIndex          = newFragments[newFragmentIndex].styleIndex;
                    newBackground          = newFragments[newFragmentIndex].background;
                    newStyle               = textStyles[newStyleIndex];
                } else {
                    newEnd = true;
                }
            }        
            if (oldBufIndex >= oldFragmentMaxBufIndex) {
                oldFragmentIndex += 1;
                if (oldFragmentIndex < oldFragmentCount)
                {
                    oldFragmentMaxBufIndex = oldBufIndex + oldFragments[oldFragmentIndex].numberWChars;
                    oldStyleIndex          = oldFragments[oldFragmentIndex].styleIndex;
                    oldBackground          = oldFragments[oldFragmentIndex].background;
                    oldStyle               = textStyles[oldStyleIndex];
                } else {
                    oldEnd = true;
                }
            }
            if (newEnd || oldEnd) {
                if (xMin == -1) {
                    xMin = util::minimum(newX + newCharLBearing, oldX + oldCharLBearing);
                    if (xMin < 0) xMin = 0;
                    xMax = getWidth();
                }
            } else {
                newX += newCharWidth;
                oldX += oldCharWidth;
            }
        }
        while (!newEnd && !oldEnd);
    
        if (xMin < 0) xMin = 0;
        startX = xMin;

        if (newEnd && oldEnd && newLi->backgroundToEnd == oldLi->backgroundToEnd) {
            endX = xMax;
        } else {
            if (newLi->backgroundToEnd == oldLi->backgroundToEnd) 
            {
                int newEndX = newLi->pixWidth - newLi->leftPixOffset + newLi->charRBearingAtEnd;
                int oldEndX = oldLi->pixWidth - oldLi->leftPixOffset + oldLi->charRBearingAtEnd;

                if (considerCursorInNewLi && cursorColumnsBehindEndOfLine > 0) {
                    newEndX += newLi->spaceWidthAtEnd * cursorColumnsBehindEndOfLine + CURSOR_WIDTH;
                }
                if (considerCursorInOldLi && cursorColumnsBehindEndOfLine > 0) {
                    oldEndX += oldLi->spaceWidthAtEnd * cursorColumnsBehindEndOfLine + CURSOR_WIDTH;
                }
                endX = util::maximum(newEndX, oldEndX);
            } else {
                endX = getWidth();
            }
        }

        if (considerCursorInNewLi || cursorVisible && oldLi->isPosInLine(cursorPos))
        {
            util::maximize(&endX,   lastDrawnCursorPixX + CURSOR_WIDTH);
            util::minimize(&startX, lastDrawnCursorPixX);
        }
    }
    else {
        startX = 0;
        endX   = getWidth();
    }
    
    ASSERT(startX >= 0); ASSERT(endX >= 0);
    
    int deltaY1 = util::maximum(0, util::maximum(newLi->maxCharAscent  - lineAscent,
                                                 oldLi->maxCharAscent  - lineAscent));
    int deltaY2 = util::maximum(0, util::maximum(newLi->maxCharDescent - lineDescent, 
                                                 oldLi->maxCharDescent - lineDescent));
    
    clip(startX, y - deltaY1, endX - startX, lineHeight + deltaY1 + deltaY2);
    
    if (prevLi.isValid()) clearPartialLine             (prevLi, y - lineHeight, startX, endX);
                          clearPartialLine             ( newLi, y,              startX, endX);
    if (nextLi.isValid()) clearPartialLine             (nextLi, y + lineHeight, startX, endX);

    if (prevLi.isValid()) drawCursorInPartialLine      (prevLi, y - lineHeight, startX, endX);
                          drawCursorInPartialLine      ( newLi, y,              startX, endX);
    if (nextLi.isValid()) drawCursorInPartialLine      (nextLi, y + lineHeight, startX, endX);

    if (prevLi.isValid()) printPartialLineWithoutCursor(prevLi, y - lineHeight, startX, endX);
                          printPartialLineWithoutCursor( newLi, y,              startX, endX);
    if (nextLi.isValid()) printPartialLineWithoutCursor(nextLi, y + lineHeight, startX, endX);

    unclip();
}


inline void TextWidget::printPartialLine(RawPtr<LineInfo> li, int y, int startX, int endX, RawPtr<LineInfo> prevLi, RawPtr<LineInfo> nextLi)
{
    if (li->maxCharAscent <= lineAscent && prevLi.isValid() && prevLi->maxCharDescent <= lineDescent) {
        prevLi = Null;
    }
    if (li->maxCharDescent <= lineDescent && nextLi.isValid() && nextLi->maxCharAscent <= lineAscent) {
        nextLi = Null;
    }

    int deltaY1 = util::maximum(0, li->maxCharAscent  - lineAscent);
    int deltaY2 = util::maximum(0, li->maxCharDescent - lineDescent);
    
    clip(startX, y - deltaY1, endX - startX, lineHeight + deltaY1 + deltaY2);
    
    if (prevLi.isValid()) clearPartialLine             (prevLi, y - lineHeight, startX, endX);
                          clearPartialLine             (    li, y,              startX, endX);
    if (nextLi.isValid()) clearPartialLine             (nextLi, y + lineHeight, startX, endX);

    if (prevLi.isValid()) drawCursorInPartialLine      (prevLi, y - lineHeight, startX, endX);
                          drawCursorInPartialLine      (    li, y,              startX, endX);
    if (nextLi.isValid()) drawCursorInPartialLine      (nextLi, y + lineHeight, startX, endX);

    if (prevLi.isValid()) printPartialLineWithoutCursor(prevLi, y - lineHeight, startX, endX);
                          printPartialLineWithoutCursor(    li, y,              startX, endX);
    if (nextLi.isValid()) printPartialLineWithoutCursor(nextLi, y + lineHeight, startX, endX);

    unclip();
}


void TextWidget::drawPartialArea(int minY, int maxY, int x1, int x2)
{
    int y = 0;
    int line = 0;
    long pos = getTopLeftTextPosition();
    long oldpos;
    RawPtr<LineInfo> prevLi;
    
    do {
        RawPtr<LineInfo> li = lineInfos.getPtr(line);

        if (!li->valid) {
            fillLineInfo(pos, li);
        }
        if (y + lineAscent + li->maxCharDescent > minY && y + lineAscent - li->maxCharAscent < maxY) {
            printPartialLine(li, y, x1, x2, prevLi, Null);
        }
        y    += lineHeight;
        line += 1;
        pos   = li->endOfLinePos;

        oldpos = pos;
        pos  += textData->getLengthOfLineEnding(pos);
    
        prevLi = li;
    }
    while (oldpos != pos && y < getHeight());
    
    if (y < getHeight()) {

//        XClearArea(XGlobal_display_pst, tw->wid, 
//                0, y, 0, 0, False);

        XSetForeground(getDisplay(), textWidget_gcid, backgroundColor);
        if (getGuiWidget().isValid()) {
            XFillRectangle(getDisplay(), getGuiWidget()->getWid(), textWidget_gcid, 
                    0, y, getWidth(), getHeight() - y);
        }
    }
    endPos = pos;
}


void TextWidget::drawArea(int minY, int maxY)
{
    int y = 0;
    int line = 0;
    long pos = getTopLeftTextPosition();
    long oldpos;
    
    RawPtr<LineInfo> prevLi;
    
    if (lineInfos.getLength() > 0) {
        do {
            RawPtr<LineInfo> li = lineInfos.getPtr(line);

            if (!li->valid) {
                fillLineInfo(pos, li);
            }
            if (y + lineAscent + li->maxCharDescent > minY && y + lineAscent - li->maxCharAscent < maxY) {
                printLine(li, y, prevLi);
            }
            y    += lineHeight;
            line += 1;
            pos   = li->endOfLinePos;

            oldpos = pos;
            pos  += textData->getLengthOfLineEnding(pos);

            prevLi = li;
        } 
        while (oldpos != pos && y < getHeight());
    }
    
    if (y < getHeight()) {

//        XClearArea(getDisplay(), tw->wid, 
//                0, y, 0, 0, False);

        XSetForeground(getDisplay(), textWidget_gcid, backgroundColor);
        if (getGuiWidget().isValid()) {
            XFillRectangle(getDisplay(), getGuiWidget()->getWid(), textWidget_gcid, 
                    0, y, getWidth(), getHeight() - y);
        }
    }
    endPos = pos;
}


void TextWidget::redrawChanged(long spos, long epos)
{
    int minY = 0;
    int maxY = getHeight();
    int y = 0;
    int line = 0;
    long pos = getTopLeftTextPosition();
    long oldpos;

    unclip();
    
    if (lineInfos.getLength() > 0)
    {
        RawPtr<LineInfo> prevLi;
        RawPtr<LineInfo> nextLi;
        do {
            RawPtr<LineInfo>     li = lineInfos.getPtr(line);
            
            nextLi = line + 1 < lineInfos.getLength() ? lineInfos.getPtr(line + 1)
                                                      : Null;

            if (li->valid)
            {
                if (li->beginOfLinePos != pos || (li->beginOfLinePos <= epos && li->endOfLinePos >= spos)) // ">=" because line info for zero length lines should also be checked because of selection backliting
                {
                    tempLineInfo = *li;
                    fillLineInfo(pos, li);

                    if (tempLineInfo.isDifferentOnScreenThan(*li)) {
                        if (y + lineAscent + li->maxCharDescent > minY && y + lineAscent - li->maxCharAscent < maxY) {
                            if (nextLi.isValid() && !nextLi->valid) {
                                long npos = li->endOfLinePos;
                                     npos += textData->getLengthOfLineEnding(npos);
                                nextLi = &tempLineInfo2;
                                fillLineInfo(npos, nextLi);
                            }
                            //printLine(li, y);
                            printChangedPartOfLine(li, y, &tempLineInfo, prevLi, nextLi);
                        }
                    }
                    ASSERT(li->beginOfLinePos == pos);
                }
            } else {
                fillLineInfo(pos, li);

                if (y + lineAscent + li->maxCharDescent > minY && y + lineAscent - li->maxCharAscent < maxY) 
                {
                    if (nextLi.isValid() && !nextLi->valid) {
                        long npos = li->endOfLinePos;
                             npos += textData->getLengthOfLineEnding(npos);
                        nextLi = &tempLineInfo2;
                        fillLineInfo(npos, nextLi);
                    }
    
                    printPartialLine(li, y, 0, getWidth(), prevLi, nextLi);
                }
            }
            pos = li->endOfLinePos;
            y    += lineHeight;
            line += 1;

            oldpos = pos;
            pos  += textData->getLengthOfLineEnding(pos);
            
            prevLi = li;
        } 
        while (oldpos != pos && y < maxY && nextLi.isValid());
        
        while (line < lineInfos.getLength()) {
            lineInfos.getPtr(line)->valid = false;
            ++line;
        }
    }
    
    if (y < maxY) {

//        XClearArea(XGlobal_display_pst, tw->wid, 
//                0, y, 0, 0, False);

        XSetForeground(getDisplay(), textWidget_gcid, backgroundColor);
        if (getGuiWidget().isValid()) {
            XFillRectangle(getDisplay(), getGuiWidget()->getWid(), textWidget_gcid, 
                    0, y, getWidth(), getHeight() - y);
        }
    }
    endPos = pos;
}



inline void TextWidget::redraw() {
    drawArea(0, getHeight());
}

RawPtr<LineInfo> TextWidget::getValidLineInfo(long line)
{
    RawPtr<LineInfo> li = lineInfos.getPtr(line);

    if (!li->valid)
    {
        int l = 0;
        long pos = getTopLeftTextPosition();

        do {
            li = lineInfos.getPtr(l);

            if (!li->valid) {
                fillLineInfo(pos, li);
            }
            ASSERT(li->valid);
            l    += 1;
            pos   = li->endOfLinePos;
            pos  += textData->getLengthOfLineEnding(pos);

        } while (l <= line);
    }
    ASSERT(li->valid);
    return li;
}

void TextWidget::drawCursor(long cursorPos)
{
    int y = 0;
    int line = 0;
    RawPtr<LineInfo> li;
    RawPtr<LineInfo> prevLi;

    if (lineInfos.getLength() > 0) {
        do {
            li = getValidLineInfo(line);

            if (cursorPos < li->beginOfLinePos) {
                break;
            }
            if (cursorPos <= li->endOfLinePos) {
                // cursor is visible
                int cursorX = calcVisiblePixXForPosInLine(li, FreePos(cursorPos, cursorColumnsBehindEndOfLine));
                //clip(cursorX - 1, y, CURSOR_WIDTH + 2, lineHeight);
                RawPtr<LineInfo> nextLi = line + 1 < lineInfos.getLength() ? getValidLineInfo(line + 1)
                                                                           : Null;
                printPartialLine(li, y, cursorX - 1, cursorX + CURSOR_WIDTH + 1, prevLi, nextLi);
                //unclip();
                break;
            }
            y    += lineHeight;
            line += 1;

            prevLi = li;
        }
        while (!li->isEndOfText && y < getHeight());
    }
}

long TextWidget::getCursorPixX()
{
    textData->flushPendingUpdates();
    
    long cursorPos = getCursorTextPosition();
    int line = getCursorLineNumber() - getTopLineNumber();
    long lineBegin;
    
    if (0 <= line && line < visibleLines)
    {
        RawPtr<LineInfo> li = getValidLineInfo(line);
    
        return leftPix + calcVisiblePixXForPosInLine(li, FreePos(cursorPos, cursorColumnsBehindEndOfLine));
    }
    else 
    {
        long lineBegin = textData->getThisLineBegin(cursorPos);
        // Fallback if not visible
        long x = 0;
        for (long p = lineBegin; p < cursorPos; ) {
            int c = textData->getWChar(p);
            if (c == TAB_CHARACTER) {
                int tabWidth = hilitingBuffer->getLanguageMode()->getHardTabWidth() * defaultTextStyle->getSpaceWidth();
                x = (x / tabWidth + 1) * tabWidth;
            } else {
                int style = hilitingBuffer->getTextStyle(p);
                x += textStyles[style]->getCharWidth(c);
            }
            p = textData->getNextWCharPos(p);
        }
        if (cursorColumnsBehindEndOfLine > 0) {
            ASSERT(cursorMarkId.isAtEndOfLine());
            int styleAtEndOfLine = hilitingBuffer->getTextStyle(cursorPos);
            x += textStyles[styleAtEndOfLine]->getSpaceWidth() * cursorColumnsBehindEndOfLine;
        }
        return x;
    }
}

long TextWidget::calcLongestVisiblePixWidth()
{
    int y = 0;
    int line = 0;
    long pos = getTopLeftTextPosition();
    long rslt = 0;
    long oldpos;
    
    if (lineInfos.getLength() > 0) {
        long cursorPos = getCursorTextPosition();
        do {
            RawPtr<LineInfo> li = lineInfos.getPtr(line);
            if (!li->valid) {
                fillLineInfo(pos, li);
            }
            long thisWidth = li->totalPixWidth;
            if (li->isPosInLine(cursorPos)) {
                thisWidth += li->spaceWidthAtEnd * cursorColumnsBehindEndOfLine;
            }
            if (thisWidth > rslt)
                rslt = thisWidth;
            pos = li->endOfLinePos;
            y    += lineHeight;
            line += 1;

            oldpos = pos;
            pos  += textData->getLengthOfLineEnding(pos);

        } while (oldpos != pos && y < getHeight());
    }
    rslt += CURSOR_WIDTH;

    return rslt;
}

void TextWidget::calcTotalPixWidth()
{
    long rslt = calcLongestVisiblePixWidth();

    totalPixWidth = 0;
    MAXIMIZE(&totalPixWidth, leftPix + getWidth());
    MAXIMIZE(&totalPixWidth, rslt);
}


void TextWidget::setTopLineNumber(long n)
{
    unclip();
    
    textData->flushPendingUpdates();
    processAllExposureEvents();
    
    calcTotalPixWidth();

    long oldTopLineNumber = getTopLineNumber();
    
    long numberOfLines = getNumberOfLines();
    if ( n > numberOfLines - 1) {
        n = numberOfLines - 1;
    }
    
    if (n < 0) 
        n = 0;
        
    if (n != oldTopLineNumber)
    {
        bool redrawTopLinePart    = false;
        int  redrawTopLineDeltaY = 0;
        
        if (n - 1 >= oldTopLineNumber && n - 1 < oldTopLineNumber + lineInfos.getLength()) {
            RawPtr<LineInfo> prevLi = lineInfos.getPtr((n - 1) - oldTopLineNumber);
            if (prevLi->maxCharDescent > lineDescent) {
                redrawTopLinePart = true;
                redrawTopLineDeltaY = prevLi->maxCharDescent - lineDescent;
            }
        }
        bool redrawBottomLinePart    = false;
        int  redrawBottomLineDeltaY = 0;
        
        if (n + visibleLines >= oldTopLineNumber && n + visibleLines < oldTopLineNumber + lineInfos.getLength()) {
            RawPtr<LineInfo> nextLi = lineInfos.getPtr((n + visibleLines) - oldTopLineNumber);
            if (nextLi->maxCharAscent > lineAscent) {
                redrawBottomLinePart = true;
                redrawBottomLineDeltaY = nextLi->maxCharAscent - lineAscent;
            }
        }
        
        lineInfos.moveFirst(n - oldTopLineNumber);
        textData->moveMarkToLineAndWCharColumn(topMarkId, n, 0);
        totalPixWidth = 0; // reset TextWidget::calcTotalPixWidth
        
        if (n < oldTopLineNumber) {
            if (oldTopLineNumber - n >= visibleLines) {
                redraw();
            } else {
                long diff = oldTopLineNumber - n;
                
                if (getGuiWidget().isValid()) {
                    XCopyArea(getDisplay(), getGuiWidget()->getWid(), getGuiWidget()->getWid(), 
                            textWidget_gcid,
                            0, 0,
                            getWidth(), getHeight() - diff * lineHeight,
                            0, diff * lineHeight);
                }
                exposureNeedsSync = true;
                drawArea(0, diff * lineHeight);
                
                if (redrawBottomLinePart) {
                    drawArea(getHeight() - redrawBottomLineDeltaY, getHeight());
                }
            }
        } else {
            if (n - oldTopLineNumber >= visibleLines) {
                redraw();
            } else {
                long diff = n - oldTopLineNumber;
                
                if (getGuiWidget().isValid()) {
                    XCopyArea(getDisplay(), getGuiWidget()->getWid(), getGuiWidget()->getWid(), 
                            textWidget_gcid,
                            0, diff * lineHeight,
                            getWidth(), getHeight() - diff * lineHeight,
                            0, 0);
                }
                exposureNeedsSync = true;
                drawArea(getHeight() - (diff * lineHeight), getHeight());

                if (redrawTopLinePart) {
                    drawArea(0, redrawTopLineDeltaY);
                }
            }

        }
        updateVerticalScrollBar = true;

        calcTotalPixWidth();
        updateHorizontalScrollBar= true;

        if (!cursorVisible && cursorIsBlinking) {
            startCursorBlinking();
        }
    }
}


TextWidget::FreePos TextWidget::getFreePosForPixX(long pixX, long beginOfLinePos)
{
    long p            = beginOfLinePos;
    long extraColumns = 0;
    
    long x = 0, ox = 0;
    ASSERT(textData->isBeginOfLine(p));
    while (x < pixX && !textData->isEndOfLine(p)) {
        int c = textData->getWChar(p);
        ox = x;
        if (c == TAB_CHARACTER) {
            long tabWidth = hilitingBuffer->getLanguageMode()->getHardTabWidth() * defaultTextStyle->getSpaceWidth();
            x = (x / tabWidth + 1) * tabWidth;
        } else {
            x += textStyles[hilitingBuffer->getTextStyle(p)]->getCharWidth(c);
        }
        p = textData->getNextWCharPos(p);
    }
    if (x >= pixX) {
        if (p >= 1) {
            if (x - pixX > pixX - ox) {
                p = textData->getPrevWCharPos(p);
            }
        }
    } else {
        ASSERT(textData->isEndOfLine(p));
        long spaceWidth = textStyles[hilitingBuffer->getTextStyle(p)]->getSpaceWidth();
        do {
            x += spaceWidth;
            ++extraColumns;
        } while (x < pixX);
        if (x - pixX > pixX - (x - spaceWidth)) {
            --extraColumns;
        }
    }
    return FreePos(p, extraColumns);
}

TextWidget::FreePos TextWidget::getFreePosFromPixXY(int pixX, int pixY, bool optimizeForThinCursor)
{
    textData->flushPendingUpdates();

    int screenLine = pixY / lineHeight;
    if (screenLine >= getNumberOfVisibleLines()) {
        screenLine = getNumberOfVisibleLines() - 1;
    } else if (screenLine < 0) {
        screenLine = 0;
    }
    
    long topLineNumber = getTopLineNumber();
    long pixLine = topLineNumber + screenLine;
    long newCursorPos;

    long numberOfLines = getNumberOfLines();
    if (pixLine >= numberOfLines) {
        pixLine = numberOfLines - 1;
    }
    long totalPixX = pixX + leftPix;
    
    int line = -1;
    long pos;
    long nextPos = getTopLeftTextPosition();

    RawPtr<LineInfo> li;
    
    do {
        line += 1;
        pos = nextPos;
        li = lineInfos.getPtr(line);
        if (!li->valid) {
            fillLineInfo(pos, li);
        }
        nextPos = li->endOfLinePos;
        nextPos  += textData->getLengthOfLineEnding(nextPos);
        
    } while (line < screenLine);
    
    if (li->startPos == -1) {
        return getFreePosForPixX(pixX, li->beginOfLinePos); // FreePos(li->endOfLinePos, 0);
    } else {
        pixX += li->leftPixOffset;
        int i = 0;
        int endI = li->styles.getLength();

        int x, nextX;
        long p = li->startPos;
        for (x = 0, nextX = 0; nextX < pixX && i < endI; ++i) {
            x = nextX;
            const RawPtr<TextStyle> style = textStyles[li->styles[i]];
            int c = textData->getWChar(p);
            if (c == TAB_CHARACTER) {
                long tabWidth = hilitingBuffer->getLanguageMode()->getHardTabWidth() * defaultTextStyle->getSpaceWidth();
                nextX = (x / tabWidth + 1) * tabWidth;
            } else {
                nextX += style->getCharWidth(c);
            }
            p = textData->getNextWCharPos(p);
        }
        
        if (nextX > pixX)
        {
            long rslt;
            if (optimizeForThinCursor) {
                if (i == 0 || nextX - pixX < pixX -x) {
                    rslt = p;
                } else {
                    rslt = textData->getPrevWCharPos(p);
                }
            } else {
                if (i == 0) {
                    rslt = p;
                } else {
                    rslt = textData->getPrevWCharPos(p);
                }
            }
            return FreePos(rslt, 0);
        }
        else {
            int spaceWidth = li->spaceWidthAtEnd;
            long extraColumns = 0;
            while (nextX < pixX) {
                x = nextX;
                nextX += spaceWidth;
                ++extraColumns;
            }
            if (optimizeForThinCursor) {
                if (nextX - pixX > pixX - x) {
                    --extraColumns;
                }
            } else {
                --extraColumns;
            }
            return FreePos(p, extraColumns);
        }
    }
}

void TextWidget::internSetLeftPix(long newLeftPix)
{
    if (newLeftPix < 0) {
        newLeftPix = 0;
    }
    if (newLeftPix == leftPix) {
        return;
    }
    textData->flushPendingUpdates();
    calcTotalPixWidth();
#if 1
    if (newLeftPix > totalPixWidth - getWidth()) {
        newLeftPix = totalPixWidth - getWidth();
    }
#endif
    if (newLeftPix == leftPix) {
        return;
    }
    processAllExposureEvents();
    
    if (leftPix < newLeftPix) {
        long diffPix = newLeftPix - leftPix;
        if (!cursorVisible && cursorIsBlinking) {
            startCursorBlinking();
        }
        if (diffPix < getWidth()) {
            redrawChanged(getTopLeftTextPosition(), textData->getLength()); // assure that new lineinfos match screen content
            if (getGuiWidget().isValid()) {
                XCopyArea(getDisplay(), getGuiWidget()->getWid(), getGuiWidget()->getWid(), textWidget_gcid,
                        diffPix, 0,
                        getWidth() - diffPix, getHeight(),
                        0, 0);
            }
            exposureNeedsSync = true;
            leftPix = newLeftPix;
            lineInfos.setAllInvalid();
            clip(getWidth() - diffPix, 0, diffPix, getHeight());
            drawPartialArea(0, getHeight(), getWidth() - diffPix, getWidth());
            unclip();
        } else {
            leftPix = newLeftPix;
            lineInfos.setAllInvalid();
            redraw();
        }

    } else if (leftPix > newLeftPix) {
        long diffPix = leftPix - newLeftPix;
        if (!cursorVisible && cursorIsBlinking) {
            startCursorBlinking();
        }
        if (diffPix < getWidth()) {
            redrawChanged(getTopLeftTextPosition(), textData->getLength()); // assure that new lineinfos match screen content
            if (getGuiWidget().isValid()) {
                XCopyArea(getDisplay(), getGuiWidget()->getWid(), getGuiWidget()->getWid(), textWidget_gcid,
                        0, 0,
                        getWidth() - diffPix, getHeight(),
                        diffPix, 0);
            }
            exposureNeedsSync = true;
            leftPix = newLeftPix;
            lineInfos.setAllInvalid();
            clip(0, 0, diffPix, getHeight());
            drawPartialArea(0, getHeight(), 0, diffPix);
            unclip();
        } else {
            leftPix = newLeftPix;
            lineInfos.setAllInvalid();
            redraw();
        }
    }
}

void TextWidget::setLeftPix(long newLeftPix)
{
    internSetLeftPix(newLeftPix);
    updateHorizontalScrollBar= true;
}

void TextWidget::setVerticalAdjustmentStrategy(VerticalAdjustmentStrategy verticalAdjustmentStrategy)
{
    this->verticalAdjustmentStrategy = verticalAdjustmentStrategy;
}

void TextWidget::setHorizontalAdjustmentStrategy(HorizontalAdjustmentStrategy horizontalAdjustmentStrategy)
{
    this->horizontalAdjustmentStrategy = horizontalAdjustmentStrategy;
}

void TextWidget::setLastEmptyLineStrategy(LastEmptyLineStrategy lastEmptyLineStrategy)
{
    this->lastEmptyLineStrategy = lastEmptyLineStrategy;
}

void TextWidget::setPosition(const Position& p)
{
    FocusableWidget::setPosition(p);
}

void TextWidget::processGuiWidgetNewPositionEvent(const Position& p)
{
    Position newPosition = p;
    
    if (getPosition() != newPosition || !hasPosition)
    {
        if (!hasPosition)
        {
            exposureNeedsSync = true;
            hasPosition       = true;
        }
        textData->flushPendingUpdates();
        
        long oldTopLineNumber = getTopLineNumber();
        
        int oldVisibleLines = getNumberOfVisibleLines();

        int cursorLine = getCursorLineNumber();
        int cursorPixX = getCursorPixX();
        bool cursorWasInVisibleArea = false;
        bool cursorShouldBeInVisibleArea = false;
        int oldCursorLinesToTop;
        int oldCursorLinesToBottom;
        int oldCursorPixXToLeft;
        int oldCursorPixXToRight;
        
        if (cursorLine >= oldTopLineNumber && cursorLine < oldTopLineNumber + getNumberOfVisibleLines()
            && !neverShowCursorFlag)
        {
            if (cursorPixX >= getLeftPix() && cursorPixX < getRightPix()) {
                cursorWasInVisibleArea      = true;
                cursorShouldBeInVisibleArea = true;
                oldCursorLinesToTop = cursorLine - oldTopLineNumber;
                oldCursorLinesToBottom = oldTopLineNumber + getNumberOfVisibleLines() - cursorLine;
                oldCursorPixXToLeft = cursorPixX - getLeftPix();
                oldCursorPixXToRight = getRightPix() - cursorPixX;
            }
        }
        int oldPositionH = getHeight();
        int oldPositionW = getWidth();
        int oldLeftPix   = getLeftPix();
        
        //newPosition.w = calculateWidthOrHeightWithoutBorder(newPosition.w, border);
        //newPosition.h = calculateWidthOrHeightWithoutBorder(newPosition.h, border);

        int newVisibleLines = util::maximum(0, (newPosition.h - 2 * border) / lineHeight); // not rounded
        long newTopLine;
        bool forceRedraw = false;
        
        if (   verticalAdjustmentStrategy ==     STRICT_TOP_LINE_ANCHOR
            || verticalAdjustmentStrategy == NOT_STRICT_TOP_LINE_ANCHOR)
        {
            newTopLine = oldTopLineNumber;
            FocusableWidget::processGuiWidgetNewPositionEvent(newPosition);
        }
        else if (oldTopLineNumber + oldVisibleLines - newVisibleLines >= 0)
        {
            getGuiWidget()->setBitGravity(StaticGravity); // screen content can be preservered
            FocusableWidget::processGuiWidgetNewPositionEvent(newPosition);
            getGuiWidget()->setBitGravity(NorthWestGravity);

            newTopLine = oldTopLineNumber + oldVisibleLines - newVisibleLines;

            oldTopLineNumber = newTopLine; // no forceRedraw below
            textData->moveMarkToLineAndWCharColumn(topMarkId, oldTopLineNumber, 0);

        } else {
            newTopLine = 0;
            forceRedraw = true;
            getGuiWidget()->setBitGravity(StaticGravity); // prevent flickering
            FocusableWidget::processGuiWidgetNewPositionEvent(newPosition);
            getGuiWidget()->setBitGravity(NorthWestGravity);
        }


        unclip();

        
        visibleLines = newVisibleLines; // not rounded
        
        lineInfos.setLength(util::maximum(1, ROUNDED_UP_DIV(newPosition.h - 2*border, lineHeight)));

        lineInfos.setAllInvalid();

        int  newLeftPix = oldLeftPix;

        long numberOfLines = getNumberOfLines();
        
        if (verticalAdjustmentStrategy == NOT_STRICT_TOP_LINE_ANCHOR
            && newTopLine != 0 
            && newTopLine + visibleLines > numberOfLines)
        {
            newTopLine = numberOfLines - visibleLines;
            if (newTopLine < 0)
                newTopLine = 0;
        }
        if (cursorShouldBeInVisibleArea)
        {
            if (cursorLine < newTopLine) {
                newTopLine = cursorLine;
            }
            if (cursorLine >= newTopLine + visibleLines) {
                newTopLine = cursorLine - visibleLines + 1;
            }
            if (cursorPixX <= oldLeftPix) {
                newLeftPix = cursorPixX - defaultTextStyle->getSpaceWidth();
                if (newLeftPix < 0) {
                    newLeftPix = 0;
                }
            }
            if (cursorPixX >= oldLeftPix + newPosition.w - defaultTextStyle->getSpaceWidth()) {
                newLeftPix = cursorPixX - newPosition.w + defaultTextStyle->getSpaceWidth();
                if (newLeftPix < 0) {
                    newLeftPix = 0;
                }
            }
        }
        
        bool setLeftPix = false;
        
        totalPixWidth = 0;
        if (newTopLine != oldTopLineNumber || forceRedraw) {
            if (newLeftPix != oldLeftPix || forceRedraw) {
                leftPix = newLeftPix;
                textData->moveMarkToLineAndWCharColumn(topMarkId, newTopLine, 0);
                redraw();
            } else {
                setTopLineNumber(newTopLine);
            }
        }
        else if (newLeftPix != oldLeftPix) {
            setLeftPix = true;
        }

        if (horizontalAdjustmentStrategy == NOT_STRICT_LEFT_COLUMN_ANCHOR)
        {
            long longestVisiblePixWidth = calcLongestVisiblePixWidth(); // calcLongestVisiblePixWidth after topline has been set
            
            if (newLeftPix + getWidth() > longestVisiblePixWidth)
            {
                long newLeftPix2 = longestVisiblePixWidth - getWidth();
                if (newLeftPix2 < 0) {
                    newLeftPix2 = 0;
                }
                if (newLeftPix2 != newLeftPix) {
                    setLeftPix = true;
                    newLeftPix = newLeftPix2;
                }
            }       
        }
        else if (horizontalAdjustmentStrategy == RIGHT_COLUMN_ANCHOR)
        {
            long longestVisiblePixWidth = calcLongestVisiblePixWidth(); // calcLongestVisiblePixWidth after topline has been set
            long newLeftPix2 = longestVisiblePixWidth - getWidth();
            if (newLeftPix2 < 0) {
                newLeftPix2 = 0;
            }
            if (newLeftPix2 != newLeftPix) {
                setLeftPix = true;
                newLeftPix = newLeftPix2;
            }
        }
        if (setLeftPix) {
            internSetLeftPix(newLeftPix);
        }
        
        totalPixWidth = 0;
        calcTotalPixWidth();
        updateVerticalScrollBar = true;
        updateHorizontalScrollBar = true;
    }
}

GuiElement::Measures TextWidget::internalGetDesiredMeasures()
{
    int incrWidth  = getSpaceCharWidth();
    int incrHeight = getLineHeight();
    
    Measures rslt(  minWidthChars * defaultTextStyle->getSpaceWidth() + 2*border, 
                     minHeightChars * lineHeight + 2*border, 
                    
                     bestWidthChars * defaultTextStyle->getSpaceWidth() + 2*border, 
                    bestHeightChars * lineHeight + 2*border,

                     maxWidthChars == INT_MAX ? INT_MAX :  maxWidthChars * defaultTextStyle->getSpaceWidth() + 2*border,
                    maxHeightChars == INT_MAX ? INT_MAX : maxHeightChars * lineHeight + 2*border,

                    incrWidth, incrHeight);

//    return Measures( 2 * defaultTextStyle->getSpaceWidth() + 2*border, lineHeight + 2*border, 
//                    35 * defaultTextStyle->getSpaceWidth() + 2*border, lineHeight + 2*border,
//                    INT_MAX, INT_MAX,
//                    defaultTextStyle->getSpaceWidth(), lineHeight);

    return rslt;
}



void TextWidget::unclip()
{
    XSetClipMask(getDisplay(), textWidget_gcid, None);
}


void TextWidget::clip(int x, int y, int w, int h)
{
    XRectangle r;
    r.x = x;
    r.y = y;
    r.width  = w;
    r.height = h;
    XSetClipRectangles(getDisplay(), textWidget_gcid, 
            0, 0, &r, 1, Unsorted);    
}

void TextWidget::blinkCursor()
{
    if (!neverShowCursorFlag)
    {
        if (cursorIsBlinking)
        {
            const TimeVal now = TimeVal::now();
            
            if (now.isLaterThan(cursorNextBlinkTime))
            {
                cursorVisible = !cursorVisible;
                drawCursor(getCursorTextPosition());
                
                cursorNextBlinkTime = now + MicroSeconds(400000);

                EventDispatcher::getInstance()->registerTimerCallback(cursorNextBlinkTime, cursorBlinkCallback);
    
            } else { 
            }
        }
    }
}

void TextWidget::setCursorActive()
{
    if (!neverShowCursorFlag)
    {
        cursorIsActive = true;
        cursorVisible = true;
        drawCursor(getCursorTextPosition());
    }
}

void TextWidget::setCursorInactive()
{
    if (!neverShowCursorFlag)
    {
        cursorIsActive = false;
        cursorVisible = true;
        drawCursor(getCursorTextPosition());
    }
}


void TextWidget::hideCursor()
{
    cursorIsBlinking = false;
    if (cursorVisible) {
        cursorVisible = false;
        drawCursor(getCursorTextPosition());
    }
}

void TextWidget::showCursor()
{
    if (!neverShowCursorFlag)
    {
        cursorIsBlinking = false;
        if (!cursorVisible) {
            cursorVisible = true;
            drawCursor(getCursorTextPosition());
        }
    }
}

void TextWidget::stopCursorBlinking()
{
    if (!neverShowCursorFlag)
    {
        cursorIsBlinking = false;
        if (!cursorVisible) {
            cursorVisible = true;
            drawCursor(getCursorTextPosition());
        }
    }
}

void TextWidget::startCursorBlinking()
{
    if (!neverShowCursorFlag)
    {
        textData->flushPendingUpdates();
    
        if (!cursorVisible) {
            cursorVisible = true;
            drawCursor(getCursorTextPosition());
        }
    
        cursorNextBlinkTime = TimeVal::now() + MicroSeconds(400000);
    
        cursorIsBlinking = true;
        EventDispatcher::getInstance()->registerTimerCallback(cursorNextBlinkTime, cursorBlinkCallback);
    }
}


void TextWidget::moveCursorToFreePos(FreePos freePos)
{
    textData->flushPendingUpdates();

    long pos          = freePos.pos;
    long extraColumns = freePos.extraColumns;

    ASSERT(extraColumns == 0 || textData->isEndOfLine(pos));
    ASSERT(0 <= pos && pos <= textData->getLength());

    if (   extraColumns != cursorColumnsBehindEndOfLine
        || pos          != textData->getTextPositionOfMark(cursorMarkId))
    {
        bool cursorWasVisible = cursorVisible;
        if (cursorWasVisible) {
            cursorVisible = false;
            drawCursor(getCursorTextPosition());
        }
        long oldCursorPos = textData->getTextPositionOfMark(cursorMarkId);
        long topLinePos   = textData->getTextPositionOfMark(topMarkId);
        if (abs(pos - topLinePos) < abs(pos - oldCursorPos)) {
            textData->moveMarkToPosOfMark(cursorMarkId, topMarkId);
        }
        {
            cursorColumnsBehindEndOfLine = extraColumns;
            textData->moveMarkToPos(cursorMarkId, pos);
        }
        if (cursorIsBlinking) {
            startCursorBlinking();
        } else if (cursorWasVisible && !neverShowCursorFlag) {
            cursorVisible = true;
            drawCursor(getCursorTextPosition());
        }
    }
}

void TextWidget::moveCursorToWCharColumn(long newColumn)
{
    ASSERT(0 <= newColumn);

    long oldColumn = getCursorWCharColumn();
    
    moveCursorRelativeWCharColumns(newColumn - oldColumn);
}

void TextWidget::moveCursorRelativeWCharColumns(long columns)
{
    textData->flushPendingUpdates();

    if (columns != 0)
    {
        bool cursorWasVisible = cursorVisible;
        if (cursorWasVisible) {
            cursorVisible = false;
            drawCursor(getCursorTextPosition());
        }
        {
            long p = getCursorTextPosition();

            if (columns > 0)
            {
                if (cursorColumnsBehindEndOfLine > 0)
                {
                    cursorColumnsBehindEndOfLine += columns;
                }
                else
                {
                    long c = 0;
                    while (c < columns && !textData->isEndOfLine(p)) {
                        p = textData->getNextBeginOfWChar(p);
                        ++c;
                    }
                    if (c < columns) {
                        cursorColumnsBehindEndOfLine = columns - c;
                    } else {
                        cursorColumnsBehindEndOfLine = 0;
                    }
                }
            }
            else {
                columns = -columns;
                if (cursorColumnsBehindEndOfLine >= columns) {
                    cursorColumnsBehindEndOfLine -= columns;
                }
                else
                {
                    columns -= cursorColumnsBehindEndOfLine;
                    cursorColumnsBehindEndOfLine = 0;
                    long c = 0; 
                    while (c < columns && !textData->isBeginOfLine(p)) {
                        p = textData->getPrevBeginOfWChar(p);
                        ++c;
                    }
                }
            }
            
            textData->moveMarkToPos(cursorMarkId, p);
        }
        if (cursorIsBlinking) {
            startCursorBlinking();
        } else if (cursorWasVisible && !neverShowCursorFlag) {
            cursorVisible = true;
            drawCursor(getCursorTextPosition());
        }
    }
}

void TextWidget::moveCursorToTextMark(TextData::MarkHandle m)
{
    textData->flushPendingUpdates();
    
    ASSERT(0 <= textData->getTextPositionOfMark(m) && textData->getTextPositionOfMark(m) <= textData->getLength());

    if (cursorColumnsBehindEndOfLine != 0 || textData->getTextPositionOfMark(cursorMarkId) != textData->getTextPositionOfMark(m))
    {
        bool cursorWasVisible = cursorVisible;
        if (cursorWasVisible) {
            cursorVisible = false;
            drawCursor(getCursorTextPosition());
        }
        {
            cursorColumnsBehindEndOfLine = 0;
            textData->moveMarkToPosOfMark(cursorMarkId, m);
        }
        if (cursorIsBlinking) {
            startCursorBlinking();
        } else if (cursorWasVisible && !neverShowCursorFlag) {
            cursorVisible = true;
            drawCursor(getCursorTextPosition());
        }
    }
}

inline long TextWidget::fillCursorColumnsBehindEndOfLine()
{
    long rslt = 0;
    if (cursorColumnsBehindEndOfLine != 0)
    {
        ASSERT(cursorMarkId.isAtEndOfLine());
        ByteArray whiteSpaces;
        whiteSpaces.appendAndFillAmountWith(cursorColumnsBehindEndOfLine, ' ');
        textData->insertAtMark(cursorMarkId, whiteSpaces);
        textData->moveMarkToPos(cursorMarkId, cursorMarkId.getPos() + cursorColumnsBehindEndOfLine);
       
        rslt = cursorColumnsBehindEndOfLine;
               cursorColumnsBehindEndOfLine = 0;
    }
    return rslt;
}

TextData::TextMark TextWidget::createNewMarkFromCursor()
{
    return textData->createNewMark(cursorMarkId);
}

long TextWidget::insertAtCursor(char c)
{
    return fillCursorColumnsBehindEndOfLine() + textData->insertAtMark(cursorMarkId, c);
}

long TextWidget::insertAtCursor(const ByteArray& buffer)
{
    return fillCursorColumnsBehindEndOfLine() + textData->insertAtMark(cursorMarkId, buffer);
}

long TextWidget::insertAtCursor(const byte* buffer, long length)
{
    return fillCursorColumnsBehindEndOfLine() + textData->insertAtMark(cursorMarkId, buffer, length);
}

void TextWidget::removeAtCursor(long amount)
{
    fillCursorColumnsBehindEndOfLine();
    amount = util::minimum(amount, textData->getLength() - getCursorTextPosition());
    if (amount > 0) {
        textData->removeAtMark(cursorMarkId, amount);
    }
}



static inline bool areIntersected(XRectangle* r1, XRectangle* r2)
{
    bool rslt = false;
    
    if (r1->x < r2->x) {
        if (r2->x - r1->x < r1->width)
            rslt = true;
    } else {
        if (r1->x - r2->x < r2->width)
            rslt = true;
    }
    if (!rslt)
        return false;
    
    if (r1->y < r2->y) {
        if (r2->y - r1->y < r1->height)
            return true;
    } else {
        if (r1->y - r2->y < r2->height)
            return true;
    }
    return false;
}

GuiWidget::ProcessingResult TextWidget::processGuiWidgetEvent(const XEvent* event)
{
    switch (event->type)
    {
        case NoExpose: {
            exposureNeedsSync = false;
            return GuiWidget::EVENT_PROCESSED;
        }
        case GraphicsExpose:
        case Expose: {

            XRectangle r;
            int count;
            if (event->type == GraphicsExpose) {
                r.x      = event->xgraphicsexpose.x;
                r.y      = event->xgraphicsexpose.y;
                r.width  = event->xgraphicsexpose.width;
                r.height = event->xgraphicsexpose.height;
                count    = event->xgraphicsexpose.count; // Anzahl der noch folgenden Events
                if (count == 0) {
                    exposureNeedsSync = false;
                }
            } else {
                r.x      = event->xexpose.x;
                r.y      = event->xexpose.y;
                r.width  = event->xexpose.width;
                r.height = event->xexpose.height;
                count    = event->xexpose.count; // Anzahl der noch folgenden Events
            }
            {
                Region newRegion = XCreateRegion();
                XUnionRectWithRegion(&r, redrawRegion, newRegion);
                XDestroyRegion(redrawRegion);
                redrawRegion = newRegion;
            }
/*            XEvent newEvent;
            while (XCheckWindowEvent(getDisplay(), getGuiWidget()->getWid(), ExposureMask, &newEvent) == True)
            {
                if (newEvent.type == GraphicsExpose) {
                    r.x      = newEvent.xgraphicsexpose.x;
                    r.y      = newEvent.xgraphicsexpose.y;
                    r.width  = newEvent.xgraphicsexpose.width;
                    r.height = newEvent.xgraphicsexpose.height;
                    count    = newEvent.xgraphicsexpose.count; // Anzahl der noch folgenden Events
                } else {
                    r.x      = newEvent.xexpose.x;
                    r.y      = newEvent.xexpose.y;
                    r.width  = newEvent.xexpose.width;
                    r.height = newEvent.xexpose.height;
                    count    = newEvent.xexpose.count; // Anzahl der noch folgenden Events
                }
                Region newRegion = XCreateRegion();
                XUnionRectWithRegion(&r, redrawRegion, newRegion);
                XDestroyRegion(redrawRegion);
                redrawRegion = newRegion;
            }
*/            if (count == 0)
            {
                XSetRegion(getDisplay(), textWidget_gcid, redrawRegion);
                redraw();
                unclip();
                XDestroyRegion(redrawRegion);
                redrawRegion = XCreateRegion();
            }
            return GuiWidget::EVENT_PROCESSED;
        }
        default:
            return GuiWidget::NOT_PROCESSED;
    }
}


void TextWidget::processAllExposureEvents()
{
/*    if (getGuiWidget().isValid())
    {
        XEvent newEvent;
        {
            while (XCheckWindowEvent(getDisplay(), getGuiWidget()->getWid(), ExposureMask, &newEvent) == True)
            {
                this->processGuiWidgetEvent(&newEvent);
            }
        }
        if (exposureNeedsSync)
        {
            XSync(getDisplay(), False);
            exposureNeedsSync = false;   
            while (XCheckWindowEvent(getDisplay(), getGuiWidget()->getWid(), ExposureMask, &newEvent) == True)
            {
                this->processGuiWidgetEvent(&newEvent);
            }
        }
    }
*/
}

static inline bool adjustLineInfoPosition(long* pos, long beginChangedPos, long oldEndChangedPos, long changedAmount)
{
    ASSERT(0 <= *pos);

    if (*pos > oldEndChangedPos) {
        *pos += changedAmount;
        return true;
    } else if (*pos < beginChangedPos) {
        return true;
    } else {
        return false;
    }
}

void TextWidget::treatTextDataUpdate(TextData::UpdateInfo u)
{
    if (cursorColumnsBehindEndOfLine > 0 && !cursorMarkId.isAtEndOfLine())
    {
        long p = cursorMarkId.getPos();
        long i = cursorColumnsBehindEndOfLine;
        do {
            p = textData->getNextWCharPos(p);
            --i;
        }
        while (i > 0 && !textData->isEndOfLine(p));
        cursorColumnsBehindEndOfLine = i;
        cursorMarkId.moveToPos(p);
    }
    
    
    bool redraw = false;
    long newEndChangedPos = u.oldEndChangedPos + u.changedAmount;

    long oldTopPos = textData->getTextPositionOfMark(topMarkId);
    if (!textData->isBeginOfLine(topMarkId.getPos())) {
        textData->moveMarkToBeginOfLine(topMarkId);
    }
    long newTopPos = textData->getTextPositionOfMark(topMarkId);

    if ((newTopPos <= newEndChangedPos+1 || newTopPos <= u.oldEndChangedPos+1) && endPos >= u.beginChangedPos) {
                                                       // not > because text could end before display
        redraw = true;
        if (newTopPos >= textData->getLength()) {
            leftPix = 0;
        }
    }

    for (int i = 0; i < lineInfos.getLength(); ++i) {
        RawPtr<LineInfo> li = lineInfos.getPtr(i);
        if (li->valid)
        {
            if (adjustLineInfoPosition(&li->beginOfLinePos, u.beginChangedPos, u.oldEndChangedPos, u.changedAmount)
             && adjustLineInfoPosition(&li->endOfLinePos,   u.beginChangedPos, u.oldEndChangedPos, u.changedAmount))
            {
                if (li->startPos != -1) {
                    adjustLineInfoPosition(&li->startPos, u.beginChangedPos, u.oldEndChangedPos, u.changedAmount);
                    adjustLineInfoPosition(&li->endPos,   u.beginChangedPos, u.oldEndChangedPos, u.changedAmount);
                }
            } else {
                redraw = true;
                //li->valid = false;
            }
        }
    }

    if (redraw) {
        redrawChanged(u.beginChangedPos, u.oldEndChangedPos);
    }
    updateVerticalScrollBar = true;
    totalPixWidth = 0;
    calcTotalPixWidth();
    updateHorizontalScrollBar = true;
}


long TextWidget::getOpticalCursorColumn() const
{
    const long cursorPos    = getCursorTextPosition();
    const long hardTabWidth = hilitingBuffer->getLanguageMode()->getHardTabWidth();
    long       opticalCursorColumn = 0;
    long realByteColumn = textData->getByteColumnNumberOfMark(cursorMarkId);
    for (long p = cursorPos - realByteColumn; p < cursorPos;) {
        if (textData->hasWCharAtPos(TAB_CHARACTER, p)) {
            opticalCursorColumn = ((opticalCursorColumn / hardTabWidth) + 1) * hardTabWidth;
        } else {
            ++opticalCursorColumn;
        }
        p = textData->getNextWCharPos(p);
    }
    return opticalCursorColumn + cursorColumnsBehindEndOfLine;
}


long TextWidget::getOpticalColumn(long pos) const
{
    const long hardTabWidth = hilitingBuffer->getLanguageMode()->getHardTabWidth();
    const long lineBegin    = textData->getThisLineBegin(pos);
    long       opticalCursorColumn = 0;
    for (long p = lineBegin; p < pos;) {
        if (textData->hasWCharAtPos(TAB_CHARACTER, p)) {
            opticalCursorColumn = ((opticalCursorColumn / hardTabWidth) + 1) * hardTabWidth;
        } else {
            ++opticalCursorColumn;
        }
        p = textData->getNextWCharPos(p);
    }
    return opticalCursorColumn;
}


void TextWidget::flushPendingUpdates()
{
    if (updateVerticalScrollBar) {
        scrollBarVerticalValueRangeChangedCallback->call(
                    getNumberOfLines(),
                    visibleLines, 
                    getTopLineNumber());
        updateVerticalScrollBar = false;
    }
    if (updateHorizontalScrollBar) {
        scrollBarHorizontalValueRangeChangedCallback->call(
                    totalPixWidth,
                    getWidth(), leftPix);
        updateHorizontalScrollBar = false;
    }
    long newLine = getCursorLineNumber();
    long newColumn = getOpticalCursorColumn();
    long newPos    = getCursorTextPosition();

    long newSelectionLength = 0;
    if (backliteBuffer->hasActiveSelection() && backliteBuffer->isSelectionPrimary()) {
        newSelectionLength = backliteBuffer->getSelectionWCharsInSameLineLength();
    }
    if (   newLine != lastLineOfLineAndColumnListeners || newColumn != lastColumnOfLineAndColumnListeners
        || newPos != lastPosOfLineAndColumnListeners || newSelectionLength != lastLengthOfSelectionLengthListeners)
    {
        lastLineOfLineAndColumnListeners = newLine;
        lastColumnOfLineAndColumnListeners = newColumn;
        lastPosOfLineAndColumnListeners = newPos;
        lastLengthOfSelectionLengthListeners = newSelectionLength;

        lineAndColumnListeners.invokeAllCallbacks(CursorPositionData(newLine, 
                                                                     newColumn, 
                                                                     newPos, 
                                                                     newSelectionLength));
    }
}

void TextWidget::setDesiredMeasuresInChars(int minWidth, int minHeight, 
        int bestWidth, int bestHeight, int maxWidth, int maxHeight)
{
    this->minWidthChars   = minWidth;
    this->minHeightChars  = minHeight;
    this->bestWidthChars  = bestWidth;
    this->bestHeightChars = bestHeight;
    this->maxWidthChars   = maxWidth;
    this->maxHeightChars  = maxHeight;
}

void TextWidget::setDesiredMeasuresInChars(int bestWidth, int bestHeight)
{
    this->bestWidthChars  = bestWidth;
    this->bestHeightChars = bestHeight;
}

void TextWidget::treatNotificationOfHotKeyEventForOtherWidget()
{
    if (isCursorBlinking()) {
        startCursorBlinking(); // redraw Cursor while Hotkey for other widget
    }
}

void TextWidget::internalShowMousePointer()
{
    isMousePointerHidden = false;
    if (getGuiWidget().isValid()) {
        XDefineCursor(getDisplay(), getGuiWidget()->getWid(), TextWidgetSingletonData::getInstance()->getTextMouseCursor());
    }
}


void TextWidget::internalHideMousePointer()
{
    isMousePointerHidden = true;
    if (getGuiWidget().isValid()) {
        XDefineCursor(getDisplay(), getGuiWidget()->getWid(), TextWidgetSingletonData::getInstance()->getEmptyMouseCursor());
    }
}


void TextWidget::setBackgroundColor(GuiColor color)
{
    // do not set background in the parent class, because we have no x11 handeld background!
    backgroundColor = color;
    if (getGuiWidget().isValid()) {
        getGuiWidget()->setBorderColor(backgroundColor);
    }
}

