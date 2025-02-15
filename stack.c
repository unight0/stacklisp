#include <stdio.h>
#include <assert.h>

#include "stack.h"

/* Stack stuff ********************************/

void dump_stack_dispatch(Data d) {
    switch(d.t){
    case 'i':printf("%ld ", d.raw); break;
    case 'f':printf("%f ", (float)d.raw); break;
    case 's':printf("\"%s\" ", (char*)d.raw); break;
	
    case 'c':
	printf("(");
	dump_stack_dispatch(((Cons*)d.raw)->car);
	printf(" . ");
	dump_stack_dispatch(((Cons*)d.raw)->cdr);
	printf("\b) ");
	break;
    case 'p':
	// Display nil properly
	if (d.raw == 0) {
	    printf("nil ");
	    break;
	}
	printf("<sysfun> ");
	break;
    case 'l':printf("<lambda> "); break;
    default: printf("<dump not implemented: %c>", d.t);
    }    
}


void dump_stack(Stack *st) {
    printf("STACK SIZE: %16lu\n", st->sz);
    for (size_t i = 0; i < st->sz; i++) {
	dump_stack_dispatch(st->arr[i]);
    }
    if(st->sz) putchar('\n');
}


void dump_data_stack() {
    printf("---DATA STACK DUMP--------------------------------------------------------------\n");
    dump_stack(&data_stack);
    printf("--------------------------------------------------------------------------------\n");
}
void dump_main_stack() {
    printf("---MAIN STACK DUMP--------------------------------------------------------------\n");
    dump_stack(&stack);
    printf("--------------------------------------------------------------------------------\n");
}

void dump_both_stacks() {
    printf("---MAIN STACK DUMP--------------------------------------------------------------\n");
    dump_stack(&stack);
    printf("---DATA STACK DUMP--------------------------------------------------------------\n");
    dump_stack(&data_stack);
    printf("--------------------------------------------------------------------------------\n");
}

void stack_push(Stack *st, Data d) {
    st->sz++;
    st->arr = realloc(st->arr, st->sz*sizeof(Data));
    st->arr[st->sz-1] = d;
}

Data stack_pop(Stack *st) {
    assert(st->sz != 0);
    Data d = st->arr[st->sz-1];
    st->sz--;

    if(st->sz != 0)
	st->arr = realloc(st->arr, st->sz*sizeof(Data));
    else {
	free(st->arr);
	st->arr = NULL;
    }
    
    return d;
}
Data stack_top(Stack *st) {
    return st->arr[st->sz-1];
}
