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

#include <iconv.h>
#include <errno.h>
#include <string.h>

#include "EncodingConverter.hpp"
#include "SystemException.hpp"
#include "EncodingException.hpp"
#include "ByteArray.hpp"
#include "System.hpp"

using namespace LucED;

namespace // anonymous
{

class IconvHandle : public NonCopyable
{
public:
    IconvHandle(const String& fromCodeset, 
                const String& toCodeset) 

        : value(iconv_open(toCodeset.toCString(),
                           fromCodeset.toCString())),
          fromCodeset(fromCodeset),
          toCodeset(toCodeset)
    {}
    ~IconvHandle() {
        if (isValid()) {
            iconv_close(value);
        }
    }
    void reset() {
        if (isValid()) {
            iconv_close(value);
            value = iconv_open(toCodeset.toCString(),
                               fromCodeset.toCString());
        }
    }
    bool isValid() const {
        return value != (iconv_t)(-1);
    }
    operator iconv_t() const {
        return value;
    }
private:
    iconv_t value;
    String fromCodeset;
    String toCodeset;
};

String normalizeEncoding(String encoding)
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

} // anonymous namespace

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
    
    IconvHandle iconvHandle(fromCodeset, toCodeset);
    
    if (!iconvHandle.isValid())
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
        
        size_t s = iconv(iconvHandle, &fromPtr1, &fromBytesLeft,
                                      &toPtr1,   &outBytesLeft);
        if (s == (size_t)(-1)) {
            if (errno == E2BIG) {
                // The output buffer has no more room for the next converted character. 
                // In this case it sets errno to E2BIG and returns (size_t)(-1).
            
                if (outBytesLeft > 0) {
                    nextInsertSize += outBytesLeft;
                }
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
            
                if (outBytesLeft > 0 && fromBytesLeft > 0)
                {
                    *(toPtr1++) = *(fromPtr1++);
                    --outBytesLeft;
                    --fromBytesLeft;
                    hasErrors       = true;
                    hasInvalidBytes = true;
                    iconvHandle.reset();
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
            else {
                // should not happen
                buffer->removeAmount(0, buffer->getLength() - fromLength);
                throw SystemException(String() << "Error converting from codeset " << fromCodeset
                                               << " to codeset " << toCodeset
                                               << " at poisition " << (long)(fromPtr1 - fromPtr0 + fromPos - (toPos + insertSize))
                                               << ": " << strerror(errno));
            }
        } else {
            // The input byte sequence has been entirely converted, i.e. *inbytesleft has 
            // gone down to 0. In this case iconv returns the number of non-reversible 
            // conversions performed during this call.
            
            hasErrors = hasErrors || (s > 0);
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
    
    IconvHandle iconvHandle(fromCodeset, toCodeset);
    
    if (!iconvHandle.isValid())
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
        
        size_t s = iconv(iconvHandle, &fromPtr1, &fromBytesLeft,
                                      &toPtr1,   &outBytesLeft);
        if (s == (size_t)(-1)) {
            if (errno == E2BIG) {
                // The output buffer has no more room for the next converted character. 
                // In this case it sets errno to E2BIG and returns (size_t)(-1).
            
                if (outBytesLeft > 0) {
                    nextOutBufferSize += outBytesLeft;
                }
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
            
                if (outBytesLeft > 0 && fromBytesLeft > 0)
                {
                    *(toPtr1++) = *(fromPtr1++);
                    --outBytesLeft;
                    --fromBytesLeft;
                    hasErrors       = true;
                    hasInvalidBytes = true;
                    iconvHandle.reset();
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
            else {
                // should not happen
                fileWriter->write(toPtr0, toPtr1 - toPtr0);
                fileWriter->write(fromPtr1, fromBytesLeft);
                throw SystemException(String() << "Error converting from codeset " << fromCodeset
                                               << " to codeset " << toCodeset
                                               << " at poisition " << (long)(fromPtr1 - fromPtr0)
                                               << ": " << strerror(errno));
            }
        } else {
            // The input byte sequence has been entirely converted, i.e. *inbytesleft has 
            // gone down to 0. In this case iconv returns the number of non-reversible 
            // conversions performed during this call.
            
            hasErrors = hasErrors || (s > 0);
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
