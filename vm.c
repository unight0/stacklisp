#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "defs.h"
#include "stack.h"
#include "globals.h"
#include "sysfuncs.h"
#include "utils.h"


#define LINK_TABLE_SZ 13
LinkTable link_table[LINK_TABLE_SZ] = {
    {"print", s_print, 1, -1},

    /* Arithmetics */
    {"+", s_add, 2, -1},
    {"*", s_mul, 2, -1},
    {"-", s_sub, 2, 2},
    {"/", s_div, 2, 2},

    /* Basic functionality */
    {"atom", s_atom, 2, 2},
    {"cons", s_cons, 2, -1},
    {"car", s_car, 1, 1},
    {"cdr", s_cdr, 1, 1},
    {"nil", 0, 0, 0},


    /* Logic operations */
    {"and", s_and, 2, -1},
    {"or", s_or, 2, -1},
    {"eq", s_eq, 2, -1}
};

/* Main code **********************************/

int check_type(char *code, size_t sz, size_t *ptr) {
    if (sz < 6) {
	printf("Invalid file size\n");
	exit(1);
    }
    char file_magic[7] = {
	code[0], code[1],
	code[2], code[3],
	code[4], code[5],
	0
    };
    if (strcmp(file_magic, "VMCSYM")) {
	printf("Invalid file format\n");
	exit(1);
    }
    *ptr += 6;
    return 0;
}

Symbol load_symbol(char *code, size_t sz, size_t *ptr) {
    char *name = NULL;
    size_t n_sz = 0;
    int32_t id = 0;

    // Parse string
    for (;code[*ptr];(*ptr)++) {
	EOFCHK(sz, *ptr);
	ALLOCNEW(name, char, n_sz) = code[*ptr];
    }
    ALLOCNEW(name, char, n_sz) = 0;

    ++*ptr;

    //Parse number (bigEndian)
    for (int i = 0; i < 4; ++*ptr, i++) {
	id <<= 8; 
	id |= code[*ptr];
    }

    return (Symbol){id, name};
}

void load_symbols(char *code, size_t sz, size_t *ptr) {
    while (*ptr < sz) {
	Symbol s = load_symbol(code, sz, ptr);

	if (!(s.id ^ (int32_t)0xFFFFFFFF)) {
	    free(s.name);
	    break;
	}
	
	symbols = realloc(symbols, ++symbols_sz*sizeof(Symbol));
	symbols[symbols_sz-1] = s;
    }
}

// Data == reusable strings
// Structure:
// CMDD_ID id H i 0 --> "Hi"
ReuseString load_one_data(char *code, size_t sz, size_t *ptr) {
    Raw id = 0;
    char *str = NULL;
    
    // No id block detected
    if (code[*ptr] != CMDD_ID) {
	return (ReuseString){0, NULL};
    }

    ++*ptr;

    // Read id (big endian)
    for (int i = 0; i < 8; i++, ++*ptr) {
	EOFCHK(sz, *ptr);
	id <<= 8;
	id |= code[*ptr];
    }

    str = &code[*ptr];

    // Skip the c-string
    for (;code[*ptr];++*ptr)
	EOFCHK(sz, *ptr);
    ++*ptr;
    
    return (ReuseString){id, str};
}

void load_data(char *code, size_t sz, size_t *ptr) {
    do {
	ALLOCNEW(rstrs, ReuseString, rstrs_sz) = load_one_data(code, sz, ptr);
    } while(rstrs[rstrs_sz-1].str != NULL);

    rstrs = realloc(rstrs, (--rstrs_sz) * sizeof(ReuseString));
}

