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

#include <X11/cursorfont.h>

#include "TextWidget.hpp"
#include "util.hpp"
#include "TimeVal.hpp"
#include "EventDispatcher.hpp"
#include "GlobalConfig.hpp"

#define CURSOR_WIDTH 2

using namespace LucED;

static bool textWidgetInitialized = false;
static GC textWidget_gcid;

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
    Cursor getTextMouseCursor() { return textMouseCursor; }

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
    }
    
    ~TextWidgetSingletonData()
    {
        XFreeCursor(GuiRoot::getInstance()->getDisplay(), emptyMouseCursor);
        XFreeCursor(GuiRoot::getInstance()->getDisplay(), textMouseCursor);
    }
    
    static SingletonInstance<TextWidgetSingletonData> instance;

    Cursor emptyMouseCursor;
    Cursor textMouseCursor;
};

} // namespace LucED


using namespace LucED;

SingletonInstance<TextWidgetSingletonData> TextWidgetSingletonData::instance;


/**
 * TextWidget-Constructor.
 */
TextWidget::TextWidget(GuiWidget *parent, TextStyles::Ptr textStyles, HilitedText::Ptr hilitedText, int border)

    : GuiWidget(parent, 0, 0, textStyles->get(0)->getSpaceWidth()*200, textStyles->get(0)->getLineHeight(), border),

      position(GuiWidget::getPosition()),
      textData(hilitedText->getTextData()),
      cursorBlinkCallback(this, &TextWidget::blinkCursor),
      textStyles(textStyles),
      hilitingBuffer(HilitingBuffer::create(hilitedText)),
      backliteBuffer(BackliteBuffer::create(textData)),
      lineInfos(),
      topMarkId(textData->createNewMark()),
      cursorMarkId(textData->createNewMark()),
      opticalCursorColumn(0),
      minWidthChars(10),
      minHeightChars(1),
      bestWidthChars(80),
      bestHeightChars(25),
      maxWidthChars(INT_MAX),
      maxHeightChars(INT_MAX),
      border(border),
      adjustment(VerticalAdjustment::TOP),
      isMousePointerHidden(false),
      
      primarySelectionColor(  getGuiRoot()->getGuiColor(GlobalConfig::getInstance()->getPrimarySelectionColor())),
      secondarySelectionColor(getGuiRoot()->getGuiColor(GlobalConfig::getInstance()->getPseudoSelectionColor()))
{
    totalPixWidth = 0;
    leftPix = 0;
    endPos = 0;
    cursorVisible = 0;
    cursorIsBlinking = false;
    cursorIsActive = false;
    updateVerticalScrollBar = false;
    updateHorizontalScrollBar = false;

    if (!textWidgetInitialized) {
        textWidgetInitialized = true;
        textWidget_gcid = XCreateGC(getDisplay(), getRootWid(), 0, NULL);
        XSetGraphicsExposures(getDisplay(), textWidget_gcid, True);
    }

    setBackgroundColor(getGuiRoot()->getWhiteColor());
    setBorderColor(getGuiRoot()->getWhiteColor());

    lineHeight  = textStyles->get(0)->getLineHeight();
    lineAscent  = textStyles->get(0)->getLineAscent();

    visibleLines = position.h / lineHeight; // not rounded;

    lineInfos.setLength(ROUNDED_UP_DIV(position.h, lineHeight));

    addToXEventMask(ExposureMask);

//    XSetWindowAttributes at;
//    at.backing_store = Always;
//    at.bit_gravity = StaticGravity; // NorthWestGravity; //StaticGravity;
//    XChangeWindowAttributes(getDisplay(), getWid(), 
//            //CWBackingStore|
//            CWBitGravity, &at);
    setBitGravity(NorthWestGravity);
            
    textData->flushPendingUpdates();
    textData->registerUpdateListener(Callback1<TextData::UpdateInfo>(this, &TextWidget::treatTextDataUpdate));
    
    EventDispatcher::getInstance()->registerUpdateSource(Callback0(this, &TextWidget::flushPendingUpdates));

    hilitingBuffer->registerUpdateListener(Callback1<HilitingBuffer::UpdateInfo>(this, &TextWidget::treatHilitingUpdate));
    backliteBuffer->registerUpdateListener(Callback1<HilitingBuffer::UpdateInfo>(this, &TextWidget::treatHilitingUpdate));
    
    redrawRegion = XCreateRegion();
    
    XDefineCursor(getDisplay(), getWid(), TextWidgetSingletonData::getInstance()->getTextMouseCursor());
}

TextWidget::~TextWidget()
{
    XDestroyRegion(redrawRegion);
}

void TextWidget::registerLineAndColumnListener(const Callback2<long,long>& listener) {
    lineAndColumnListeners.registerCallback(listener);
}

void TextWidget::treatHilitingUpdate(HilitingBuffer::UpdateInfo update)
{
    ASSERT(update.beginPos <= update.endPos);
    
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
    TextWidgetFillLineInfoIterator(const TextData*       textData, 
                                   HilitingBuffer*       hilitingBuffer, 
                                   BackliteBuffer*       backliteBuffer,
                                   const TextStyles*     textStyles,
                                   long                  textPos)
        : textData(textData),
          hilitingBuffer(hilitingBuffer),
          backliteBuffer(backliteBuffer),
          textStyles(textStyles),
          pixelPos(0),
          textPos(textPos),
          isEndOfLineFlag(textData->isEndOfLine(textPos)),
          c(isEndOfLineFlag ? 0 : textData->getChar(textPos)),
          styleIndex(hilitingBuffer->getTextStyle(textPos)),
          style(textStyles->get(styleIndex)),
          spaceWidth(textStyles->get(0)->getSpaceWidth()),
          tabWidth(hilitingBuffer->getLanguageMode()->getHardTabWidth() * textStyles->get(0)->getSpaceWidth()),
          charWidth(isEndOfLineFlag ? 0 : style->getCharWidth(c)),
          doBackgroundFlag(true),
          background(backliteBuffer->getBackground(textPos))
    {}
    bool isAtEndOfLine()    const { return isEndOfLineFlag; }
    long getTextPos()       const { return textPos; }
    long getPixelPos()      const { return pixelPos; }
    unsigned char getChar() const { return c; }
    int getCharWidth() const { return charWidth; }
    int getTabWidth()  const { return tabWidth; }
    int getSpaceWidth() const { return spaceWidth; }
    int getBackground() const { ASSERT(doBackgroundFlag == true); return background; }

    int setDoBackground(bool flag)
    {
        doBackgroundFlag = flag; 
        if (flag) {
            background = backliteBuffer->getBackground(textPos);
        }
    }

    int getCharRBearing() const
    { 
        if (c != '\t') {
            return style->getCharRBearing(c);
        } else {
            return (((pixelPos / tabWidth) + 1) * tabWidth) - pixelPos;
        }
    }
    int getCharLBearing() const { return style->getCharLBearing(c); }
    int getStyleIndex()  const { return styleIndex; }

    void increment()
    {
        ASSERT(isEndOfLineFlag == false);

        if (c== '\t') {
            pixelPos  = ((pixelPos / tabWidth) + 1) * tabWidth;
        } else {
            pixelPos += charWidth;
        }
        textPos += 1;
        if (!textData->isEndOfLine(textPos)) {
            c          = textData->getChar(textPos);
            styleIndex = hilitingBuffer->getTextStyle(textPos);
            style      = textStyles->get(styleIndex);
            charWidth  = style->getCharWidth(c);
        } else {
            isEndOfLineFlag = true;
            c          = 0;
            styleIndex = 0;
            style      = textStyles->get(0);
            charWidth  = 0;
        }
        if (doBackgroundFlag == true) {
            background = backliteBuffer->getBackground(textPos);
        }
    }

private:
    const TextData*   const textData;
    HilitingBuffer*   const hilitingBuffer;
    BackliteBuffer*   const backliteBuffer;
    const TextStyles* const textStyles;
    long pixelPos;
    long textPos;
    bool isEndOfLineFlag;
    unsigned char c;
    int  styleIndex;
    const TextStyle* style;
    const int tabWidth;
    const int spaceWidth;
    int charWidth;
    bool doBackgroundFlag;
    int background;
};

