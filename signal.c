#include <stdio.h>
#include <stdlib.h>

#include "signal.h"
#include "utils.h"

void tf_sighdlr(int signo)
{
    if (signo == SIGINT)
    {
        // Clear the terminal
        tf_clear_term();
        // And set the cursor color back to normal
        printf("%s", tf_color_from_enum(TF_NORMAL));
        fflush(stdout);
    }
    exit(signo);
}