// Use link table and the read symbols to
// Push function onto env with respective id
void link_symbols() {
    size_t max_id = 0;

    // Get max id of symbols
    for (size_t i = 0; i < symbols_sz; i++) {
	max_id = max_id <= (size_t)symbols[i].id ? (size_t)symbols[i].id : max_id;
    }

    env_sz = max_id+1;

    // Allocate env
    env = realloc(env, env_sz * sizeof(Data));
    
    // Set to 0 
    memset(env, 0, env_sz * sizeof(Data));

    // Set env to function pointers at id
    for (size_t i = 0; i < symbols_sz; i++) {
	int link_success = 0;
	for (size_t j = 0; j < LINK_TABLE_SZ; j++) {
	    // Link! :D
	    if (!strcmp(symbols[i].name, link_table[j].name)) {
		env[symbols[i].id] = (Data){'p', (Raw)link_table[j].fn};
		link_success = 1;
		//printf("LINK:%s --> %ld\n", symbols[i].name, symbols[i].id);
	    }
	}
	// Failed to link
	if (!link_success) {
	    printf("ERROR: failed to link \"%s\"!\n", symbols[i].name);
	    exit(1);
	}
    }

}

void print_code(char *code, size_t sz, size_t ptr);

void init_vm(char *code, size_t sz, size_t *ptr) {
    check_type(code, sz, ptr);
    load_symbols(code, sz, ptr);
    link_symbols();
    load_data(code, sz, ptr);
}

void free_symbols() {
    if (!symbols) return;
    for (size_t i = 0; i < symbols_sz; i++) {
	free(symbols[i].name);
    }
    free(symbols);
}

void free_ufunc(UserFunction *f) {
    free(f->cmds);
    f->sz = 0;
}
/*
void free_funcs() {
    for (size_t i = 0; i < funcs_sz; i++) {
	if (!funcs[i].is_sys) free_ufunc(&funcs[i].uf);
    }
    }*/
void free_ufuncs() {
    for (size_t i = 0; i < ufuncs_sz; i++) {
	free(ufuncs[i].cmds);
    }
    free(ufuncs);
}

void deinit_vm() {
    free_symbols();
    free_ufuncs();
    if (rstrs) free(rstrs);
    if (env) free(env);
    if (stack.arr) free(stack.arr);
    if (data_stack.arr) free(data_stack.arr);
}


Data read_data(char *code, size_t sz, size_t *ptr, char bytes) {
    Raw d = 0;
    // BigEndian
    for (int i = 0; i < bytes; ++*ptr, i++) {
	EOFCHK(sz, *ptr);
	d <<= 8;
	d |= code[*ptr];
    }

    return DATA(d);
}

// Push constant string onto a stack
void envv_str(Raw id) {
    for (size_t i = 0; i < rstrs_sz; i++) {
	if (rstrs[i].id == id) {
	    stack_push(&stack, (Data){'s', (Raw)rstrs[i].str});
	    return;
	}
    }
    printf("ERROR: envv_str: str not found: %ld!\n", id);
    exit(1);
}

// Push data from env onto stack
void envv_env(Raw id) {
    if ((size_t)id >= env_sz) {
	//printf("ENVSZ: %lu\n", env_sz);
	printf("ENVV id doesn't exist: %lu\n", id);
    }
    stack_push(&stack, env[(size_t)id]);
}

// Dispatch between envv_env and envv_str
void exec_envv(Raw id) {
    // String id
    if (id < 0) {
	envv_str(id);
	return;
    }
    // Just variable in the environment
    envv_env(id);
}

// Predefinitions
void exec_instr(char *code, size_t sz, size_t *ptr);
void block_instr(char *code, size_t sz, size_t *ptr);

// Apply system (predefined) function
void sys_apply(SysFunction fn, Raw argnum) {
    if (fn == 0) {
	printf("Applying nil as function!\n");
	exit(1);
    }
    fn(argnum);
}

void user_apply(Raw idx, Raw argnum) {
    //TODO: use it
    (void)argnum;

    UserFunction *fn = &ufuncs[idx];
    
    size_t ptr = 0;
    for (;ptr < fn->sz;) {
	if (block_depth > 0) {
	    block_instr(fn->cmds, fn->sz, &ptr);
	    continue;
	}
	exec_instr(fn->cmds, fn->sz, &ptr);
    }

}

