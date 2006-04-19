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

#include "RegexException.h"

using namespace LucED;

RegexException::RegexException(const char* message, int position)
        : whatString(message), position(position)
{}

RegexException::RegexException(const string& message)
        : message(message), position(-1)
{
    whatString = "RegexException";
}

const char* RegexException::what()
{
    return whatString;
}

string RegexException::getMessage()
{
    if (message.length() > 0) {
        return message;
    } else {
        return string(whatString);
    }
}

