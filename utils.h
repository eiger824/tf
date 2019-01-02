#ifndef TF_UTILS_H_
#define TF_UTILS_H_

#include <stdbool.h>

#include "defs.h"
#include "colors.h"

struct term_size g_ts;

struct term_size tf_get_term_size(const char* device);
void   tf_fill_term(struct term_size t, int c, enum tf_color_type ct);
void   tf_fill_random_term(struct term_size t);
void   tf_fill_vertical_rain(struct term_size t);
void   tf_clear_term(struct term_size t);
int    tf_get_cursor_pos(struct term_size* t);
void   tf_set_cursor_pos(struct term_size* t, size_t row, size_t col);
void   tf_paint_text(struct term_size t, const char* text);
void   tf_die(const char* fmt, ...);
void   tf_write_dev(int fd, const char* fmt, ...);
bool   tf_is_animation_in_progress();
void   tf_set_stop_thread_cond();
bool   tf_set_color(const char* c);

#endif /* TF_UTILS_H_ */
