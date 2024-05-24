#ifndef MY_LOX_COMPILE_H
#define MY_LOX_COMPILE_H
#include "array.h"

void compile_code(array_t *tokens);

void var_declaration(array_t *tokens);
void expression(array_t *tokens);
void declaration(array_t *tokens);

#endif
