/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2012 Oliver Schmidt, oliver at luced dot de
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

#ifndef LUA_STACK_TRACE_HPP
#define LUA_STACK_TRACE_HPP

#include "HeapObject.hpp"
#include "OwningPtr.hpp"
#include "String.hpp"
#include "ByteBuffer.hpp"
#include "LuaAccess.hpp"
#include "Nullable.hpp"

namespace LucED
{

class LuaStackTrace : public HeapObject
{
public:
    typedef OwningPtr<LuaStackTrace> Ptr;
    
    static Ptr create(const LuaAccess& luaAccess,
                      int              callLevel = 0)
    {
        Ptr rslt(new LuaStackTrace());
        rslt->fillStackTrace(luaAccess, callLevel);
        return rslt;
    }
    static Ptr createParsingScriptError(const LuaAccess& luaAccess,
                                        int              lineNumber,
                                        const char*      parsedScriptBegin,
                                        long             parsedScriptLength)
    {
        Ptr rslt(new LuaStackTrace());
        rslt->appendEntry(Entry::createScriptBytesEntry((const byte*)parsedScriptBegin,
                                                                     parsedScriptLength,
                                                        lineNumber));
        rslt->fillStackTrace(luaAccess);
        return rslt;
    }

    static Ptr createParsingFileError(const LuaAccess& luaAccess,
                                      int              lineNumber,
                                      const String&    fileName)
    {
        Ptr rslt(new LuaStackTrace());
        rslt->appendEntry(Entry::createFileEntry(fileName,
                                                 lineNumber));
        rslt->fillStackTrace(luaAccess);
        return rslt;
    }
    
    class Entry : public HeapObject
    {
    public:
        typedef OwningPtr<Entry> Ptr;

        bool hasFileName()  const {
            return fileName.isValid();
        }
        String getFileName() const {
            return fileName.get();
        }
        bool isBuiltinFile() const {
            return fileName.isValid() && isBuiltinFileFlag;
        }
        bool hasScriptBytes() const {
            return !fileName.isValid();
        }
        RawPtr<ByteBuffer> getPtrToScriptBytes() {
            return &scriptBytes;
        }
        int getLineNumber() const {
            return lineNumber;
        }
        int getEntryIndex() const {
            return entryIndex;
        }
        bool refersToSameFileLocation(Entry::Ptr rhs) const {
            return    this->hasFileName() 
                   &&  rhs->hasFileName()
                   && (this->isBuiltinFile() == rhs->isBuiltinFile())
                   && this->getFileName() == rhs->getFileName()
                   && this->getLineNumber() == rhs->getLineNumber();
        }
        String toString() const;
        
    private:
        friend class LuaStackTrace;
        
        // prepend fileName with "@" for external file, with "$" for builtin file
        static Ptr createFileEntry(const String& fileName, int lineNumber);

        static Ptr createScriptBytesEntry(const byte* scriptBegin, long scriptLength, int lineNumber) {
            return Ptr(new Entry(scriptBegin, scriptLength, lineNumber));
        }
        void setEntryIndex(int entryIndex) {
            this->entryIndex = entryIndex;
        }

    private:
        Entry(const String fileName, bool isBuiltinFileFlag, int lineNumber)
            : entryIndex(-1),
              fileName(fileName),
              isBuiltinFileFlag(isBuiltinFileFlag),
              lineNumber(lineNumber)
        {}
        Entry(const byte* scriptBegin, long scriptLength, int lineNumber)
            : entryIndex(-1),
              isBuiltinFileFlag(false),
              lineNumber(lineNumber)
        {
            scriptBytes.append(scriptBegin, scriptLength);
        }
        int              entryIndex;
        Nullable<String> fileName;
        bool             isBuiltinFileFlag;
        int              lineNumber;
        ByteBuffer       scriptBytes;
    };
    
    int getEntryCount() const {
        return entries.getLength();
    }
    Entry::Ptr getEntry(int i) const {
        return entries[i];
    }
    Entry::Ptr findFirstExternalFileEntry() const;
    Entry::Ptr findFirstScriptBytesEntry() const;
    Entry::Ptr findFirstBuiltinFileEntry() const;
    Entry::Ptr findFirstFileEntry() const;
    
    String toString() const;
    
private:
    LuaStackTrace()
    {}
    
    void appendEntry(Entry::Ptr e) {
        e->setEntryIndex(entries.getLength());
        entries.append(e);
    }

    void fillStackTrace(LuaAccess luaAccess,
                        int       callLevel = 0);

    ObjectArray<Entry::Ptr> entries;
};

} // namespace LucED

#endif // LUA_STACK_TRACE_HPP
