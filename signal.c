#include <stdio.h>
#include <stdlib.h>

#include "signal.h"
#include "utils.h"

void tf_sighdlr(int signo)
{
    if (signo == SIGINT)
    {
        // Clear the terminal
        tf_clear_term(g_ts);
        // And set the cursor color back to normal
        tf_write_dev(g_ts.fd, "%s", tf_color_from_enum(TF_NORMAL));
    }
    exit(signo);
}
