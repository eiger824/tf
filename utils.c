#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/ioctl.h>

#include "utils.h"
#include "dbg.h"

struct term_size tf_get_term_size()
{
    struct winsize w;
    struct term_size t;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    t.rows = w.ws_row;
    t.cols = w.ws_col;
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
                printf("%s%c", color, c);
            else
                printf("?");
        }
        printf("\n");
    }
    // Set back color to normal
    if (ct != TF_NORMAL)
        printf("%s", tf_color_from_enum(TF_NORMAL));
}

void tf_fill_random_term(struct term_size t)
{
    size_t rows, cols;
    srand(time(NULL));
    for (rows = 0; rows < t.rows; ++rows)
    {
        for (cols = 0; cols < t.cols; ++cols)
        {
            printf("%s%c",
                    tf_color_from_enum(TF_RANDOM),
                    rand() % (TF_ASCII_MAX - TF_ASCII_MIN + 1) + TF_ASCII_MIN);
        }
        printf("\n");
    }
    // Set back color to normal
    printf("%s", tf_color_from_enum(TF_NORMAL));
}

void tf_clear_term()
{
    TF_CLEAR_TERM();
}
void tf_goto_coord(struct term_size ts, size_t x, size_t y)
{
//     if (y >= ts.rows || x >= ts.cols)
//         return;
    TF_GOTO_COORD(x,y);
}

//TODO: change return type (better to handle errors)
void tf_paint_text(struct term_size t, const char* text)
{
//     tf_clear_term();
    size_t text_length = strlen(text);
    // Calculate how the screen will be divided for every char
    size_t width_per_char  = t.cols / text_length;
    size_t height_per_char = t.rows;
    tf_dbg(1, "Each char will be contained in a %zux%zu box\n", width_per_char, height_per_char);
    size_t current_row = 0, current_col = 0, current_char;

    for (current_char = 0; current_char < text_length; ++current_char)
    {
        int c = 97 + current_char; 
        // Move the cursor to the current position
//         tf_goto_coord(t, current_row, current_col); 
        printf("set cursor on: %zu x %zu, end up in: %zu x %zu\n",
                current_row, current_col, height_per_char, (current_char + 1) * width_per_char);
        for (current_row = 0; current_row < height_per_char; ++current_row)
        {
            for (current_col = current_char * width_per_char; current_col < (current_char + 1) * width_per_char; ++current_col)
            {
//                 printf("%c", c);
            }
        }
        current_row = 0;
    }
}
