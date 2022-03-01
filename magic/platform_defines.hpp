#if (defined(__WIN32) or defined(_WIN32)) and !defined(__WIN32__)
#define __WIN32__
#endif

#if defined(__unix) and !defined (__unix__)
#define __unix__
#endif

#if defined(__MACH__) and !defined(__APPLE__)
#define __APPLE__
#endif

// Linux plays nicely with __linux__