class TextWidgetFragmentFiller
{
public:
    TextWidgetFragmentFiller(MemArray<LineInfo::FragmentInfo>& fragments)
        : fragments(fragments),
          lastStyle(-1),
          lastBackground(-1),
          lastStylePixBegin(-1),
          lastStyleBeginTextPos(-1)
    {}

    bool hasStyleChanged(const TextWidgetFillLineInfoIterator& i) const
    {
        return lastStyle      != i.getStyleIndex() 
            || lastBackground != i.getBackground()
            || i.getChar() == '\t';
    }

    bool hasRememberedStyle() const
    {
        return lastStyle != -1;
    }

    void completeFragment(const TextWidgetFillLineInfoIterator& i) 
    {
        ASSERT(lastStyle             != -1);
        ASSERT(lastStyleBeginTextPos != -1);
        ASSERT(lastStylePixBegin     != -1);
        ASSERT(fragments.getLength() >=  1);

        fragments.getLast().numberBytes = i.getTextPos() - lastStyleBeginTextPos;
        fragments.getLast().pixWidth    = i.getPixelPos() - lastStylePixBegin;
    }

    void beginNewFragment(const TextWidgetFillLineInfoIterator& i)
    {
        if (i.getChar() == '\t') {
            lastStyle = -2;
        } else {
            lastStyle = i.getStyleIndex();
        }
        lastBackground        = i.getBackground();
        lastStylePixBegin     = i.getPixelPos();
        lastStyleBeginTextPos = i.getTextPos();

        fragments.appendAmount(1);
        fragments.getLast().background = i.getBackground();
        fragments.getLast().styleIndex = i.getStyleIndex();
    #ifdef DEBUG
        fragments.getLast().numberBytes = -1;
        fragments.getLast().pixWidth    = -1;
    #endif
    }

private:
    MemArray<LineInfo::FragmentInfo>&       fragments;
    int                                     lastStyle;
    int                                     lastBackground;
    long                                    lastStylePixBegin;
    long                                    lastStyleBeginTextPos;
};

} // namespace LucED


void TextWidget::fillLineInfo(long beginOfLinePos, LineInfo* li)
{
    TextWidgetFillLineInfoIterator i(textData, hilitingBuffer, backliteBuffer, textStyles, beginOfLinePos);
    TextWidgetFragmentFiller       f(li->fragments);
    
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
            
            if (i.getPixelPos() + i.getCharLBearing() >= this->leftPix + this->position.w)
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
            
            long addLength = i.getTextPos() - li->startPos;
            memcpy(li->outBuf.appendAmount(addLength), 
                   textData->getAmount(li->startPos, addLength),
                   addLength);

            byte* stylesPtr = hilitingBuffer->getTextStyles(li->startPos, addLength);
            if (stylesPtr != NULL) {
                memcpy(li->styles.appendAmount(addLength), 
                       stylesPtr,
                       addLength);
            } else {
                memset(li->styles.appendAmount(addLength), 0, addLength);
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

        li->backgroundToEnd = i.getBackground();
    }

    li->valid = true;
    li->endOfLinePos = i.getTextPos();
    li->totalPixWidth = i.getPixelPos();
    li->pixWidth = i.getPixelPos() - (this->leftPix - li->leftPixOffset);
    li->isEndOfText = (i.getTextPos() == textData->getLength());
    
    ASSERT((print == 0 && (   (li->startPos == -1 && li->endPos == -1)
                           || (li->startPos >=  0 && li->endPos >= li->startPos)
                          )
           )
        || (print != 0 && li->startPos >=  0 && li->endPos >=  li->startPos));
        
    ASSERT(li->styles.getLength() == li->endPos - li->startPos);
}

inline void TextWidget::applyTextStyle(int styleIndex)
{
    const TextStyle *style = textStyles->get(styleIndex);
    
//    XSetBackground(getDisplay(), textWidget_gcid, getGuiRoot()->getWhiteColor());

    XSetForeground(getDisplay(), textWidget_gcid, style->getColor());
    XSetFont(      getDisplay(), textWidget_gcid, style->getFontId());
}

inline int TextWidget::calcVisiblePixX(LineInfo *li, long pos)
{
    int x = -li->leftPixOffset;
    int  p, i;
    
    ASSERT(li->valid);
    ASSERT(li->startPos <= pos && pos <= li->endPos);
    ASSERT(pos - li->startPos <= li->styles.getLength());
    
    for (i = 0, p = li->startPos; p < pos; ++i, ++p) {
        unsigned char c = textData->getChar(p);
        int style = li->styles[i];
        if (c == '\t') {
            int tabWidth = hilitingBuffer->getLanguageMode()->getHardTabWidth() * textStyles->get(0)->getSpaceWidth();
            long totalX = leftPix + x;
            totalX = (totalX / tabWidth + 1) * tabWidth;
            x = totalX - leftPix;
        } else {
            x += textStyles->get(style)->getCharWidth(c);
        }
    }
    return x;
}

