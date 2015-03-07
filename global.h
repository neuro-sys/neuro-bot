#ifndef __GLOBAL_H
#define __GLOBAL_H

#define CLEAR   "\033[0m"
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN    "\033[1;36m"

#define BG_RED  "\033[1;41m"

#define BIT_ON(WORD, BIT)   WORD |= 1 << BIT
#define BIT_OFF(WORD, BIT)  WORD &= ~(1 << BIT)

#define IS_BIT_ON(WORD, BIT) WORD & (1 << BIT)

/* #define USE_PYTHON_MODULES */
#define debug(args...) \
    fprintf(stderr, MAGENTA "%25s" ":" "%4d" ":" "%25s" ":" CLEAR, __FILE__, __LINE__, __FUNCTION__); \
    fprintf(stderr, args);

#define debug_ex(args...) fprintf(stderr, RED args CLEAR);

#ifdef _WIN32

  #pragma warning(disable : 4996)

  #define __func__ __FUNCTION__
  
  #define snprintf _snprintf
  #define getcwd _getcwd
  #define alloca _alloca

#endif

#endif
