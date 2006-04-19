/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, osch@luced.de
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

#ifndef FILE_H
#define FILE_H

#include <string>

#include "NonCopyable.h"
#include "ByteBuffer.h"

namespace LucED {

using std::string;

class File : public NonCopyable
{
public:

    File(const string& fileName)
        : name(fileName)
    {}
    
    string getAbsoluteFileName();

    void loadInto(ByteBuffer& buffer);
    
private:
    const string name;
};

} // namespace LucED

#endif // FILE_H

