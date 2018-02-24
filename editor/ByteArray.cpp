#include "ByteArray.hpp"
#include "String.hpp"

using namespace LucED;

ByteArray& ByteArray::appendString(const String& s) 
{
    append((const byte*) s.toCString(), s.getLength());
    return *this;
}

String ByteArray::toString() const 
{
    return String( (const char*) getPtr(0), getLength() );
}