// Dispatch between sys_apply() and user_apply()
void apply() {
    if (stack.sz < 2) {
	printf("APPLY: expected 2 arguments on stack, got %lu\n", stack.sz);
	return;
    }
    
    Raw argnum = stack_pop(&stack).raw;
    //Function *fun = (Function*)stack_pop(&stack).raw;
    Data fun = stack_pop(&stack);

    if (data_stack.sz < (size_t)argnum) {
	dump_data_stack();
	printf("APPLY: expected %lu arguments on data stack, got %lu\n", argnum, data_stack.sz);
	return;
    }

    if (fun.t == 'p') {
	sys_apply((SysFunction)fun.raw,
		  argnum);
	return;
    }
   
    user_apply(fun.raw, argnum);
}

void do_if() {
    assert(stack.sz >= 3);

    Data cond = stack_pop(&stack);
    Data then = stack_pop(&stack);
    Data otherwise = stack_pop(&stack);

    // NIL --> do otherwise
    if (cond.t == 'p' && cond.raw == 0) {
	user_apply(otherwise.raw, 0);
	return;
    }

    // Do then
    user_apply(then.raw, 0);
}

void exec_instr(char *code, size_t sz, size_t *ptr) {
    Data d = DATA(0);

    #ifdef DEBUG_STACK
    if (code[*ptr] < NUM_CMDS) {
	printf("CURRENT POINTER: %8lu\nCURRENT COMMAND: %8s\n", *ptr, cmdstr[(int)code[*ptr]]);
    }
    else printf("CURRENT POINTER: %8lu\nCURRENT COMMAND IS INVALID\n", *ptr);
    dump_both_stacks();
    //printf("BLOCK DEPTH: %lu\n", block_depth);
    #endif
    switch (code[*ptr]) {
    case CMD_PUSH:
	//printf("WARNING: CMD_PUSH is deprecated\n");
	++*ptr;
	d = read_data(code, sz, ptr, 8);
	stack_push(&stack, d);
	break;
	/*
    case CMD_PUSHB:
	++*ptr;
	d = DATA(code[*ptr]);
	stack_push(&stack, d);
	break;
    case CMD_PUSHW:
	++*ptr;
	d = read_data(code, sz, ptr, 2);
	stack_push(&stack, d);
	break;
    case CMD_PUSHD:
	++*ptr;
	d = read_data(code, sz, ptr, 4);
	stack_push(&stack, d);
	break;
    case CMD_PUSHQ:
	++*ptr;
	d = read_data(code, sz, ptr, 8);
	stack_push(&stack, d);
	break;*/
    case CMD_ENVV:
	++*ptr;
	d = read_data(code, sz, ptr, 8);
	exec_envv(d.raw);
	break;
    case CMD_DROP:
	++*ptr;
	(void)stack_pop(&stack);
	break;
    case CMD_TD:
	++*ptr;
	d = stack_pop(&stack);
	stack_push(&data_stack, d);
	break;
    case CMD_FD:
	++*ptr;
	d = stack_pop(&data_stack);
	stack_push(&stack, d);
	break;
    case CMD_D: // Get nth element from data stack
	++*ptr;
	d = stack_pop(&stack);
	if (data_stack.sz <= (size_t)d.raw) {
	    printf("D: data stack is not deep enough!\n");
	    break;
	}
	d = data_stack.arr[data_stack.sz-1-d.raw];
	stack_push(&stack, d);
	break;
    case CMD_ND: // Push n elements to data stack
	++*ptr;
	d = stack_pop(&stack);
	if (stack.sz < (size_t)d.raw) {
	    printf("ND: stack is not deep enough: expected %lu, got %lu\n", (size_t)d.raw, stack.sz);
	    break;
	}
	for (size_t i = 0; i < (size_t)d.raw; i++) {
	    stack_push(&data_stack,
		       stack_pop(&stack));
	}
	break;
    case CMD_APPL:
	++*ptr;
	apply();
	break;
    case CMD_LAM:
	++*ptr;

	// Add to the list (will be deallocated)
	ALLOCNEW(ufuncs, UserFunction, ufuncs_sz) = (UserFunction) {
		blk_sz,
		block
	    };

	block = NULL;
	blk_sz = 0;
	// Pop argnum off the stack (unused)
	(void)stack_pop(&stack);
	
	// Push the lambda onto the stack
	stack_push(&stack,
		   (Data){'l', ufuncs_sz-1});
	break;
    case CMD_IF:
	++*ptr;
	do_if();
	break;
    case CMD_BLKB:
	++*ptr;
	block_depth++;
	break;
    case CMD_BLKE:
	printf("Unexpected CMD_BLKE instruction\n");
	exit(1);
    case CMDD_BD:
    case CMDD_ED:
    case CMDD_ID:
	printf("Data command outside of data section: %d\n", code[*ptr]);
	exit(1);
    default:
	printf("Command %d not implemented\n", code[*ptr]);
	exit(1);
    }
}

