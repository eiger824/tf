#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <stdarg.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>
#include <fcntl.h>

#include "utils.h"
#include "dbg.h"

struct term_size tf_get_term_size(const char* device)
{
    struct winsize w;
    struct term_size t;
    int fd;

    /* Don't re-open file descriptors if standard input/output/error are selected */
    if (!strcmp(device, "/dev/stdout"))
    {
        fd = STDOUT_FILENO;
    }
    else if (!strcmp(device, "/dev/stderr"))
    {
        fd = STDERR_FILENO;
    }
    else if (!strcmp(device, "/dev/stdin"))
    {
        fd = STDIN_FILENO;
    }
    else
    {
        /* Open this device */
        fd = open(device, O_RDWR | O_APPEND);
        if (fd < 0)
        {
            tf_die("Could not open file %s for reading", device);
        }
    }
    /* Check whether this device really is a tty */
    if (!isatty(fd))
    {
        close(fd);
        tf_die("This file is not a tty");
    }
    /* Query the size of this tty device */
    ioctl(fd, TIOCGWINSZ, &w);

    /* Fill in the data structure */
    t.rows = w.ws_row;
    t.cols = w.ws_col;
    t.fd   = fd;

    tf_dbg(1, "Obtained a size of %zu x %zu for %s!\n", t.rows, t.cols, device);

    /* Close only if we are NOT dealing with standard input/output/error */
    if (fd != STDOUT_FILENO && fd != STDERR_FILENO && STDIN_FILENO)
        close(fd);
    return t;
}

void tf_fill_term(struct term_size t, int c, enum tf_color_type ct)
{
    size_t rows, cols;
    const char* color = tf_color_from_enum(ct);
    for (rows = 0; rows < t.rows; ++rows)
    {
        for (cols = 0; cols < t.cols; ++cols)
        {
            if (c >= TF_ASCII_MIN && c <= TF_ASCII_MAX)
                tf_write_dev(t.fd, "%s%c", color, c);
            else
                tf_write_dev(t.fd, "?");
        }
        tf_write_dev(t.fd, "\n");
    }
    // Set back color to normal
    if (ct != TF_NORMAL)
    {
        tf_write_dev(t.fd, "%s", tf_color_from_enum(TF_NORMAL));
    }
}

void* tf_thread_run(void* args)
{
    size_t row, col, nrows, ncols;
    int c;
    tf_thread_data_t* d = (tf_thread_data_t*)args;
    nrows = d->t.rows;
    ncols = d->t.cols;
    srand(time(NULL));

    for (;;)
    {
        col = rand() % (ncols) + 1;
        for (row = 1; row <= nrows; ++row)
        {
            // Place the cursor here
            tf_goto_coord(d->t, row, col);
            // Generate a random ASCII char (including spaces)
            c = rand() % (TF_ASCII_MAX - TF_ASCII_MIN ) + TF_ASCII_MIN - 1;
            // And print it
            tf_write_dev(d->t.fd, "%s%c", tf_color_from_enum(TF_RANDOM), c);
            // Sleep a bit to notice the effect
            usleep(2000);
        }
    }
    return NULL;
}

void tf_fill_vertical_rain(struct term_size t)
{
    /* Get available cpus, not necessarily HT threads though... */
    size_t n = get_nprocs();

    pthread_t thread[n];
    pthread_attr_t attr;

    tf_thread_data_t arg[n];
    pthread_attr_init(&attr);

    tf_clear_term(t);

    size_t i;
    for (i = 0; i < n; ++i)
    {
        arg[i].id = i;
        arg[i].t = t;
        pthread_create(&thread[i], &attr, tf_thread_run,  (void*)&arg[i]);
    }
    for (i = 0; i < n; i++)
    {
        pthread_join(thread[i], NULL);
    }
}

void tf_fill_random_term(struct term_size t)
{
    size_t rows, cols;
    srand(time(NULL));
    for (rows = 0; rows < t.rows; ++rows)
    {
        for (cols = 0; cols < t.cols; ++cols)
        {
            tf_write_dev(t.fd, "%s%c",
                    tf_color_from_enum(TF_RANDOM),
                    rand() % (TF_ASCII_MAX - TF_ASCII_MIN + 1) + TF_ASCII_MIN);
        }
        tf_write_dev(t.fd, "\n");
    }
    // Set back color to normal
    tf_write_dev(t.fd, "%s", tf_color_from_enum(TF_NORMAL));
}

void tf_clear_term(struct term_size t)
{
    tf_write_dev(t.fd, "\033[H\033[J");
}
void tf_goto_coord(struct term_size t, size_t row, size_t col)
{
    assert (row >= 1 && row <= t.rows);
    assert (col >= 1 && col <= t.cols);
    /* Remember: indexes start at 1 */
    tf_write_dev(t.fd, "\033[%zu;%zuH", row, col);
}

//TODO: change return type (better to handle errors)
void tf_paint_text(struct term_size t, const char* text)
{
    tf_clear_term(t);
    size_t text_length = strlen(text);
    // Calculate how the screen will be divided for every char
    size_t width_per_char  = t.cols / text_length;
    size_t height_per_char = t.rows;
    tf_dbg(2, "Each char will be contained in a %zux%zu box\n", width_per_char, height_per_char);
    size_t current_row = 0, current_col = 0, current_char;

    /* Start off att 1,1 */
    tf_goto_coord(t, 1, 1);

    for (current_char = 0; current_char < text_length; ++current_char)
    {
        for (current_row = 1; current_row <= height_per_char; ++current_row)
        {
            for (current_col = 1; current_col <= width_per_char; ++current_col)
            {
                tf_write_dev(t.fd, "%c",
                        (current_col == 1 || current_col == width_per_char) || \
                        (current_row == 1 || current_row == height_per_char) ?
                        '*' : ' ');
            }
            // Update the cursor
            int offset = current_row < height_per_char ? 1 : 0;
            tf_goto_coord(t, current_row + offset, (current_char * width_per_char) + 1);
        }
        /* Update these so @tf_goto_coord places the cursor on the right position */
        /* We dont do this for the last char */
        if (current_char < text_length - 1)
        {
            current_row = 1;
            current_col = ((current_char + 1) * width_per_char) + 1;
            tf_goto_coord(t, current_row, current_col);
        }
    }
    // Move cursor to the leftmost bottom corner ;-)
    tf_goto_coord(t, t.rows, 1);
    tf_write_dev(t.fd, "\n");
}

void tf_die(const char* fmt, ...)
{
    int ret = errno;
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, ": %s\n", strerror(ret));
    va_end(args);
    exit(ret);
}

void tf_write_dev(int fd, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char buffer[1024];
    vsprintf(buffer, fmt, args);
    if (write(fd, buffer, strlen(buffer)) < 0)
    {
        va_end(args);
        tf_die("Failed to write %zu bytes to file descriptor %d\n", strlen(buffer), fd);
    }
    va_end(args);
}

