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
            tf_set_stop_thread_cond();
        }
    }
}
