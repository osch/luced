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

#ifndef FONT_HANDLE_HPP
#define FONT_HANDLE_HPP

#include "headers.hpp"

namespace LucED
{

class FontHandle
{
#ifdef WIN32_GUI

public:
    FontHandle()
        : hFont()
    {}
    
    explicit FontHandle(HFONT hFont)
        : hFont(hFont)
    {}
    
    operator HFONT() const {
        return hFont;
    }
private:
    HFONT hFont;

#else

public:
    FontHandle()
        : fid()
    {}
    
    explicit FontHandle(Font fid)
        : fid(fid)
    {}
    
    operator Font() const {
        return fid;
    }
private:
    Font fid;

#endif
};

} // namespace LucED

#endif // FONT_HANDLE_HPP
