#ifndef TF_COLORS_H_
#define TF_COLORS_H_

enum tf_color_type
{
    TF_NORMAL,
    TF_RED,
    TF_GREEN,
    TF_YELLOW,
    TF_BLUE,
    TF_MAGENTA,
    TF_CYAN,
    TF_WHITE,
    TF_RANDOM,
    TF_INVALID
};

const char* tf_color_from_enum(enum tf_color_type ct);
const enum tf_color_type tf_color_from_str(const char *c);

#endif /* TF_COLORS_H_ */
