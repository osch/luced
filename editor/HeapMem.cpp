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

#include <stdio.h>

#include "HeapMem.h"

using namespace LucED;

void HeapMem::increase(long plusAmount, long blockSize)
{
    long new_cap;
    byte *ptr;
    
    if (buffer == NULL) {
        new_cap = plusAmount;
        ptr     = (byte*) malloc(new_cap);
    } else {
        if (capacity >= plusAmount) {
            new_cap = capacity * 2;
            ptr = (byte*) realloc(buffer, new_cap);
            while (ptr == NULL) {
                new_cap = (((new_cap - capacity) / 2) / blockSize) 
                        * blockSize;
                if (new_cap < capacity + plusAmount) {
                    break;
                }
                ptr = (byte*) realloc(buffer, new_cap);
            }
        } else {
            new_cap = capacity + plusAmount;
            ptr = (byte*) realloc(buffer, new_cap);
        }
    }
    if (ptr == NULL) {
        fprintf(stderr, "Out of Memory!\n");
        abort();
    }
    buffer   = ptr;
    capacity = new_cap;
}