inline GuiColor TextWidget::getColorForBackground(byte background)
{
    switch (background)
    {
        case 0:  return getGuiRoot()->getWhiteColor();
        case 1:  return primarySelectionColor;
        case 2:  return secondarySelectionColor;
        default: ASSERT(false);
                 return getGuiRoot()->getWhiteColor();
    }
}


inline void TextWidget::clearLine(LineInfo *li, int y)
{
/*{
    long x = 0;
    
    XSetForeground(getDisplay(), textWidget_gcid, getGuiRoot()->getWhiteColor());
    XFillRectangle(getDisplay(), getWid(), textWidget_gcid, 
            x, y, position.w - x, lineHeight);
}*/
///////////////////


}



inline void TextWidget::clearPartialLine(LineInfo *li, int y, int x1, int x2)
{
    ByteArray& buf = li->outBuf;
    MemArray<LineInfo::FragmentInfo>& fragments = li->fragments;
    byte *ptr, *end;
    long x = -li->leftPixOffset;;
    long accX = -li->leftPixOffset;;
    int  accBackground = -1;
    
    if (buf.getLength() > 0) {

        ptr = buf.getPtr(0);
        end = ptr + buf.getLength();

        for (int i = 0, n = fragments.getLength(); i < n; ++i)
        {
            ASSERT(ptr < end);
            
            int background = fragments[i].background;
            int styleIndex = fragments[i].styleIndex;
            int len        = fragments[i].numberBytes;
            int pixWidth   = fragments[i].pixWidth;

            if (background != accBackground) {
                if (accBackground != -1) {
                    XSetForeground(getDisplay(), textWidget_gcid, getColorForBackground(accBackground));
                    int useX1 = accX;
                    int useX2 = x;
                    if (useX1 < x2 && useX2 >= x1) {
                        XFillRectangle(getDisplay(), getWid(), textWidget_gcid, 
                                useX1, y, useX2 - useX1, lineHeight);
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
            int useX2 = position.w;
            if (useX1 < x2 && useX2 >= x1) {
                XFillRectangle(getDisplay(), getWid(), textWidget_gcid, 
                        useX1, y, useX2 - useX1, lineHeight);
            }
        } else {
            int useX1 = accX;
            int useX2 = x;
            if (useX1 < x2 && useX2 >= x1) {
                XFillRectangle(getDisplay(), getWid(), textWidget_gcid, 
                        useX1, y, useX2 - useX1, lineHeight);
            }
            XSetForeground(getDisplay(), textWidget_gcid, getColorForBackground(li->backgroundToEnd));
            useX1 = x;
            useX2 = position.w;
            if (useX1 < x2 && useX2 >= x1) {
                XFillRectangle(getDisplay(), getWid(), textWidget_gcid, 
                        useX1, y, useX2 - useX1, lineHeight);
            }
        }
    } else {
        XSetForeground(getDisplay(), textWidget_gcid, getColorForBackground(li->backgroundToEnd));
        int useX1 = x;
        int useX2 = position.w;
        if (useX1 < x2 && useX2 >= x1) {
            XFillRectangle(getDisplay(), getWid(), textWidget_gcid, 
                    useX1, y, useX2 - useX1, lineHeight);
        }
    }

}

inline void TextWidget::printPartialLineWithoutCursor(LineInfo *li, int y, int x1, int x2)
{
    ByteArray& buf = li->outBuf;
    MemArray<LineInfo::FragmentInfo>& fragments = li->fragments;
    unsigned char *ptr, *end;
    int x = -li->leftPixOffset;
    
    if (buf.getLength() > 0) {

        ptr = buf.getPtr(0);
        end = ptr + buf.getLength();

        for (int i = 0, n = fragments.getLength(); i < n; ++i)
        {
            ASSERT(ptr < end);
            
            int background = fragments[i].background;
            int styleIndex = fragments[i].styleIndex;
            int len        = fragments[i].numberBytes;
            int pixWidth   = fragments[i].pixWidth;

            const TextStyle *style = textStyles->get(styleIndex);
            
            if (x + style->getCharLBearing(*ptr) >= x2) {
                break;
            }
            
            if (len > 0 && *ptr != '\t'
                    && x + pixWidth 
                         - style->getCharWidth(ptr[len - 1])
                         + style->getCharRBearing(ptr[len - 1]) >= x1) {
                unsigned char *ptrpend = ptr + len;
                unsigned char *ptrp = ptr;
                unsigned char *ptrp1;
                unsigned char *ptrp2;
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
                XDrawString(getDisplay(), getWid(), 
                        textWidget_gcid, xp1, y + lineAscent, (char*) ptrp1, ptrp2 - ptrp1);
                //XDrawString(getDisplay(), tw->wid, 
                //        tw->gcid, x, y + tw->lineAscent, ptr, len);
                //printf("print <%.*s> \n", len, ptr);
            }
            ptr += len;
            x += pixWidth;
        }
    }
}


inline void TextWidget::printPartialLine(LineInfo *li, int y, int x1, int x2)
{
    long cursorPos = getCursorTextPosition();
    
    clearPartialLine(li, y, x1, x2);
    
    if (cursorVisible && (li->startPos <= cursorPos  && cursorPos <= li->endPos)) {
        
        // cursor visible in this line
    
        int cursorX = calcVisiblePixX(li, cursorPos);
        
        if (cursorX < x2 && x1 <= cursorX + CURSOR_WIDTH - 1) {
        
            // cursor visible in this part
            
            if (cursorIsActive) {
                XSetForeground(getDisplay(), textWidget_gcid, getGuiRoot()->getBlackColor());
            } else {
                XSetForeground(getDisplay(), textWidget_gcid, getGuiRoot()->getGreyColor());
            }
            XFillRectangle(getDisplay(), getWid(), textWidget_gcid, 
                    cursorX, y, CURSOR_WIDTH, lineHeight);
        }
        
    }
    printPartialLineWithoutCursor(li, y, x1, x2);
}


inline void TextWidget::printLine(LineInfo *li, int y)
{
    long cursorPos      = getCursorTextPosition();
    bool considerCursor = false;
    int  cursorX;
    
    if (cursorVisible && (li->startPos <= cursorPos  && cursorPos <= li->endPos)) {
        // cursor visible in this line
        considerCursor = true;
        cursorX = calcVisiblePixX(li, cursorPos);
    }

    ByteArray& buf                    = li->outBuf;
    MemArray<LineInfo::FragmentInfo>& fragments = li->fragments;
    byte *ptr, *end;
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
            int len        = fragments[i].numberBytes;
            int pixWidth   = fragments[i].pixWidth;

            int tx1 = -leftPixOffset + x;
            int tx2 = tx1 + pixWidth;

            int bx1 = -leftPixOffset + lastBackgroundX;
            int bx2 = tx2;

            XSetForeground(getDisplay(), textWidget_gcid, getColorForBackground(background));
            XFillRectangle(getDisplay(), getWid(), textWidget_gcid, 
                    bx1, y, bx2 - bx1, lineHeight);
            
            lastBackgroundX = x + pixWidth;
            
            if (len > 0 && *ptr != '\t' ) 
            {
                const TextStyle* style = textStyles->get(styleIndex);
                int txCorrection = - style->getCharWidth(ptr[len - 1]) + style->getCharRBearing(ptr[len - 1]);
                
                if (txCorrection > 0)
                {
                    if (i + 1 < n) {
                        XSetForeground(getDisplay(), textWidget_gcid, getColorForBackground(fragments[i + 1].background));
                    } else {
                        XSetForeground(getDisplay(), textWidget_gcid, getColorForBackground(li->backgroundToEnd));
                    }
                    XFillRectangle(getDisplay(), getWid(), textWidget_gcid, 
                            tx2, y, txCorrection, lineHeight);
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
                XFillRectangle(getDisplay(), getWid(), textWidget_gcid,     // DrawCursor
                        cx1, y, cx2 - cx1, lineHeight);
            }

            if (len > 0 && *ptr != '\t') {
                applyTextStyle(styleIndex);
                XDrawString(getDisplay(), getWid(), 
                        textWidget_gcid, -leftPixOffset + x, y + lineAscent, (char*) ptr, len);
                //printf("print <%.*s> \n", len, ptr);
            }
            ptr += len;
            x += pixWidth;
        }
    }
    {
        int bx1 = -leftPixOffset + lastBackgroundX;
        int bx2 = position.w;
        XSetForeground(getDisplay(), textWidget_gcid, getColorForBackground(li->backgroundToEnd));
        XFillRectangle(getDisplay(), getWid(), textWidget_gcid, 
                bx1, y, bx2 - bx1, lineHeight);

        if (considerCursor && cursorX < bx2 && cursorX + CURSOR_WIDTH > bx1) {
            if (cursorIsActive) {
                XSetForeground(getDisplay(), textWidget_gcid, getGuiRoot()->getBlackColor());
            } else {
                XSetForeground(getDisplay(), textWidget_gcid, getGuiRoot()->getGreyColor());
            }
            int cx1 = util::maximum(cursorX, bx1);
            int cx2 = util::minimum(cursorX + CURSOR_WIDTH, bx2);
            XFillRectangle(getDisplay(), getWid(), textWidget_gcid,     // DrawCursor
                    cx1, y, cx2 - cx1, lineHeight);
        }
    }
}

void TextWidget::printChangedPartOfLine(LineInfo* newLi, int y, LineInfo* oldLi)
{
    ByteArray& newBuf                              = newLi->outBuf;
    ByteArray& oldBuf                              = oldLi->outBuf;
    
    long newBufLength = newBuf.getLength();
    long oldBufLength = oldBuf.getLength();
    
    MemArray<LineInfo::FragmentInfo>& newFragments = newLi->fragments;
    MemArray<LineInfo::FragmentInfo>& oldFragments = oldLi->fragments;

    if (oldBufLength == 0 || newBufLength == 0) {
        printLine(newLi, y);
        return;
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
    
    int newFragmentMaxBufIndex = newBufIndex + newFragments[newFragmentIndex].numberBytes;
    int oldFragmentMaxBufIndex = oldBufIndex + oldFragments[oldFragmentIndex].numberBytes;

    int newStyleIndex = newFragments[newFragmentIndex].styleIndex;
    int oldStyleIndex = oldFragments[oldFragmentIndex].styleIndex;

    int newBackground   = newFragments[newFragmentIndex].background;
    int oldBackground   = oldFragments[oldFragmentIndex].background;

    const TextStyle* newStyle = textStyles->get(newStyleIndex);
    const TextStyle* oldStyle = textStyles->get(oldStyleIndex);

    int tabWidth = hilitingBuffer->getLanguageMode()->getHardTabWidth() * textStyles->get(0)->getSpaceWidth();
    
    do
    {
        byte newChar = newBuf[newBufIndex];
        byte oldChar = oldBuf[oldBufIndex];
        
        int  newCharLBearing = newStyle->getCharLBearing(newChar);
        int  newCharWidth;
        int  newCharRBearing;
        if (newChar == '\t') {
            newCharWidth    = (((newX + leftPix) / tabWidth) + 1) * tabWidth - (newX + leftPix);
            newCharRBearing = newCharWidth;
        } else {
            newCharWidth    = newStyle->getCharWidth(newChar);
            newCharRBearing = newStyle->getCharRBearing(newChar);
        }
        
        int  oldCharLBearing = oldStyle->getCharLBearing(oldChar);
        int  oldCharWidth;
        int  oldCharRBearing;
        if (oldChar == '\t') {
            oldCharWidth    = (((oldX + leftPix) / tabWidth) + 1) * tabWidth - (oldX + leftPix);
            oldCharRBearing = oldCharWidth;
        } else {
            oldCharWidth    = oldStyle->getCharWidth(oldChar);
            oldCharRBearing = oldStyle->getCharRBearing(oldChar);
        }
        
        
        if (   newX != oldX
            || newChar != oldChar
            || newStyleIndex != oldStyleIndex
            || newBackground != oldBackground)
        {
            if (xMin == -1) {
                xMin = util::minimum(newX + newCharLBearing, oldX + oldCharLBearing);
                if (xMin < 0) xMin = 0;
            }
            xMax = util::maximum(newX + newCharRBearing, oldX + oldCharRBearing);
        }

        
        newBufIndex += 1;
        oldBufIndex += 1;
        
        if (newBufIndex >= newFragmentMaxBufIndex) {
            newFragmentIndex += 1;
            if (newFragmentIndex < newFragmentCount)
            {
                newFragmentMaxBufIndex = newBufIndex + newFragments[newFragmentIndex].numberBytes;
                newStyleIndex          = newFragments[newFragmentIndex].styleIndex;
                newBackground          = newFragments[newFragmentIndex].background;
                newStyle               = textStyles->get(newStyleIndex);
            } else {
                newEnd = true;
            }
        }        
        if (oldBufIndex >= oldFragmentMaxBufIndex) {
            oldFragmentIndex += 1;
            if (oldFragmentIndex < oldFragmentCount)
            {
                oldFragmentMaxBufIndex = oldBufIndex + oldFragments[oldFragmentIndex].numberBytes;
                oldStyleIndex          = oldFragments[oldFragmentIndex].styleIndex;
                oldBackground          = oldFragments[oldFragmentIndex].background;
                oldStyle               = textStyles->get(oldStyleIndex);
            } else {
                oldEnd = true;
            }
        }
        if (newEnd || oldEnd) {
            if (xMin == -1) {
                xMin = util::minimum(newX + newCharLBearing, oldX + oldCharLBearing);
                if (xMin < 0) xMin = 0;
                xMax = position.w;
            }
        } else {
            newX += newCharWidth;
            oldX += oldCharWidth;
        }
    }
    while (!newEnd && !oldEnd);
    
    if (newEnd && oldEnd && newLi->backgroundToEnd == oldLi->backgroundToEnd) {
        clip(xMin, y, xMax - xMin, lineHeight);
        printPartialLine(newLi, y, xMin, xMax);
        unclip();
    } else {
        clip(xMin, y, position.w - xMin, lineHeight);
        printPartialLine(newLi, y, xMin, position.w);
        unclip();
    }
}

void TextWidget::drawPartialArea(int minY, int maxY, int x1, int x2)
{
    int y = 0;
    int line = 0;
    long pos = getTopLeftTextPosition();
    long oldpos;
    
    do {
        LineInfo *li = lineInfos.getPtr(line);

        if (!li->valid) {
            fillLineInfo(pos, li);
        }
        if (y + lineHeight > minY && y < maxY) {
            printPartialLine(li, y, x1, x2);
        }
        y    += lineHeight;
        line += 1;
        pos   = li->endOfLinePos;

        oldpos = pos;
        pos  += textData->getLengthOfLineEnding(pos);
    
    } while (oldpos != pos && y < position.h);
    
    if (y < position.h) {

//        XClearArea(XGlobal_display_pst, tw->wid, 
//                0, y, 0, 0, False);

        XSetForeground(getDisplay(), textWidget_gcid, getGuiRoot()->getWhiteColor());
        XFillRectangle(getDisplay(), getWid(), textWidget_gcid, 
                0, y, position.w, position.h - y);
    }
    endPos = pos;
}


void TextWidget::drawArea(int minY, int maxY)
{
    int y = 0;
    int line = 0;
    long pos = getTopLeftTextPosition();
    long oldpos;

    if (lineInfos.getLength() > 0) {
        do {
            LineInfo *li = lineInfos.getPtr(line);

            if (!li->valid) {
                fillLineInfo(pos, li);
            }
            if (y + lineHeight > minY && y < maxY) {
                printLine(li, y);
            }
            y    += lineHeight;
            line += 1;
            pos   = li->endOfLinePos;

            oldpos = pos;
            pos  += textData->getLengthOfLineEnding(pos);

        } while (oldpos != pos && y < position.h);
    }
    
    if (y < position.h) {

//        XClearArea(getDisplay(), tw->wid, 
//                0, y, 0, 0, False);

        XSetForeground(getDisplay(), textWidget_gcid, getGuiRoot()->getWhiteColor());
        XFillRectangle(getDisplay(), getWid(), textWidget_gcid, 
                0, y, position.w, position.h - y);
    }
    endPos = pos;
}

void TextWidget::redrawChanged(long spos, long epos)
{
    int minY = 0;
    int maxY = position.h;
    int y = 0;
    int line = 0;
    long pos = getTopLeftTextPosition();
    long oldpos;

    unclip();

    if (lineInfos.getLength() > 0) {
        do {
            LineInfo *li = lineInfos.getPtr(line);

            if (li->valid && li->beginOfLinePos == pos) {
                
                if (li->beginOfLinePos <= epos && li->endOfLinePos >= spos) // ">=" because line info for zero length lines should also be checked because of selection backliting
                {
                    tempLineInfo = *li;
                    fillLineInfo(pos, li);

                    if (tempLineInfo.isDifferentOnScreenThan(*li)) {
                        if (y + lineHeight > minY && y < maxY) {
                            //printLine(li, y);
                            printChangedPartOfLine(li, y, &tempLineInfo);
                        }
                    }
                }
            } else {
                fillLineInfo(pos, li);

                if (y + lineHeight > minY && y < maxY) {
                    printLine(li, y);
                }
            }
            pos = li->endOfLinePos;
            y    += lineHeight;
            line += 1;

            oldpos = pos;
            pos  += textData->getLengthOfLineEnding(pos);

        } while (oldpos != pos && y < maxY);
        while (line < lineInfos.getLength()) {
            lineInfos.getPtr(line)->valid = false;
            ++line;
        }
    }
    
    if (y < maxY) {

//        XClearArea(XGlobal_display_pst, tw->wid, 
//                0, y, 0, 0, False);

        XSetForeground(getDisplay(), textWidget_gcid, getGuiRoot()->getWhiteColor());
        XFillRectangle(getDisplay(), getWid(), textWidget_gcid, 
                0, y, position.w, position.h - y);
    }
    endPos = pos;
}



inline void TextWidget::redraw() {
    drawArea(0, position.h);
}

LineInfo* TextWidget::getValidLineInfo(long line)
{
    LineInfo *li = lineInfos.getPtr(line);

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

        } while (l < line);
    }
    ASSERT(li->valid);
    return li;
}

void TextWidget::drawCursor(long cursorPos)
{
    int y = 0;
    int line = 0;
    LineInfo *li;
    
    if (lineInfos.getLength() > 0) {
        do {
            li = getValidLineInfo(line);

            if (cursorPos < li->startPos) {
                break;
            }
            if (cursorPos <= li->endPos) {
                // cursor is visible
                int cursorX = calcVisiblePixX(li, cursorPos);
                clip(cursorX - 1, y, CURSOR_WIDTH + 2, lineHeight);
                printPartialLine(li, y, cursorX - 1, cursorX + CURSOR_WIDTH + 1);
                unclip();
                break;
            }
            y    += lineHeight;
            line += 1;

        } while (!li->isEndOfText && y < position.h);
    }
}

long TextWidget::getCursorPixX()
{
    textData->flushPendingUpdates();
    
    long cursorPos = getCursorTextPosition();
    int line = getCursorLineNumber() - getTopLineNumber();
    long lineBegin;
    
    if (0 <= line && line < visibleLines) {

        LineInfo *li = getValidLineInfo(line);
    
        if (li->startPos <= cursorPos && cursorPos <= li->endPos) {
            // cursor visible
            return leftPix + calcVisiblePixX(li, cursorPos);
        }
        lineBegin = li->beginOfLinePos;
    } else {
        lineBegin = textData->getThisLineBegin(cursorPos);
    }
    // Fallback if not visible
    long x = 0;
    for (long p = lineBegin; p < cursorPos; ++p) {
        unsigned char c = textData->getChar(p);
        if (c == '\t') {
            int tabWidth = hilitingBuffer->getLanguageMode()->getHardTabWidth() * textStyles->get(0)->getSpaceWidth();
            x = (x / tabWidth + 1) * tabWidth;
        } else {
            int style = hilitingBuffer->getTextStyle(p);
            x += textStyles->get(style)->getCharWidth(c);
        }
    }
    return x;
}

long TextWidget::calcLongestVisiblePixWidth()
{
    int y = 0;
    int line = 0;
    long pos = getTopLeftTextPosition();
    long rslt = 0;
    long oldpos;
    
    if (lineInfos.getLength() > 0) {
        do {
            LineInfo *li = lineInfos.getPtr(line);
            if (!li->valid) {
                fillLineInfo(pos, li);
            }
            if (li->totalPixWidth > rslt)
                rslt = li->totalPixWidth;
            pos = li->endOfLinePos;
            y    += lineHeight;
            line += 1;

            oldpos = pos;
            pos  += textData->getLengthOfLineEnding(pos);

        } while (oldpos != pos && y < position.h);
    }
    rslt += CURSOR_WIDTH;

    return rslt;
}

void TextWidget::calcTotalPixWidth()
{
    long rslt = calcLongestVisiblePixWidth();

    totalPixWidth = 0;
    MAXIMIZE(&totalPixWidth, leftPix + position.w);
    MAXIMIZE(&totalPixWidth, rslt);
}


void TextWidget::setTopLineNumber(long n)
{
    unclip();
    
    textData->flushPendingUpdates();
    processAllExposureEvents();
    
    calcTotalPixWidth();

    long oldTopLineNumber = getTopLineNumber();
    
    if ( n > textData->getNumberOfLines() - 1)
        n = textData->getNumberOfLines() - 1;
    
    if (n < 0) 
        n = 0;
        
    if (n != oldTopLineNumber)
    {
        
        lineInfos.moveFirst(n - oldTopLineNumber);
        textData->moveMarkToLineAndColumn(topMarkId, n, 0);
        totalPixWidth = 0; // reset TextWidget::calcTotalPixWidth
        
        if (n < oldTopLineNumber) {
            if (oldTopLineNumber - n >= visibleLines) {
                redraw();
            } else {
                long diff = oldTopLineNumber - n;
                
                XCopyArea(getDisplay(), getWid(), getWid(), 
                        textWidget_gcid,
                        0, 0,
                        position.w, position.h - diff * lineHeight,
                        0, diff * lineHeight);
                drawArea(0, diff * lineHeight);
            }
        } else {
            if (n - oldTopLineNumber >= visibleLines) {
                redraw();
            } else {
                long diff = n - oldTopLineNumber;
                
                XCopyArea(getDisplay(), getWid(), getWid(), 
                        textWidget_gcid,
                        0, diff * lineHeight,
                        position.w, position.h - diff * lineHeight,
                        0, 0);
            
                drawArea(position.h - (diff * lineHeight), position.h);
            }

        }
        updateVerticalScrollBar = true;

        calcTotalPixWidth();
        updateHorizontalScrollBar= true;

        if (!cursorVisible && cursorIsBlinking) {
            startCursorBlinking();
        }
    }
    processAllExposureEvents();
}


long TextWidget::getTextPosForPixX(long pixX, long beginOfLinePos)
{
    long p = beginOfLinePos;
    long x = 0, ox = 0;
    ASSERT(textData->isBeginOfLine(p));
    while (x < pixX && !textData->isEndOfLine(p)) {
        int c = textData->getChar(p);
        ox = x;
        if (c == '\t') {
            long tabWidth = hilitingBuffer->getLanguageMode()->getHardTabWidth() * textStyles->get(0)->getSpaceWidth();
            x = (x / tabWidth + 1) * tabWidth;
        } else {
            x += textStyles->get(hilitingBuffer->getTextStyle(p))->getCharWidth(c);
        }
        p += 1;
    }
    if (p >= 1) {
        if (x - pixX > pixX - ox) {
            p -= 1;
        }
    }
    return p;
}

long TextWidget::getTextPosFromPixXY(int pixX, int pixY, bool optimizeForThinCursor)
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

    if (pixLine >= textData->getNumberOfLines()) {
        pixLine = textData->getNumberOfLines() - 1;
    }
    long totalPixX = pixX + leftPix;
    
    int line = -1;
    long pos;
    long nextPos = getTopLeftTextPosition();

    LineInfo* li;
    
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
        return li->endOfLinePos;
    } else {
        pixX += li->leftPixOffset;
        int i = 0;
        int endI = li->styles.getLength();

        int x, nextX;
        for (x = 0, nextX = 0; nextX < pixX && i < endI; ++i) {
            x = nextX;
            const TextStyle* style = textStyles->get(li->styles[i]);
            byte c = textData->getChar(li->startPos + i);
            if (c == '\t') {
                long tabWidth = hilitingBuffer->getLanguageMode()->getHardTabWidth() * textStyles->get(0)->getSpaceWidth();
                nextX = (x / tabWidth + 1) * tabWidth;
            } else {
                nextX += style->getCharWidth(c);
            }
        }
        if (optimizeForThinCursor) {
            if (i == 0 || nextX - pixX < pixX -x) {
                return li->startPos + i;
            } else {
                return li->startPos + i - 1;
            }
        } else {
            if (i == 0) {
                return li->startPos + i;
            } else {
                return li->startPos + i - 1;
            }
        }
    }
}


