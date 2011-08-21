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

#ifndef STACK_TRACE_HPP
#define STACK_TRACE_HPP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

namespace LucED
{

class StackTrace
{
public:
    StackTrace();

    StackTrace(const StackTrace& rhs) {
        if (this != &rhs && rhs.size > 0) {
            this->array = (void**) malloc(rhs.size * sizeof(void*));
            if (this->array != NULL) {
                this->size = rhs.size;
                memcpy(this->array, rhs.array, rhs.size * sizeof(void*));
            } else {
                this->size = 0;
            }
        } else {
            this->array = NULL;
            this->size = 0;
        }
    }
    StackTrace& operator=(const StackTrace& rhs) {
        if (this != &rhs) {
            if (rhs.size > 0) {
                this->array = (void**) malloc(rhs.size * sizeof(void*));
                if (this->array != NULL) {
                    this->size = rhs.size;
                    memcpy(this->array, rhs.array, rhs.size * sizeof(void*));
                } else {
                    this->size = 0;
                }
            } else {
                this->array = NULL;
                this->size = 0;
            }
        }
        return *this;
    }
    ~StackTrace() throw() {
        if (array != NULL) {
            free(array);
        }
    }
    std::string toString() const;
    
    void print(FILE* fprintfOutput = stdout) const;
    
    static void printCurrent(FILE* fprintfOutput = stdout);
    static std::string getCurrentAsString();

private:
    void** array;
    int    size;
};

} // namespace LucED

#endif // STACK_TRACE_HPP

