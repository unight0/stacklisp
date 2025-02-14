#include <stdio.h>

#include "globals_extern.h"
#include "stack.h"
#include "utils.h"

// I'm retarded and have many times forgotten that the arguments
// lie on the data stack
#define ARGPOP() stack_pop(&data_stack)
#define RESPUSH(r) stack_push(&stack, (r));

void s_format_dispatch(Data d) {
    switch (d.t) {
    case 'i':
	printf("%lu", d.raw);
	break;
    case 'f':
	printf("%f", (float)d.raw);
	break;
    case 's':
	printf("%s", (char*)d.raw);
	break;
    case 'c':
	printf("(");
	s_format_dispatch(((Cons*)d.raw)->car);
	printf(" . ");
	s_format_dispatch(((Cons*)d.raw)->cdr);
	printf(")");
	break;
    case 'p':
	// Display nil properly
	if (d.raw == 0) {
	    printf("nil ");
	    break;
	}
	printf("<system function>");
	break;
    case 'l':
	printf("<user function>");
    }
}

void s_format(size_t argnum) {
    if (!argnum) return;

    for(size_t i = 0; i < argnum; i++) {
	Data d = stack_pop(&data_stack);
	s_format_dispatch(d);
    }
}

void s_add(size_t argnum) {
    if(argnum < 2) return;

    //NOTE: change later

    Data first = stack_pop(&data_stack);

    Raw r = first.raw;
    
    for (size_t i = 0; i < argnum-1; i++) {
	Data pop = stack_pop(&data_stack);
	if (pop.t != 'i') {
	    printf("Wrong type for +: %c\n", pop.t);
	    exit(1);
	}
	r += pop.raw;
    }

    //printf("+RES:%ld\n", r);
    stack_push(&stack, DATA(r));
    //dump_main_stack();
}
void s_mul(size_t argnum) {
    if (argnum < 2) return;

    //NOTE: change later

    Data first = stack_pop(&data_stack);

    Raw r = first.raw;
    
    for (size_t i = 0; i < argnum-1; i++) {
	Data pop = stack_pop(&data_stack);
	r *= pop.raw;
    }

    stack_push(&stack, DATA(r));
}
void s_cons(size_t argnum) {
    if (argnum != 2) return;

    Data car = ARGPOP();
    Data cdr = ARGPOP();

    ALLOCNEW(pairs, Cons, pairs_sz) = (Cons){car,cdr};
    RESPUSH(((Data){'c', (Raw)&pairs[pairs_sz-1]}));
}
