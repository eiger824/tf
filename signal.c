#include <stdio.h>
#include <stdlib.h>

#include "signal.h"
#include "utils.h"

void tf_sighdlr(int signo)
{
    if (signo == SIGINT)
    {
        // If there's an animation going on, release memory and stop stuff
        if (tf_is_animation_in_progress())
        {
            free(tf_columns);
        }
        // Clear the terminal
        tf_clear_term(g_ts);
        // And set the cursor color back to normal
        tf_write_dev(g_ts.fd, "%s", tf_color_from_enum(TF_NORMAL));
    }
    exit(signo);
}