void block_instr(char *code, size_t sz, size_t *ptr) {
    // Transfer data untouched
    // (avoid CMD_BLKE false trigger)
    if (code[*ptr] == CMD_PUSH
	|| code[*ptr] == CMD_ENVV) {
	// Put in CMD_PUSH or CMD_ENVV
	ALLOCNEW(block, char, blk_sz) = code[*ptr];
	
	++*ptr;
	
	// Transfer data
	for (int i = 0; i < 8; ++*ptr, i++) {
	    EOFCHK(sz, *ptr);
	    ALLOCNEW(block, char, blk_sz) = code[*ptr];
	}
	
	return;
    }

    if (code[*ptr] == CMD_BLKE) {
	block_depth--;
    }

    if (!block_depth) {
	// Omit final CMD_BLKE
	++*ptr;
	return;
    }

    ALLOCNEW(block, char, blk_sz) = code[*ptr];
    ++*ptr;
}

void print_code(char *code, size_t sz, size_t i) {
    for (; i < sz;) {
	if (code[i] == CMD_PUSH || code[i] == CMD_ENVV) {
	    i++;
	    Raw data = 0;

	    for (int j = 0; j < 8; j++) {
		data <<= 8;
		data |= code[i+j];
	    }
	    i += 8;

	    if (code[i-9] == CMD_PUSH)
		printf("%ld ", data);
	    else printf("(%ld) ", data);

	    continue;
	}

	if (code[i] < NUM_CMDS)
	    printf("%s ", cmdstr[(int)code[i]]);
	else // Unknown command
	    printf("<> ");
	
	i++;
    }
    putchar('\n');
}

void vm(char *code, size_t sz) {
    size_t ptr = 0;
    init_vm(code, sz, &ptr);
    for (;ptr < sz;) {
	if (block_depth > 0) {
	    block_instr(code, sz, &ptr);
	    continue;
	}
	exec_instr(code, sz, &ptr);
    }
    deinit_vm();
}

void read_and_display(char *code, size_t codesize) {
    size_t ptr = 0;
    check_type(code, codesize, &ptr);
    load_symbols(code, codesize, &ptr);
    link_symbols();
    load_data(code, codesize, &ptr);
    printf("<symbol section><data section>");
    print_code(code, codesize, ptr);    
}

void usage() {
    printf("USAGE: vm run[|read] file\n");
}

//TODO:
//[] format -> print
//[] define
//[] floats
//[] --> conversion, casting?
int main(int argc, char **argv) {
    if (argc != 3) {
	usage();
	return -1;
    }

    size_t codesize = 0;
    char *code = readfile(argv[2], &codesize);
    if (code == NULL) return -1;

    if (!strcmp(argv[1], "read")) {
	read_and_display(code, codesize);
	free(code);
	return 0;
    }

    if (strcmp(argv[1], "run")) {
	printf("Unknown command: %s\n", argv[1]);
	return -1;
    }
    
    vm(code, codesize);
    
    free(code);

    return 0;
}