void TextWidget::internSetLeftPix(long newLeftPix)
{
    textData->flushPendingUpdates();
    processAllExposureEvents();

    calcTotalPixWidth();

    if (newLeftPix < 0)
        newLeftPix = 0;
    
    if (newLeftPix > totalPixWidth - position.w)
        newLeftPix = totalPixWidth - position.w;


    if (leftPix < newLeftPix) {
        long diffPix = newLeftPix - leftPix;
        if (!cursorVisible && cursorIsBlinking) {
            startCursorBlinking();
        }
        if (diffPix < position.w) {
            XCopyArea(getDisplay(), getWid(), getWid(), textWidget_gcid,
                    diffPix, 0,
                    position.w - diffPix, position.h,
                    0, 0);
            leftPix = newLeftPix;
            lineInfos.setAllInvalid();
            clip(position.w - diffPix, 0, diffPix, position.h);
            drawPartialArea(0, position.h, position.w - diffPix, position.w);
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
        if (diffPix < position.w) {
            XCopyArea(getDisplay(), getWid(), getWid(), textWidget_gcid,
                    0, 0,
                    position.w - diffPix, position.h,
                    diffPix, 0);
            leftPix = newLeftPix;
            lineInfos.setAllInvalid();
            clip(0, 0, diffPix, position.h);
            drawPartialArea(0, position.h, 0, diffPix);
            unclip();
        } else {
            leftPix = newLeftPix;
            lineInfos.setAllInvalid();
            redraw();
        }
    }
    processAllExposureEvents();
}

void TextWidget::setLeftPix(long newLeftPix)
{
    internSetLeftPix(newLeftPix);
    updateHorizontalScrollBar= true;
}

void TextWidget::setResizeAdjustment(VerticalAdjustment::Type adjustment)
{
    this->adjustment = adjustment;
}


void TextWidget::setPosition(Position newPosition)
{
    if (position != newPosition)
    {
        textData->flushPendingUpdates();
        processAllExposureEvents();
        
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
        
        if (cursorLine >= oldTopLineNumber && cursorLine < oldTopLineNumber + getNumberOfVisibleLines()) {
            if (cursorPixX >= getLeftPix() && cursorPixX < getRightPix()) {
                cursorWasInVisibleArea      = true;
                cursorShouldBeInVisibleArea = true;
                oldCursorLinesToTop = cursorLine - oldTopLineNumber;
                oldCursorLinesToBottom = oldTopLineNumber + getNumberOfVisibleLines() - cursorLine;
                oldCursorPixXToLeft = cursorPixX - getLeftPix();
                oldCursorPixXToRight = getRightPix() - cursorPixX;
            }
        }
        int oldPositionH = position.h;
        int oldPositionW = position.w;
        int oldLeftPix   = getLeftPix();
        
        newPosition.w = calculateWidthOrHeightWithoutBorder(newPosition.w, border);
        newPosition.h = calculateWidthOrHeightWithoutBorder(newPosition.h, border);

        int newVisibleLines = newPosition.h / lineHeight; // not rounded
        long newTopLine;
        bool forceRedraw = false;
        
        if (adjustment == VerticalAdjustment::TOP) {
            newTopLine = oldTopLineNumber;
            GuiWidget::setPosition(newPosition);
        } else if (oldTopLineNumber + oldVisibleLines - newVisibleLines >= 0) {
            setBitGravity(StaticGravity); // screen content can be preservered
            GuiWidget::setPosition(newPosition);
            setBitGravity(NorthWestGravity);

            newTopLine = oldTopLineNumber + oldVisibleLines - newVisibleLines;
            oldTopLineNumber = newTopLine;
            textData->moveMarkToLineAndColumn(topMarkId, oldTopLineNumber, 0);
        } else {
            newTopLine = 0;
            forceRedraw = true;
            setBitGravity(StaticGravity); // prevent flickering
            GuiWidget::setPosition(newPosition);
            setBitGravity(NorthWestGravity);
        }


        position = newPosition;
        unclip();

        
        visibleLines = newVisibleLines; // not rounded
        
        lineInfos.setLength(ROUNDED_UP_DIV(newPosition.h, lineHeight));
        lineInfos.setAllInvalid();

        int  newLeftPix = oldLeftPix;

        if (oldTopLineNumber != 0 && 
                oldTopLineNumber + visibleLines > textData->getNumberOfLines())
        {
            long newTopLine = textData->getNumberOfLines() - visibleLines;
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
                newLeftPix = cursorPixX - textStyles->get(0)->getSpaceWidth();
                if (newLeftPix < 0) {
                    newLeftPix = 0;
                }
            }
            if (cursorPixX >= oldLeftPix + newPosition.w - textStyles->get(0)->getSpaceWidth()) {
                newLeftPix = cursorPixX - newPosition.w + textStyles->get(0)->getSpaceWidth();
                if (newLeftPix < 0) {
                    newLeftPix = 0;
                }
            }
        }
        
        totalPixWidth = 0;
        if (newTopLine != oldTopLineNumber || forceRedraw) {
            if (newLeftPix != oldLeftPix || forceRedraw) {
                leftPix = newLeftPix;
                textData->moveMarkToLineAndColumn(topMarkId, newTopLine, 0);
                redraw();
            } else {
                setTopLineNumber(newTopLine);
            }
        }
        else if (newLeftPix != oldLeftPix) {
            internSetLeftPix(newLeftPix);
        }
        totalPixWidth = 0;
        calcTotalPixWidth();
        updateVerticalScrollBar = true;
        updateHorizontalScrollBar = true;
    
        processAllExposureEvents();
    }
}

