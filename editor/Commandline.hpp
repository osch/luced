/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2008 Oliver Schmidt, oliver at luced dot de
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

#ifndef COMMANDLINE_HPP
#define COMMANDLINE_HPP

#include "HeapObjectArray.hpp"
#include "String.hpp"

namespace LucED
{

class Commandline : public HeapObjectArray<String>
{
public:

    typedef OwningPtr<Commandline> Ptr;
    
    static Ptr create() {
        return Ptr(new Commandline());
    }
    
    static Ptr create(int argc, char** argv) {
        return Ptr(new Commandline(argc, argv));
    }
    
    static Ptr createFromQuotedString(const String& commandline);
    
    String toQuotedString() const;
    
    const char* const* getArgvPtr();
    
private:
    Commandline()
    {}
    
    Commandline(int argc, char** argv)
    {
        // ignore executable name argv[0]
        for (int i = 1; i < argc; ++i) {
            this->append(String(argv[i]));
        }
    }
    MemArray<const char*> argvArray;
};

} // namespace LucED

#endif // COMMANDLINE_HPP

