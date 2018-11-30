#ifndef TF_UTILS_H_
#define TF_UTILS_H_

#include "defs.h"
#include "colors.h"

struct term_size tf_get_term_size(const char* device);
void   tf_fill_term(struct term_size t, int c, enum tf_color_type ct);
void   tf_fill_random_term(struct term_size t);
void   tf_clear_term(void);
void   tf_goto_coord(struct term_size ts, size_t row, size_t col);
void   tf_paint_text(struct term_size t, const char* text);
void   tf_die(const char* fmt, ...);
void   tf_write_dev(struct term_size t, const char* fmt, ...);

#endif /* TF_UTILS_H_ */
