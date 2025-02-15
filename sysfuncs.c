#include <stdio.h>
#include <assert.h>

#include "globals_extern.h"
#include "stack.h"
#include "utils.h"

// I'm retarded and have many times forgotten that the arguments
// lie on the data stack
#define ARGPOP() stack_pop(&data_stack)
#define RESPUSH(r) stack_push(&stack, (r));

// Comes in handy
#define NIL ((Data){'p',0})
#define T DATA(1)

void s_format_dispatch(Data d) {
    switch (d.t) {
    case 'i':
	printf("%ld", d.raw);
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
    assert(argnum >= 2);

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

void s_sub(size_t argnum) {
    assert(argnum == 2);

    Data from = ARGPOP();
    Data what = ARGPOP();

    if (what.t == 'f' || from.t == 'f') {
	RESPUSH(((Data){'f',((float)from.raw)-((float)what.raw)}));
	return;
    }

    RESPUSH(((Data){'i',((Raw)from.raw)-((Raw)what.raw)}));
}

void s_mul(size_t argnum) {
    assert(argnum >= 2);

    //NOTE: change later

    Data first = stack_pop(&data_stack);

    Raw r = first.raw;
    
    for (size_t i = 0; i < argnum-1; i++) {
	Data pop = stack_pop(&data_stack);
	r *= pop.raw;
    }

    stack_push(&stack, DATA(r));
}

void s_div(size_t argnum) {
    assert(argnum == 2);

    Data what = ARGPOP();
    Data by = ARGPOP();

    if (what.t == 'f' || by.t == 'f') { 
	RESPUSH(((Data){'f',((float)what.raw)/((float)by.raw)}));
	return;
    }

    RESPUSH(((Data){'i',((Raw)what.raw)/((Raw)by.raw)}));
}

void s_cons(size_t argnum) {
    assert(argnum == 2);

    Data car = ARGPOP();
    Data cdr = ARGPOP();

    ALLOCNEW(pairs, Cons, pairs_sz) = (Cons){car,cdr};
    RESPUSH(((Data){'c', (Raw)&pairs[pairs_sz-1]}));
}

void s_atom(size_t argnum) {
    assert(argnum == 1);

    Data d = ARGPOP();

    if (d.t == 'f' || d.t == 'i' || d.t == 's') {
	RESPUSH(DATA(1));
	return;
    }

    RESPUSH(((Data){'p',0}));
}

void s_car(size_t argnum) {
    assert(argnum == 1);

    Data cons = ARGPOP();

    if (cons.t != 'c') {
	printf("ERROR: argument for car is not cons!\n");
	exit(1);
    }

    RESPUSH(((Cons*)cons.raw)->car);
}

void s_cdr(size_t argnum) {
    assert(argnum == 1);

    Data cons = ARGPOP();

    if (cons.t != 'c') {
	printf("ERROR: argument for cdr is not cons!\n");
	exit(1);
    }

    RESPUSH(((Cons*)cons.raw)->cdr);    
}

void s_and(size_t argnum) {
    assert(argnum >= 2);

    for (size_t i = 0; i < argnum; i++) {
	Data pop = ARGPOP();
	// Nil = false
	if(pop.t == 'p' && pop.raw == 0) {
	    RESPUSH(NIL);
	    return;
	}
    }

    RESPUSH(T);
}
void s_or(size_t argnum) {
    assert(argnum >= 2);
    for (size_t i = 0; i < argnum; i++) {
	Data pop = ARGPOP();
	// If only one of arguments is true, return true
	if(pop.t != 'p' || pop.raw != 0) {
	    RESPUSH(T);
	    return;
	}
    }

    // Return nil if every argument is nil
    RESPUSH(NIL);    
}