GuiElement::Measures TextWidget::getDesiredMeasures()
{
    
    Measures rslt(  minWidthChars * textStyles->get(0)->getSpaceWidth() + 2*border, 
                     minHeightChars * lineHeight + 2*border, 
                    
                     bestWidthChars * textStyles->get(0)->getSpaceWidth() + 2*border, 
                    bestHeightChars * lineHeight + 2*border,

                     maxWidthChars == INT_MAX ? INT_MAX :  maxWidthChars * textStyles->get(0)->getSpaceWidth() + 2*border,
                    maxHeightChars == INT_MAX ? INT_MAX : maxHeightChars * lineHeight + 2*border,

                    textStyles->get(0)->getSpaceWidth(), lineHeight);
//                    1, 1);
    
//    return Measures( 2 * textStyles->get(0)->getSpaceWidth() + 2*border, lineHeight + 2*border, 
//                    35 * textStyles->get(0)->getSpaceWidth() + 2*border, lineHeight + 2*border,
//                    INT_MAX, INT_MAX,
//                    textStyles->get(0)->getSpaceWidth(), lineHeight);
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
    if (cursorIsBlinking)
    {
        TimeVal now;
        now.setToCurrentTime();
        
        if (now.isLaterThan(cursorNextBlinkTime))
        {
            cursorVisible = !cursorVisible;
            drawCursor(getCursorTextPosition());
            
            cursorNextBlinkTime = now;
            cursorNextBlinkTime.add(MicroSeconds(400000));
            EventDispatcher::getInstance()->registerTimerCallback(cursorNextBlinkTime, cursorBlinkCallback);

        } else { 
        }
    }
}

