/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2009 Oliver Schmidt, oliver at luced dot de
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

#include "config.h"

#if !defined(LUCED_USE_ICONV)
#  if HAVE_ICONV && !DISABLE_ICONV
#    define LUCED_USE_ICONV 1
#  else
#    define LUCED_USE_ICONV 0
#  endif
#endif

#if LUCED_USE_ICONV
#  include <iconv.h>
#endif

#include <errno.h>
#include <string.h>

#include "util.hpp"
#include "EncodingConverter.hpp"
#include "SystemException.hpp"
#include "EncodingException.hpp"
#include "ByteArray.hpp"
#include "System.hpp"

using namespace LucED;

namespace // anonymous namespace
{
    enum LowLevelResult
    {
        CONVERSION_OK,
        NON_REVERSIBLE_CONVERSIONS_OCCURRED,
        OUTPUT_BUFFER_TOO_SMALL,
        INVALID_SEQUENCE,
        UNKNOWN_ERROR
    };

} // anonymous namespace

#if LUCED_USE_ICONV

class EncodingConverter::LowLevelConverter : public NonCopyable
{
public:
    LowLevelConverter(const String& fromCodeset, 
                      const String& toCodeset) 

        : handle(iconv_open(toCodeset.toCString(),
                            fromCodeset.toCString())),
          fromCodeset(fromCodeset),
          toCodeset(toCodeset)
    {}
    ~LowLevelConverter() {
        if (isValid()) {
            iconv_close(handle);
        }
    }

    LowLevelResult convert(const char**  inbuf, size_t*  inbytesleft,
                                 char** outbuf, size_t* outbytesleft)
    {
        size_t rslt;

    #if ICONV_USES_CONST_POINTER
        rslt = iconv(handle, inbuf,  inbytesleft,
                             outbuf, outbytesleft);
    #else
        rslt = iconv(handle, (char**)inbuf,  inbytesleft,
                                     outbuf, outbytesleft);
    #endif

        if (rslt == (size_t)(-1)) {
            if (errno == E2BIG) {
                // The output buffer has no more room for the next converted character. 
                // In this case it sets errno to E2BIG and returns (size_t)(-1).
                return OUTPUT_BUFFER_TOO_SMALL;
            }
            else if (errno == EINVAL || errno == EILSEQ) {
                // An incomplete multibyte sequence is encountered in the input, and the 
                // input byte sequence terminates after it. In this case it sets errno 
                // to EINVAL and returns (size_t)(-1). *inbuf is left pointing to the 
                // beginning of the incomplete multibyte sequence.
                //
                // An invalid multibyte sequence is encountered in the input. In this 
                // case it sets errno to EILSEQ and returns (size_t)(-1). *inbuf is 
                // left pointing to the beginning of the invalid multibyte sequence.
                return INVALID_SEQUENCE;
            }
            else {
                // should not happen
                return UNKNOWN_ERROR;
            }
        } else {
            // The input byte sequence has been entirely converted, i.e. *inbytesleft has 
            // gone down to 0. In this case iconv returns the number of non-reversible 
            // conversions performed during this call.
            if (rslt == 0) {
                return CONVERSION_OK;
            } else {
                return NON_REVERSIBLE_CONVERSIONS_OCCURRED;
            }
        }
    }
    void reset() {
        if (isValid()) {
            iconv_close(handle);
            handle = iconv_open(toCodeset.toCString(),
                                fromCodeset.toCString());
        }
    }
    bool isValid() const {
        return handle != (iconv_t)(-1);
    }
private:
    iconv_t handle;
    String fromCodeset;
    String toCodeset;
};

#else // LUCED_USE_ICONV

class EncodingConverter::LowLevelConverter : public NonCopyable
{
private:
    enum ConvertDirection
    {
        DO_NOTHING,
        UTF8_TO_LATIN1,
        LATIN1_TO_UTF8
    };
public:
    LowLevelConverter(const String& fromCodeset, 
                      const String& toCodeset) 

        : fromCodeset(fromCodeset.toSubstitutedString("-", "").toLower()),
          toCodeset  (toCodeset  .toSubstitutedString("-", "").toLower())
    {
        init();
    }

