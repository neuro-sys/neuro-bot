#ifndef __GLOBAL_H
#define __GLOBAL_H

#ifdef __WIN32__
    #define NO_COLOR_TERM 1
#endif // __WIN32__

#ifndef NO_COLOR_TERM
    #define CLEAR   "\033[0m"
    #define RED     "\033[1;31m"
    #define GREEN   "\033[1;32m"
    #define YELLOW  "\033[1;33m"
    #define BLUE    "\033[1;34m"
    #define MAGENTA "\033[1;35m"
    #define CYAN    "\033[1;36m"
    #define BG_RED  "\033[1;41m"
#else
    #define CLEAR   ""
    #define RED     ""
    #define GREEN   ""
    #define YELLOW  ""
    #define BLUE    ""
    #define MAGENTA ""
    #define CYAN    ""
    #define BG_RED  ""
#endif // NO_COLOR_TERM


//    fprintf(stderr, MAGENTA "%25s" ":" "%4d" ":" "%25s" ":" CLEAR, __FILE__, __LINE__, __FUNCTION__);

#define debug(args...) \
    fprintf(stderr, args);

#define debug_ex(args...) fprintf(stderr, RED args CLEAR);

#ifdef __WIN32__

  #define __func__ __FUNCTION__

  #define snprintf _snprintf
  #define getcwd _getcwd
  #define alloca _alloca

#endif

#endif