void TextWidget::setCursorActive()
{
    cursorIsActive = true;
    cursorVisible = true;
    drawCursor(getCursorTextPosition());
}

void TextWidget::setCursorInactive()
{
    cursorIsActive = false;
    cursorVisible = true;
    drawCursor(getCursorTextPosition());
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
    cursorIsBlinking = false;
    if (!cursorVisible) {
        cursorVisible = true;
        drawCursor(getCursorTextPosition());
    }
}

void TextWidget::stopCursorBlinking()
{
    cursorIsBlinking = false;
    if (!cursorVisible) {
        cursorVisible = true;
        drawCursor(getCursorTextPosition());
    }
}

void TextWidget::startCursorBlinking()
{
    textData->flushPendingUpdates();

    if (!cursorVisible) {
        cursorVisible = true;
        drawCursor(getCursorTextPosition());
    }

    cursorNextBlinkTime.setToCurrentTime().add(MicroSeconds(400000));

    cursorIsBlinking = true;
    EventDispatcher::getInstance()->registerTimerCallback(cursorNextBlinkTime, cursorBlinkCallback);
}

void TextWidget::moveCursorToTextPosition(long pos)
{
    textData->flushPendingUpdates();
    
    ASSERT(0 <= pos && pos <= textData->getLength());

    if (textData->getTextPositionOfMark(cursorMarkId) != pos) {
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
        textData->moveMarkToPos(cursorMarkId, pos);
        if (cursorIsBlinking) {
            startCursorBlinking();
        } else if (cursorWasVisible) {
            cursorVisible = true;
            drawCursor(getCursorTextPosition());
        }
    }
}

