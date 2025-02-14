#ifndef STACK_H
#define STACK_H

#include "globals_extern.h"

void dump_stack(Stack *st);
void dump_main_stack();
void dump_data_stack();
void dump_both_stacks();

void stack_push(Stack *st, Data d);
Data stack_pop(Stack *st);
Data stack_top(Stack *st);

#endif
