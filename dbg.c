#include <string.h>
#include <stdarg.h>

#include "dbg.h"

static size_t tf_debug_level  = 0;

void tf_set_debug_level(size_t level)
{
    // Always print this if we are setting some debug
    printf("Setting debug level: %zu\n", level);
    tf_debug_level = level;
}

void tf_dbg(size_t level, const char* fmt, ...)
{
    if (level > tf_debug_level)
        return;
    va_list args;
    va_start(args, fmt);
    char tmp[1024];
    char out[1024];
    vsprintf(tmp, fmt, args);
    strcpy(out, "[dbg] ");
    strcat(out, tmp);
    fprintf(stderr, "%s", out);
    va_end(args);
}