void TextWidget::moveCursorToTextMark(TextData::MarkHandle m)
{
    textData->flushPendingUpdates();
    
    ASSERT(0 <= textData->getTextPositionOfMark(m) && textData->getTextPositionOfMark(m) <= textData->getLength());

    if (textData->getTextPositionOfMark(cursorMarkId) != textData->getTextPositionOfMark(m)) {
        bool cursorWasVisible = cursorVisible;
        if (cursorWasVisible) {
            cursorVisible = false;
            drawCursor(getCursorTextPosition());
        }
        textData->moveMarkToPosOfMark(cursorMarkId, m);
        if (cursorIsBlinking) {
            startCursorBlinking();
        } else if (cursorWasVisible) {
            cursorVisible = true;
            drawCursor(getCursorTextPosition());
        }
    }
}

TextData::TextMark TextWidget::createNewMarkFromCursor()
{
    return textData->createNewMark(cursorMarkId);
}

long TextWidget::insertAtCursor(char c)
{
    return textData->insertAtMark(cursorMarkId, c);
}

long TextWidget::insertAtCursor(const ByteArray& buffer)
{
    return textData->insertAtMark(cursorMarkId, buffer);
}

long TextWidget::insertAtCursor(const byte* buffer, long length)
{
    return textData->insertAtMark(cursorMarkId, buffer, length);
}

