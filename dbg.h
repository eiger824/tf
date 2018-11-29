#ifndef TF_DBG_H_
#define TF_DBG_H_

#include <stdio.h>

void tf_set_debug_level(size_t level);
void tf_dbg(size_t level, const char* fmt, ...);


#endif /* TF_DBG_H_ */
