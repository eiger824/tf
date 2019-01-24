#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <string.h>
#include <getopt.h>
#include <assert.h>
#include <sys/ioctl.h>

#include "defs.h"
#include "utils.h"
#include "dbg.h"
#include "signal.h"

static struct option long_options[] = {
    {"animation", no_argument      , 0, 'a'},
    {"clear"    , no_argument      , 0, 'c'},
    {"color"    , required_argument, 0, 'C'},
    {"device"   , required_argument, 0, 'd'},
    {"debug"    , required_argument, 0, 'D'},
    {"help"     , no_argument      , 0, 'h'},
    {"rain"     , no_argument      , 0, 'r'},
    {"text"     , required_argument, 0, 't'},
    {"delay"    , required_argument, 0, 'T'},
    {"version"  , no_argument      , 0, 'v'},
    {0          , 0                , 0,  0 }
};

void tf_usage(char* progname)
{
    printf(
            "USAGE: %s [OPTIONS]\n\n"
            "  -a, --animation      Show a live animation, like a multithreaded vertical rain of characters\n"
            "  -c, --clear          Clear the current terminal and exit\n"
            "  -C, --color  <color> Use \"color\" when printing characters. Allowed values: red, green, yellow,\n"
            "                      blue, magenta, cyan, white, multicolor. Defaults to: multicolor\n"
            "  -d, --device <dev>   The device to be used. By default, the tty device attached to the current\n"
            "                      standard output is used, i.e., /dev/stdout. \"dev\" **must** be a tty.\n"
            "  -D, --debug  <level> Enable \"level\" amount of debug\n"
            "  -h, --help           Show this help and exit\n"
            "  -r, --rain           Fill the current terminal with random ASCII characters of random colors\n"
            "  -t, --text   <text>  Write \"text\" to the terminal [not yet implemented]\n"
            "  -T, --delay  <ms>    Update screen every <ms> milliseconds\n"
            "  -v, --verbose        Show version information and exit\n"
            , basename(progname)
          );
}

void tf_print_info(char* progname)
{
    printf("%s - version %d.%d, revision r%d\nAuthor: Santiago Pagola - Nov. 2018\n",
            basename(progname), TF_VERSION_MAJOR, TF_VERSION_MINOR, TF_REVISION);
}

int main(int argc, char* argv[])
{
    int c, option_index = 0;
    int clear = 0;
    int random = 0;
    int animated = 0;
    int ms;
    char tf_program_name[100];
    strcpy(tf_program_name, argv[0]);
    char* tty_dev = "/dev/stdout";  /* Use standard output as default */
    char* text = "hello";  /* Default text to write */
    char* color;

    /* Parse command line arguments */
    while ((c = getopt_long(argc, argv, "acC:d:D:hrt:T:v", long_options, &option_index)) != -1)
    {
        switch (c)
        {
            case 'a':
                animated = 1;
                break;
            case 'c':
                clear = 1;
                break;
            case 'C':
                color = optarg;
                TF_LOWER(color);
                tf_set_color(color);
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
            case 't':
                text = optarg;
                break;
            case 'T':
                ms = atoi(optarg);
                if (ms > 0)
                    tf_set_ms_delay(ms);
                break;
            case 'v':
                tf_print_info(tf_program_name);
                exit(TF_SUCCESS);
            default:
                tf_usage(tf_program_name);
                exit(TF_ERROR);
        }
    }

    // Set the signal callback
    if (signal(SIGINT, tf_sighdlr) == SIG_ERR)
    {
        tf_dbg(1, "Wont catch SIGINT\n");
    }
    // Get the terminal size of the specified device
    g_ts = tf_get_term_size(tty_dev);

    if (clear == 1)
    {
        tf_clear_term(g_ts);
        exit (TF_SUCCESS);
    }
    if (random == 1)
    {
        tf_fill_random_term(g_ts);
        exit (TF_SUCCESS);
    }
    /* Do something */
    // tf_paint_text(g_ts, text);
    if (animated == 1)
    {
        tf_fill_vertical_rain(g_ts);
    }

    return 0;
}

