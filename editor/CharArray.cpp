#include "CharArray.hpp"
#include "String.hpp"

using namespace LucED;

CharArray& CharArray::appendString(const String& s)
{
    append((const char*) s.toCString(), s.getLength());
    return *this;
}

String CharArray::toString() const 
{
    return String((const char*) getPtr(0), getLength());
}
