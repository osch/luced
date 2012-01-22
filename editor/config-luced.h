/* This file is included at the end of config.h */



/* usage of libXpm for displaying window icons */

#if !defined(LUCED_USE_XPM)
#  if HAVE_X11_XPM_H && HAVE_X11_XUTIL_H && !DISABLE_XPM
#    define LUCED_USE_XPM 1
#  else
#    define LUCED_USE_XPM 0
#  endif
#endif



/* usage of iconv for transforming character encodings */

#if !defined(LUCED_USE_ICONV)
#  if HAVE_ICONV && !DISABLE_ICONV
#    define LUCED_USE_ICONV 1
#  else
#    define LUCED_USE_ICONV 0
#  endif
#endif



/* workaround for cygwin fork problems under Windows */

#if !defined(LUCED_USE_CYGWIN_FORK_WORKAROUND)
#  if (defined(__CYGWIN__) || defined(__CYGWIN32__)) && HAVE_WINDOWS_H && HAVE_SYS_CYGWIN_H && !DISABLE_CYGWIN_FORK_WORKAROUND
#    define LUCED_USE_CYGWIN_FORK_WORKAROUND 1
#  else
#    define LUCED_USE_CYGWIN_FORK_WORKAROUND 0
#  endif
#endif


/* usage of multi threading */

#if !defined(LUCED_USE_PTHREAD)
#  if HAVE_PTHREAD_H && (!DISABLE_MULTI_THREAD || LUCED_USE_CYGWIN_FORK_WORKAROUND)
#    define LUCED_USE_PTHREAD 1
#  else
#    define LUCED_USE_PTHREAD 0
#  endif
#endif

#if !defined(LUCED_USE_MULTI_THREAD)
#  if HAVE_PTHREAD_H && (!DISABLE_MULTI_THREAD || LUCED_USE_CYGWIN_FORK_WORKAROUND)
#    define LUCED_USE_MULTI_THREAD 1
#  else
#    define LUCED_USE_MULTI_THREAD 0
#  endif
#endif





/* usage of xkblib */

#if !defined(LUCED_USE_XKBLIB)
#  if HAVE_X11_XKBLIB_H
#    define LUCED_USE_XKBLIB 1
#  else
#    define LUCED_USE_XKBLIB 0
#  endif
#endif



/* usage of c++ hashmap */

#if !defined(LUCED_USE_STD_UNORDERED_MAP)              \
 && !defined(LUCED_USE_TR1_UNORDERED_MAP)              \
 && !defined(LUCED_USE_EXT_HASH_MAP_UNDER_STD)         \
 && !defined(LUCED_USE_EXT_HASH_MAP_UNDER_GNU_CXX)     \
 && !defined(LUCED_USE_STD_MAP)

    #if HAVE_UNORDERED_MAP 
        #define                                   LUCED_USE_STD_UNORDERED_MAP           1

    #elif    HAVE_TR1_UNORDERED_MAP \
          && !UNORDERED_MAP_UNDER_TR1_IS_BROKEN
        #define                                   LUCED_USE_TR1_UNORDERED_MAP           1
    
    #elif HAVE_EXT_HASH_MAP \
       && HASH_MAP_UNDER_STD
        #define                                   LUCED_USE_EXT_HASH_MAP_UNDER_STD      1
    
    #elif    HAVE_EXT_HASH_MAP \
          && HASH_MAP_UNDER_GNU_CXX
        #define                                   LUCED_USE_EXT_HASH_MAP_UNDER_GNU_CXX  1
    
    #else
        #define                                   LUCED_USE_STD_MAP                     1
    #endif

#endif