    LowLevelResult convert(const char**  inbuf, size_t*  inbytesleft,
                                 char** outbuf, size_t* outbytesleft)
    {
        ASSERT(*inbytesleft >= 0 && *outbytesleft >= 0);
        
        LowLevelResult rslt = CONVERSION_OK;

        const char* inPtr   = *inbuf;
        char*       outPtr  = *outbuf;
        
        const char* const  inEndPtr =  *inbuf +  *inbytesleft;
        const char* const outEndPtr = *outbuf + *outbytesleft;

        switch (convertDirection)
        {
            case UTF8_TO_LATIN1:
            {
                while (inPtr < inEndPtr)
                {
                    const char*       p    = inPtr;
                    const char* const endP = util::minimum(inEndPtr, 
                                                           inPtr + (outEndPtr - outPtr));
                    
                    while (p < endP && CharUtil::isAsciiChar(*p)) { ++p; }
        
                    size_t amount = p - inPtr;
        
                    memmove(outPtr, inPtr, amount);
                    
                    outPtr += amount;
                    inPtr  += amount;
                    
                    while (inPtr < inEndPtr && !CharUtil::isAsciiChar(*inPtr))
                    {
                        // ignore this, because we always get the full input:
                        //    
                        // int number = CharUtil::getNumberOfStrictUtf8FollowerChars(*inPtr);
                        // if (inPtr + number < inEndPtr) {
                        //     rslt = INVALID_SEQUENCE;
                        //     goto End;
                        // }
                        if (outPtr + 1 > outEndPtr)
                        {
                            if (rslt == CONVERSION_OK) {
                                rslt = OUTPUT_BUFFER_TOO_SMALL;
                            }
                            goto End;
                        }

                        Adapter adapter((const byte*)inPtr, inEndPtr - inPtr);
                        long    pos = 0;
                        int c = Utf8Parser<Adapter>(&adapter).getWCharAndIncrementPos(&pos);
                        
                        inPtr += pos;
                        
                        if (0 <= c && c <= 0xff) {
                            *(outPtr++) = (char)c;
                        } 
                        else {
                            *(outPtr++) = '?';
                            rslt = NON_REVERSIBLE_CONVERSIONS_OCCURRED;
                        }
                    }
                }
                goto End;
            }
            case LATIN1_TO_UTF8:
            {
                while (inPtr < inEndPtr)
                {
                    const char*       p    = inPtr;
                    const char* const endP = util::minimum(inEndPtr, 
                                                           inPtr + (outEndPtr - outPtr));
                    
                    while (p < endP && CharUtil::isAsciiChar(*p)) { ++p; }
        
                    size_t amount = p - inPtr;
        
                    memmove(outPtr, inPtr, amount);
                    
                    outPtr += amount;
                    inPtr  += amount;
                    
                    while (inPtr < inEndPtr && !CharUtil::isAsciiChar(*inPtr))
                    {
                        if (outPtr + 2 > outEndPtr)
                        {
                            if (rslt == CONVERSION_OK) {
                                rslt = OUTPUT_BUFFER_TOO_SMALL;
                            }
                            goto End;
                        }
                        unsigned char c = *(inPtr++);                  // 0xC0 = 1100 0000
                        outPtr[0] =  (char)(0xC0 | ((c >> 6) & 0x1F)); // 0x1F = 0001 1111
                        outPtr[1] =  (char)(0x80 | (c & 0x3F));        // 0x80 = 1000 0000
                                                                       // 0x3F = 0011 1111
                        outPtr += 2;
                    }
                }
                goto End;
            }
            default:
            {
                size_t copySize = util::minimum(inEndPtr - inPtr, outEndPtr - outPtr);

                memmove(outPtr, inPtr, copySize);
                
                inPtr  += copySize;
                outPtr += copySize;
                
                goto End;
            }
        }
    End:
        *inbuf        = inPtr;
        *outbuf       = outPtr;
        *inbytesleft  = inEndPtr - inPtr;
        *outbytesleft = outEndPtr - outPtr;
        
        return rslt;
    }
    void reset() {
    }
    bool isValid() const {
        return validFlag;
    }
private:
    static bool isKnownEncoding(const String& encoding)
    {
        return    encoding == "c"
               || encoding == "posix"
               || encoding == "ascii"
               || encoding == "utf8"
               || encoding == "iso88591"
               || encoding == "latin1";
    }
    void init()
    {
        validFlag = isKnownEncoding(fromCodeset) && isKnownEncoding(toCodeset);

        if (fromCodeset == "utf8" && (   toCodeset == "c"
                                      || toCodeset == "posix"
                                      || toCodeset == "ascii"
                                      || toCodeset == "iso88591"
                                      || toCodeset == "latin1"))
        {
            convertDirection = UTF8_TO_LATIN1;
        }
        else if (toCodeset == "utf8" && (   fromCodeset == "c"
                                         || fromCodeset == "posix"
                                         || fromCodeset == "ascii"
                                         || fromCodeset == "iso88591"
                                         || fromCodeset == "latin1"))
        {
            convertDirection = LATIN1_TO_UTF8;
        }
        else {
            convertDirection = DO_NOTHING;
        }
    }
    String           fromCodeset;
    String           toCodeset;
    bool             validFlag;
    ConvertDirection convertDirection;
};
#endif // !LUCED_USE_ICONV
    

