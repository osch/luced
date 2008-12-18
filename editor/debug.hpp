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

#ifndef DEBUG_INCLUDED
#define DEBUG_INCLUDED

#include <stdio.h>
#include <stdlib.h>

/*     #include <execinfo.h>
*/
     /* Obtain a backtrace and print it to `stdout'. */
/*     static inline void
     print_trace (void)
     {
       void *array[10];
       size_t size;
       char **strings;
       size_t i;

       size = backtrace (array, 10);
       strings = backtrace_symbols (array, size);

       printf ("Obtained %ld stack frames.\n", (long) size);

       for (i = 0; i < size; i++)
          printf ("%s\n", strings[i]);

       free (strings);
     }
*/

#ifndef DEBUG
#   define NDEBUG
#endif

#ifdef NDEBUG
#   define ASSERT(a)
#else // !NDEBUG
#   define ASSERT(a) { \
        if (!(a)) { \
            fprintf(stderr, "Assertion {" #a \
                    "} failed in " __FILE__ ":%d\n", __LINE__);\
            abort(); \
        } \
    }
#endif // !NDEBUG



#endif // DEBUG_INCLUDED
