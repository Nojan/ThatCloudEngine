#pragma once
#include <stdarg.h>
#include <stdio.h>

#if 0
inline int snprintf(char * s, size_t n, const char * format, ...) //not availlable on vs2013 :(
{
    int ret;

    va_list myargs;
    va_start(myargs, format);
    ret = vsprintf_s(s, n, format, myargs);
    va_end(myargs);

    return ret;
}
#endif