static String normalizeEncoding(String encoding)
{
    if (encoding == "") {
        encoding = System::getInstance()->getDefaultEncoding();
    }

    String e = encoding.toSubstitutedString("-", "").toLower();
    if (   e == "c" 
        || e == "posix"
        || e == "ascii"
        || e == "iso88591"
        || e == System::getInstance()->getCEncoding().toSubstitutedString("-", "").toLower())
    {
        encoding = "ISO-8859-1";
    }
    return encoding;
}

EncodingConverter::EncodingConverter(const String& fromCodeset, 
                                     const String& toCodeset)

    : fromCodeset(normalizeEncoding(fromCodeset)),
      toCodeset  (normalizeEncoding(toCodeset))
{}

bool EncodingConverter::isConvertingBetweenDifferentCodesets()
{
    return    fromCodeset.toSubstitutedString("-", "").toLower()
           !=   toCodeset.toSubstitutedString("-", "").toLower();
}

void EncodingConverter::convertInPlace(RawPtr<ByteBuffer> buffer)
{
    bool hasErrors       = false;
    bool hasInvalidBytes = false;
    
    LowLevelConverter lowLevelConverter(fromCodeset, toCodeset);
    
    if (!lowLevelConverter.isValid())
    {
        throw SystemException(String() << "Cannot convert from codeset " << fromCodeset
                                       << " to codeset " << toCodeset
                                       << ": " << strerror(errno));
    }
    long toPos   = 0;
    long fromPos = 0;
    long fromLength = buffer->getLength();

    long nextInsertSize = 1000000;
    
    if (2 * fromLength < nextInsertSize) {
        nextInsertSize = 2 * fromLength;
    }
    while (fromLength > 0)
    {
        const long insertSize = nextInsertSize;
        
        buffer->insertAmount(toPos, insertSize);
        fromPos += insertSize;
        
        const char*  fromPtr0      = (char*)    buffer->getAmount(fromPos, fromLength);
        const char*  fromPtr1      = fromPtr0;
        size_t       fromBytesLeft = fromLength;

        char*        toPtr0        = (char*)    buffer->getAmount(toPos, insertSize);
        char*        toPtr1        = toPtr0;
        size_t       outBytesLeft  = insertSize;
        
        LowLevelResult rslt = lowLevelConverter.convert(&fromPtr1, &fromBytesLeft,
                                                        &toPtr1,   &outBytesLeft);
        if (rslt == OUTPUT_BUFFER_TOO_SMALL)
        {
            if (outBytesLeft > 0) {
                nextInsertSize += outBytesLeft;
            }
        }
        else if (rslt == INVALID_SEQUENCE)
        {
            if (outBytesLeft > 0 && fromBytesLeft > 0)
            {
                *(toPtr1++) = *(fromPtr1++);
                --outBytesLeft;
                --fromBytesLeft;
                hasErrors       = true;
                hasInvalidBytes = true;
                lowLevelConverter.reset();
            }
            else {
                // should not happen
                buffer->removeAmount(0, buffer->getLength() - fromLength);
                throw SystemException(String() << "Error converting from codeset " << fromCodeset
                                               << " to codeset " << toCodeset
                                               << ": invalid byte sequence at position " 
                                               << (long)(fromPtr1 - fromPtr0 + fromPos - (toPos + insertSize)));
            }
        }
        else if (rslt == NON_REVERSIBLE_CONVERSIONS_OCCURRED)
        {
            hasErrors = true;
        }
        else if (rslt != CONVERSION_OK) 
        {
            // should not happen
            buffer->removeAmount(0, buffer->getLength() - fromLength);
            throw SystemException(String() << "Error converting from codeset " << fromCodeset
                                           << " to codeset " << toCodeset
                                           << " at position " << (long)(fromPtr1 - fromPtr0 + fromPos - (toPos + insertSize))
                                           << ": " << strerror(errno));
        }
        const long processedFromAmount = fromPtr1 - fromPtr0;
        const long processedToAmount   = toPtr1   - toPtr0;

        fromPos    += processedFromAmount;
        fromLength -= processedFromAmount;
        toPos      += processedToAmount;
        
        const long removeFromInserted = insertSize - processedToAmount;
        
        buffer->removeAmount(toPos, fromPos - toPos);
        fromPos = toPos;

        ASSERT(buffer->getLength() - fromPos == fromLength);
    }
    if (hasErrors) {
        if (hasInvalidBytes) {
            throw EncodingException(String() << "Error converting from codeset " << fromCodeset
                                             << " to codeset " << toCodeset
                                             << ": non-convertible bytes occurred.");
        
        } else {
            throw EncodingException(String() << "Error converting from codeset " << fromCodeset
                                             << " to codeset " << toCodeset
                                             << ": non-reversible conversions performed.");
        }
    }
}


