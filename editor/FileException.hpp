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

#ifndef FILEEXCEPTION_H
#define FILEEXCEPTION_H

#include "BaseException.hpp"

namespace LucED {

class FileException : public BaseException
{
public:
    FileException(int errnum, String message)
        : errnum(errnum),
          BaseException(message)
    {}
    int getErrno() const {
        return errnum;
    }
    virtual const char *what();
private:
    int errnum;
};

} // namespace LucED

#endif // FILEEXCEPTION_H
