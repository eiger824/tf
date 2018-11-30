#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <assert.h>
#include <sys/ioctl.h>

#include "defs.h"
#include "utils.h"
#include "dbg.h"

void tf_usage(char* progname)
{
    printf("USAGE: %s [OPTIONS]\n\n", basename(progname));
    printf("  -c          Clear the current terminal and exit\n");
    printf("  -d <dev>    The device to be used\n");
    printf("  -D <level>  Enable \"level\" amount of debug\n");
    printf("  -h          Show this help and exit\n");
    printf("  -r          Fill the current terminal with random ASCII characters of random colors\n");
    printf("  -v          Show version information and exit\n");
}

void tf_print_info(char* progname)
{
    printf("%s - version %d.%d, revision r%d\nAuthor: Santiago Pagola - Nov. 2018\n",
            basename(progname), TF_VERSION_MAJOR, TF_VERSION_MINOR, TF_REVISION);
}

int main(int argc, char* argv[])
{
    int c;
    int clear = 0;
    int random = 0;
    struct term_size ts;
    char tf_program_name[100];
    strcpy(tf_program_name, argv[0]);
    char* tty_dev = "/dev/stdout";  /* Use standard output as default */

    /* Parse command line arguments */
    while ((c = getopt(argc, argv, "cd:D:hrv")) != -1)
    {
        switch (c)
        {
            case 'c':
                clear = 1;
                break;
            case 'd':
                tty_dev = optarg;
                break;
            case 'D':
                tf_set_debug_level(atoi(optarg));
                break;
            case 'h':
                tf_usage(tf_program_name);
                exit(TF_SUCCESS);
            case 'r':
                random = 1;
                break;
            case 'v':
                tf_print_info(tf_program_name);
                exit(TF_SUCCESS);
        }
    }

    // Get the terminal size of the specified device
    ts = tf_get_term_size(tty_dev);

    tf_dbg(1, "Dealing with a terminal window of size %zu x %zu.\n", ts.rows, ts.cols);

    if (clear == 1)
    {
        tf_clear_term();
        exit (TF_SUCCESS);
    }
    if (random == 1)
    {
        tf_fill_random_term(ts);
        exit (TF_SUCCESS);
    }
    /* Do something */
    tf_paint_text(ts, "Hello");

    return 0;
}

