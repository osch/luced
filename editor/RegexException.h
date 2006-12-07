/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, oliver at luced dot de
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

#ifndef REGEXEXCEPTION_H
#define REGEXEXCEPTION_H

#include <exception>
#include <string>

namespace LucED {

using std::exception;
using std::string;

class RegexException : public exception
{
public:
    RegexException(const char* message, int position);
    RegexException(const string& message);
    virtual ~RegexException() throw() {}
    virtual const char *what();
    string getMessage();
    int getPosition() const {
        return position;
    }
private:
    const char* whatString;
    string message;
    int position;
};

} // namespace LucED

#endif // REGEXEXCEPTION_H