void TextWidget::removeAtCursor(long amount)
{
    amount = util::minimum(amount, textData->getLength() - getCursorTextPosition());
    if (amount > 0) {
        textData->removeAtMark(cursorMarkId, amount);
    }
}



static inline bool areIntersected(XRectangle *r1, XRectangle *r2)
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

GuiElement::ProcessingResult TextWidget::processEvent(const XEvent *event)
{
    if (GuiWidget::processEvent(event) == EVENT_PROCESSED) {
        return EVENT_PROCESSED;
    } else {
        switch (event->type) {
        
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
                XEvent newEvent;
                while (XCheckWindowEvent(getDisplay(), getWid(), ExposureMask, &newEvent) == True)
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
                if (count == 0)
                {
                    XSetRegion(getDisplay(), textWidget_gcid, redrawRegion);
                    redraw();
                    unclip();
                    XDestroyRegion(redrawRegion);
                    redrawRegion = XCreateRegion();
                }
                return EVENT_PROCESSED;
            }
            default:
                return NOT_PROCESSED;
        }
    }
}


void TextWidget::processAllExposureEvents()
{
    XFlush(getDisplay());
    XEvent newEvent;
    while (XCheckWindowEvent(getDisplay(), getWid(), ExposureMask, &newEvent) == True)
    {
        this->processEvent(&newEvent);
    }
}

static inline bool adjustLineInfoPosition(long *pos, long beginChangedPos, long oldEndChangedPos, long changedAmount)
{
    ASSERT(0 <= *pos);

    if (*pos >= oldEndChangedPos) {
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
        LineInfo *li = lineInfos.getPtr(i);
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
                li->valid = false;
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
    for (long p = cursorPos - getCursorColumn(); p < cursorPos; ++p) {
        if (textData->getChar(p) == '\t') {
            opticalCursorColumn = ((opticalCursorColumn / hardTabWidth) + 1) * hardTabWidth;
        } else {
            ++opticalCursorColumn;
        }
    }
    return opticalCursorColumn;
}


void TextWidget::flushPendingUpdates()
{
    if (updateVerticalScrollBar) {
        scrollBarVerticalValueRangeChangedCallback.call(
                    textData->getNumberOfLines(),
                    visibleLines, 
                    getTopLineNumber());
        updateVerticalScrollBar = false;
    }
    if (updateHorizontalScrollBar) {
        scrollBarHorizontalValueRangeChangedCallback.call(
                    totalPixWidth,
                    position.w, leftPix);
        updateHorizontalScrollBar = false;
    }
    lineAndColumnListeners.invokeAllCallbacks(getCursorLineNumber(), getOpticalCursorColumn());
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

void TextWidget::notifyAboutHotKeyEventForOtherWidget()
{
    if (isCursorBlinking()) {
        startCursorBlinking(); // redraw Cursor while Hotkey for other widget
    }
}

void TextWidget::internalShowMousePointer()
{
    isMousePointerHidden = false;
    XDefineCursor(getDisplay(), getWid(), TextWidgetSingletonData::getInstance()->getTextMouseCursor());
}


void TextWidget::internalHideMousePointer()
{
    isMousePointerHidden = true;
    XDefineCursor(getDisplay(), getWid(), TextWidgetSingletonData::getInstance()->getEmptyMouseCursor());
}


