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
    printf("  -d <level>  Enable \"level\" amount of debug\n");
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
    char tf_program_name[100];
    strcpy(tf_program_name, argv[0]);

    struct term_size ts = tf_get_term_size();

    /* Parse command line arguments */
    while ((c = getopt(argc, argv, "cd:hrv")) != -1)
    {
        switch (c)
        {
            case 'c':
                tf_clear_term();
                exit(TF_SUCCESS);
            case 'd':
                tf_set_debug_level(atoi(optarg));
                break;
            case 'h':
                tf_usage(tf_program_name);
                exit(TF_SUCCESS);
            case 'r':
                tf_fill_random_term(ts);
                exit(TF_SUCCESS);
            case 'v':
                tf_print_info(tf_program_name);
                exit(TF_SUCCESS);
        }
    }

    tf_dbg(1, "Dealing with a terminal window of size %zu x %zu.\n", ts.rows, ts.cols);


    /* Do something */
    tf_paint_text(ts, "Hello");

    return 0;
}

