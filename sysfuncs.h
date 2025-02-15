#ifndef SYSFUNCS_H
#define SYSFUNCS_H

#include <stdlib.h>

/* Declarations of system functions ***********/
void s_print(size_t argnum);
void s_add(size_t argnum);
void s_sub(size_t argnum);
void s_mul(size_t argnum);
void s_div(size_t argnum);
void s_cons(size_t argnum);
void s_nil(size_t argnum);
void s_atom(size_t argnum);
void s_car(size_t argnum);
void s_cdr(size_t argnum);
void s_and(size_t argnum);
void s_or(size_t argnum);
void s_eq(size_t argnum);

#endif
