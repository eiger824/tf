#ifndef TF_DEFS_H_
#define TF_DEFS_H_

#include <stdio.h>

/* Program specifics */
#define     TF_VERSION_MAJOR    0
#define     TF_VERSION_MINOR    1
#define     TF_REVISION         5

/* Error codes - TODO: implement more */
#define     TF_SUCCESS      0
#define     TF_ERROR        1

/* Ascii limits */
#define     TF_ASCII_MIN    33
#define     TF_ASCII_MAX    126

/* Terminal colors */
#define     TF_NUMCOLORS    8

typedef struct coordinate
{
    size_t row;
    size_t col;
} tf_coord_t;

/* The basic structure used everywhere */
struct term_size
{
    size_t rows;
    size_t cols;
    int fd;                     /* File descriptor which has allocated to this terminal */
    tf_coord_t init_cursor_pos; /* The original cursor position before the program start */
    tf_coord_t cursor_pos;      /* The current cursor position */
};

typedef struct tf_thread_data
{
    size_t id;
    struct term_size t;
} tf_thread_data_t;

/* The main structure to use */
extern struct term_size g_ts;

#endif /* TF_DEFS_H_ */
