#include <stdlib.h>
#include <assert.h>

#include "colors.h"
#include "defs.h"

static const char* tf_colors[] = {
    "\x1B[0m",            /*  TF_NORMAL  */
    "\x1B[31m",           /*  TF_RED     */
    "\x1B[32m",           /*  TF_GREEN   */
    "\x1B[33m",           /*  TF_YELLOW  */
    "\x1B[34m",           /*  TF_BLUE    */
    "\x1B[35m",           /*  TF_MAGENTA */
    "\x1B[36m",           /*  TF_CYAN    */
    "\x1B[37m"            /*  TF_WHITE   */
};

const char* tf_color_from_enum(enum tf_color_type ct)
{
    if (ct == TF_RANDOM)
    {
        // Fot his special case, generate a random color
        ct = rand() % (TF_WHITE - TF_NORMAL + 1) + TF_NORMAL;
    }
    assert((int)ct >= 0 && (int)ct < TF_NUMCOLORS);
    return tf_colors[ct];
}

const enum tf_color_type tf_color_from_str(const char *c)
{
    if (!strcmp(c, "red"))
        return TF_RED;
    else if (!strcmp(c, "green"))
        return TF_GREEN;
    else if (!strcmp(c, "yellow"))
        return TF_YELLOW;
    else if (!strcmp(c, "blue"))
        return TF_BLUE;
    else if (!strcmp(c, "magenta"))
        return TF_MAGENTA;
    else if (!strcmp(c, "cyan"))
        return TF_CYAN;
    else if (!strcmp(c, "white"))
        return TF_WHITE;
    else if (!strcmp(c, "multicolor"))
        return TF_RANDOM;
    else
        return TF_INVALID;
}
