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

using namespace LucED;

namespace // anonymous
{

class IconvHandle : public NonCopyable
{
public:
    IconvHandle(const String& fromCodeset, 
                const String& toCodeset) 

        : value(iconv_open(toCodeset.toCString(),
                           fromCodeset.toCString()))
    {}
    ~IconvHandle() {
        if (isValid()) {
            iconv_close(value);
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
};

} // anonymous namespace


void EncodingConverter::convertInPlace(RawPtr<ByteBuffer> buffer, 
                                       const String&      fromCodeset, 
                                       const String&      toCodeset)
{
    bool hasErrors = false;
    
    IconvHandle iconvHandle(fromCodeset, toCodeset);
    
    if (!iconvHandle.isValid())
    {
        throw SystemException(String() << "Cannot convert from codeset '" << fromCodeset
                                       << "' to codeset '" << toCodeset
                                       << "': " << strerror(errno));
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
        
        char*   fromPtr0      = (char*)    buffer->getAmount(fromPos, fromLength);
        char*   fromPtr1      = fromPtr0;
        size_t  fromBytesLeft = fromLength;

        char*   toPtr0        = (char*)    buffer->getAmount(toPos, insertSize);
        char*   toPtr1        = toPtr0;
        size_t  outBytesLeft  = insertSize;
        
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
            
                buffer->removeAmount(0, buffer->getLength() - fromLength);
                throw EncodingException(String() << "Error converting from codeset '" << fromCodeset
                                                 << "' to codeset '" << toCodeset
                                                 << "': invalid byte sequence at position " 
                                                 << (long)(fromPtr1 - fromPtr0 + fromPos - (toPos + insertSize)));
            }
            else {
                buffer->removeAmount(0, buffer->getLength() - fromLength);
                throw SystemException(String() << "Error converting from codeset '" << fromCodeset
                                               << "' to codeset '" << toCodeset
                                               << "': " << strerror(errno));
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
}

void EncodingConverter::convertToFile(const ByteBuffer&  buffer, 
                                      const String&      fromCodeset, 
                                      const String&      toCodeset, 
                                      const File&        file)
{
    bool hasErrors = false;

    File::Writer::Ptr fileWriter = file.openForWriting();
    
    IconvHandle iconvHandle(fromCodeset, toCodeset);
    
    if (!iconvHandle.isValid())
    {
        throw SystemException(String() << "Cannot convert from codeset '" << fromCodeset
                                       << "' to codeset '" << toCodeset
                                       << "': " << strerror(errno));
    }
    
    long fromLength = buffer.getLength();

    char*   fromPtr0      = (char*) buffer.getAmount(0, fromLength);
    char*   fromPtr1      = fromPtr0;
    size_t  fromBytesLeft = fromLength;
    
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
            
                throw EncodingException(String() << "Error converting from codeset '" << fromCodeset
                                                 << "' to codeset '" << toCodeset
                                                 << "': invalid byte sequence at position " 
                                                 << (long)(fromPtr1 - fromPtr0));
            }
            else {
                throw SystemException(String() << "Error converting from codeset '" << fromCodeset
                                               << "' to codeset '" << toCodeset
                                               << "': " << strerror(errno));
            }
        } else {
            // The input byte sequence has been entirely converted, i.e. *inbytesleft has 
            // gone down to 0. In this case iconv returns the number of non-reversible 
            // conversions performed during this call.
            
            hasErrors = hasErrors || (s > 0);
        }
        const long processedToAmount   = toPtr1   - toPtr0;
        
        fileWriter->write(toPtr0, processedToAmount);
    }
}
