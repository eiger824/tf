#ifndef TF_UTILS_H_
#define TF_UTILS_H_

#include "defs.h"
#include "colors.h"

struct term_size tf_get_term_size(void);
void tf_fill_term(struct term_size t, int c, enum tf_color_type ct);
void tf_fill_random_term(struct term_size t);
void tf_clear_term(void);
void tf_goto_coord(struct term_size ts, size_t x, size_t y);
void tf_paint_text(struct term_size t, const char* text);

#endif /* TF_UTILS_H_ */
