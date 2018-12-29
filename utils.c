#define _DEFAULT_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <termios.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>
#include <fcntl.h>

#include "utils.h"
#include "dbg.h"

static bool tf_animation_in_progress = false;

static pthread_mutex_t g_tf_mutex = PTHREAD_MUTEX_INITIALIZER;
static enum tf_color_type g_tf_color = TF_RANDOM;

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

    /* These two fields will be initialized after `tf_get_cursos_pos` call */
    t.init_cursor_pos.row = 0;
    t.init_cursor_pos.col = 0;

    if (tf_get_cursor_pos(&t) != 0)
        tf_dbg(1, "Unable to get current cursor position\n");

    tf_dbg(1, "Obtained a terminal window size of %zu x %zu for %s!\n", t.rows, t.cols, device);
    tf_dbg(1, "Original cursor on position (x,y)=(%zu,%zu)\n", t.init_cursor_pos.row, t.init_cursor_pos.col);

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

void tf_fill_random_col(u8_t *arr, size_t n)
{
    size_t i;
    size_t start_blanks;
    size_t nr_blanks;
    // Generate a random ASCII char
    for (i = 0; i < n; ++i)
        arr[i] = rand() % (TF_ASCII_MAX - TF_ASCII_MIN ) + TF_ASCII_MIN - 1;
    // Fill (deliberately) some spaces (visually better), at approx 1/2 of the rows 
    nr_blanks = n/2;
    start_blanks = rand() % (n - nr_blanks + 1);

    for (i = start_blanks; i < start_blanks + nr_blanks; ++i)
        arr[i] = ' ';
}

void* tf_thread_run(void* args)
{
    size_t row, col, nrows, ncols;
    int c;
    tf_thread_data_t* d = (tf_thread_data_t*)args;
    nrows = d->t.rows;
    ncols = d->stop_col - d->start_col + 1;
    srand(time(NULL));

    u8_t **colvals = d->columns; 
    colvals = (u8_t**) malloc(sizeof *colvals * ncols);
    for (size_t i = 0; i < ncols; ++i)
    {
        *(colvals + i) = (u8_t*) malloc(sizeof(u8_t) * nrows);
        tf_fill_random_col(*(colvals + i), nrows);
    }
    int tmp = 0;

    while (tf_animation_in_progress)
    {
        for (row = 1; row <= nrows; ++row)
        {
            pthread_mutex_lock(&g_tf_mutex);
            for (col = d->start_col; col <= d->stop_col; ++col)
            {
                // Place the cursor here
                tf_set_cursor_pos(&(d->t), row, col);
                // Print it
                tf_write_dev(d->t.fd, "%s%c", tf_color_from_enum(g_tf_color),
                        colvals[col - d->start_col][(nrows - row + tmp) % nrows]);
            }
            pthread_mutex_unlock(&g_tf_mutex);
        }
        tmp++;
        // Sleep a bit to notice the effect
        usleep(10000);
    }
    for (size_t i = 0; i < ncols; ++i)
        free(*(colvals + i));
    free(colvals);
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

    /* Hide cursor */
    tf_write_dev(g_ts.fd, "%s", TF_HIDE_CURSOR);

    /* Toggle this flag to be able to release memory from signal */
    tf_animation_in_progress = true;

    /* Create an array of the columns */
    tf_columns = (bool*) malloc (sizeof(bool) * t.cols);
    for (size_t i = 0; i < t.cols; ++i)
        tf_columns[i] = false;

    /* Cols per thread */
    size_t cols_per_thread = t.cols / n;

    size_t i;
    for (i = 0; i < n; ++i)
    {
        arg[i].id = i;
        arg[i].t = t;
        /* Start && stop columns for every thread */
        arg[i].start_col = i*cols_per_thread + 1; 
        arg[i].stop_col  = i < n - 1?
                           (i+1)*cols_per_thread : 
                           t.cols;                 /* Last thread: until last column */
        pthread_create(&thread[i], &attr, tf_thread_run,  (void*)&arg[i]);
    }
    for (i = 0; i < n; i++)
    {
        pthread_join(thread[i], NULL);
    }
    /* Free the created array */
    free(tf_columns);
    /* Show cursor again */
    tf_write_dev(g_ts.fd, "%s", TF_SHOW_CURSOR);
    /* Set the cursor color back to normal */
    tf_write_dev(g_ts.fd, "%s", tf_color_from_enum(TF_NORMAL));
    /* Clear the terminal */
    tf_clear_term(g_ts);
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
                    tf_color_from_enum(g_tf_color),
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

int tf_get_cursor_pos(struct term_size* t)
{
    char buf[30]={0};
    int ret, i, pow;
    char ch;

    struct termios term, restore;

    if (!t)
        return 2;

    tcgetattr(t->fd, &term);
    tcgetattr(t->fd, &restore);
    term.c_lflag &= ~(ICANON|ECHO);
    tcsetattr(t->fd, TCSANOW, &term);

    write(t->fd, "\033[6n", 4);

    for( i = 0, ch = 0; ch != 'R'; i++ )
    {
        ret = read(t->fd, &ch, 1);
        if ( !ret )
        {
            tf_write_dev(t->fd, "getpos: error reading response!\n");
            return 1;
        }
        buf[i] = ch;
    }

    if (i < 2)
        return 1;

    for( i -= 2, pow = 1; buf[i] != ';'; i--, pow *= 10)
        t->cursor_pos.col = t->cursor_pos.col + ( buf[i] - '0' ) * pow;

    for( i-- , pow = 1; buf[i] != '['; i--, pow *= 10)
        t->cursor_pos.row = t->cursor_pos.row + ( buf[i] - '0' ) * pow;

    tcsetattr(t->fd, TCSANOW, &restore);
    return 0;
}

void tf_set_cursor_pos(struct term_size* t, size_t row, size_t col)
{
    assert(t != NULL);
    assert (row >= 1 && row <= t->rows);
    assert (col >= 1 && col <= t->cols);
    /* Remember: indexes start at 1 */
    tf_write_dev(t->fd, "\033[%zu;%zuH", row, col);
    /* Update these coordinates in the term structure */
    t->cursor_pos.row = row;
    t->cursor_pos.col = col;
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
    tf_set_cursor_pos(&t, 1, 1);

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
            tf_set_cursor_pos(&t, current_row + offset, (current_char * width_per_char) + 1);
        }
        /* Update these so @tf_set_cursor_pos places the cursor on the right position */
        /* We dont do this for the last char */
        if (current_char < text_length - 1)
        {
            current_row = 1;
            current_col = ((current_char + 1) * width_per_char) + 1;
            tf_set_cursor_pos(&t, current_row, current_col);
        }
    }
    // Move cursor to the leftmost bottom corner ;-)
    tf_set_cursor_pos(&t, t.rows, 1);
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

bool tf_is_animation_in_progress()
{
    return tf_animation_in_progress;
}

void tf_set_stop_thread_cond()
{
    tf_animation_in_progress = false;
}

bool tf_set_color(const char* c)
{
    enum tf_color_type ct = tf_color_from_str(c);
    if (ct == TF_INVALID)
    {
        fprintf(stderr, "Failed to set color from string \"%s\", assuming multicolor\n", c);
        g_tf_color = TF_RANDOM;
        return false;
    }
    tf_dbg(1, "Chosen color: \"%s\"\n", c);
    g_tf_color = ct;
    return true;
}
