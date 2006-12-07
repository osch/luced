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

#include "debug.h"
#include "Regex.h"
#include "RegexException.h"

using namespace LucED;

const unsigned char* Regex::pcreCharTable = NULL;

Regex::Regex(const string& expr, CreateOptions createOptions)
{
    if (pcreCharTable == NULL) {
        pcreCharTable = pcre_maketables();
    }
    const char *errortext;
    int errorpos;

    re = pcre_compile(expr.c_str(), createOptions.getOptions(), &errortext, &errorpos, Regex::pcreCharTable);
    if (re == NULL) {
        throw RegexException(errortext, errorpos);
    }
    pcre_refcount(re, +1);
}


Regex::Regex(const ByteArray& expr, CreateOptions createOptions)
{
    const char *errortext;
    int errorpos;

    re = pcre_compile(expr.toCStr(), createOptions.getOptions(), &errortext, &errorpos, Regex::pcreCharTable);
    if (re == NULL) {
        throw RegexException(errortext, errorpos);
    }
    pcre_refcount(re, +1);
}


Regex::Regex(const Regex& src)
{
    re = src.re;
    pcre_refcount(re, +1);
}


Regex& Regex::operator=(const Regex& src)
{
    pcre *oldRe = re;
    re = src.re;
    if (re != NULL) {
        pcre_refcount(re, +1);
    }
    if (oldRe != NULL) {
        int refCount = pcre_refcount(oldRe, -1);
        if (refCount == 0) {
            pcre_free(oldRe);
        }
    }
}


Regex::~Regex()
{
    if (re != NULL) {
        int refCount = pcre_refcount(re, -1);
        if (refCount == 0) {
            pcre_free(re);
            re = NULL;
        }
    }
}


int Regex::getStringNumber(const string& substringName) const
{
    ASSERT(re != NULL);
    int rslt = pcre_get_stringnumber(re, substringName.c_str());
    if (rslt == PCRE_ERROR_NOSUBSTRING) {
        throw RegexException("named substring '" + substringName + "' not found");
    }
    return rslt;
}

int Regex::getStringNumber(const ByteArray& substringName) const
{
    ASSERT(re != NULL);
    int rslt = pcre_get_stringnumber(re, substringName.toCStr());
    if (rslt == PCRE_ERROR_NOSUBSTRING) {
        throw RegexException("named substring '" + substringName.toString() + "' not found");
    }
    return rslt;
}