void EncodingConverter::convertToFile(const ByteBuffer&  buffer, 
                                      const File&        file)
{
    bool hasErrors       = false;
    bool hasInvalidBytes = false;

    File::Writer::Ptr fileWriter = file.openForWriting();
    
    LowLevelConverter lowLevelConverter(fromCodeset, toCodeset);
    
    if (!lowLevelConverter.isValid())
    {
        throw SystemException(String() << "Cannot convert from codeset " << fromCodeset
                                       << " to codeset " << toCodeset
                                       << ": " << strerror(errno));
    }
    
    long fromLength = buffer.getLength();

    const char*   fromPtr0      = (char*) buffer.getAmount(0, fromLength);
    const char*   fromPtr1      = fromPtr0;
    size_t        fromBytesLeft = fromLength;
    
    ByteArray outBuffer;

    long nextOutBufferSize = 1000000;
    
    if (2 * fromLength < nextOutBufferSize) {
        nextOutBufferSize = 2 * fromLength;
    }
    while (fromBytesLeft > 0)
    {
        const long outBufferSize = nextOutBufferSize;
        
        outBuffer.increaseTo(outBufferSize);

        char*   toPtr0        = (char*)    outBuffer.getPtr(0);
        char*   toPtr1        = toPtr0;
        size_t  outBytesLeft  = outBufferSize;
        
        LowLevelResult rslt = lowLevelConverter.convert(&fromPtr1, &fromBytesLeft,
                                                        &toPtr1,   &outBytesLeft);
        if (rslt == OUTPUT_BUFFER_TOO_SMALL)
        {
            if (outBytesLeft > 0) {
                nextOutBufferSize += outBytesLeft;
            }
        }
        else if (rslt == INVALID_SEQUENCE)
        {
            if (outBytesLeft > 0 && fromBytesLeft > 0)
            {
                *(toPtr1++) = *(fromPtr1++);
                --outBytesLeft;
                --fromBytesLeft;
                hasErrors       = true;
                hasInvalidBytes = true;
                lowLevelConverter.reset();
            }
            else {
                // should not happen
                fileWriter->write(toPtr0, toPtr1 - toPtr0);
                fileWriter->write(fromPtr1, fromBytesLeft);
                throw SystemException(String() << "Error converting from codeset " << fromCodeset
                                               << " to codeset " << toCodeset
                                               << ": invalid byte sequence at position " 
                                               << (long)(fromPtr1 - fromPtr0));
            }
        }
        else if (rslt == NON_REVERSIBLE_CONVERSIONS_OCCURRED)
        {
            hasErrors = true;
        }
        else if (rslt != CONVERSION_OK) 
        {
            // should not happen
            fileWriter->write(toPtr0, toPtr1 - toPtr0);
            fileWriter->write(fromPtr1, fromBytesLeft);
            throw SystemException(String() << "Error converting from codeset " << fromCodeset
                                           << " to codeset " << toCodeset
                                           << " at position " << (long)(fromPtr1 - fromPtr0)
                                           << ": " << strerror(errno));
        }
        fileWriter->write(toPtr0, toPtr1 - toPtr0);
    }
    if (hasErrors) {
        if (hasInvalidBytes) {
            throw EncodingException(String() << "Error converting from codeset " << fromCodeset
                                             << " to codeset " << toCodeset
                                             << " while writing to file '" << file.getAbsoluteName()
                                             << "': non-convertible bytes occurred.");
        
        } else {
            throw EncodingException(String() << "Error converting from codeset " << fromCodeset
                                             << " to codeset " << toCodeset
                                             << " while writing to file '" << file.getAbsoluteName()
                                             << "': non-reversible conversions performed.");
        }
    }
}


String EncodingConverter::convertStringToString(const String& fromString)
{
    ByteBuffer buffer;
    buffer.appendString(fromString);
    convertInPlace(&buffer);
    return buffer.toString();
}

String EncodingConverter::convertLocaleToUtf8StringIgnoreErrors(const String& fromString)
{
    ByteBuffer buffer;
    buffer.appendString(fromString);
    try
    {
        EncodingConverter("", "UTF-8").convertInPlace(&buffer);
    }
    catch (EncodingException& ex)
    {}
    return buffer.toString();
}



String EncodingConverter::convertUtf8ToLocaleStringIgnoreErrors(const String& fromString)
{
    ByteBuffer buffer;
    buffer.appendString(fromString);
    try
    {
        EncodingConverter("UTF-8", "").convertInPlace(&buffer);
    }
    catch (EncodingException& ex)
    {}
    return buffer.toString();
}

