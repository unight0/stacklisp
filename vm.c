#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Utilities **********************************/
#define _PERRET_F(c,f,r) if ((c)) {\
	perror((f));		  \
	return (r);		  \
 }
#define PERRET(c,r) _PERRET_F(c,__func__,r)
#define _EOFCHK_F(s,f,p) if ((p) >= s) {	\
	printf("%s: unexpected EOF\n", (f));	\
	exit(1);				\
 }
#define EOFCHK(s,p) _EOFCHK_F(s,__func__,p)
// Allocated 1 new element in array, returns new element (assignable)
#define ALLOCNEW(p,t,s) p = realloc(p, (++s)*sizeof(t)); p[s-1]
    

// Just to test if such macros are possible
// That would be OP!
#define _RET1(c) if ((c)) return 1;
#define _THROWABLE_1(o, ...) _RET1(o);\
    _THROWABLE_1(__VA_ARGS__);
#define THROWABLE(...) _THROWABLE_1(__VA_ARGS__)


char *readfile(const char *filename, size_t *sz) {
    FILE *f = fopen(filename, "rb");
    char *contents = NULL;

    // Error occured
    PERRET(f == NULL, NULL);

    while(!feof(f)) {
	contents = realloc(contents, ++(*sz));
	contents[*sz-1] = fgetc(f);
    }
    
    return contents;
}

/* Declarations of system functions ***********/
//TODO!TODO!TODO!TODO!TODO!TODO!TODO!TODO!TODO!TODO!: SOS:
//#1[SOLVED]: we don't know how many arguments are passed to the function!
//    And format is a variadic function! We need to pass the argnum!
//#2[SOLVED]: we don't know the data types!
//    Probably change Data to struct {char type; int64_t raw;};
//    It would increase the size of everything though...
//    Add a dictionary with data types when needed? Possible, a bit
//    too complex for such simple vm.
void s_format(size_t argnum);
/* Data structures ****************************/
//Hehe, global

enum {
    CMD_PUSH = 0, // Push to stack
    CMD_BLKB, // Begin block (commands are saved, not interpreted)
    CMD_BLKE, // End block
    CMD_LAM,  // Lambda (function definition) (lambda args body)
    CMD_DROP, // Drop from stack
    CMD_TD,   // Push to data stack >D
    CMD_FD,   // Pop from data stack D>
    CMD_D,    // Nth from data stack
    CMD_ND,   // Push n to data stack >ND
    CMD_APPL, // Apply function
    CMD_ENVV, // Get value by ID (from Environment)
    
    //Datasection commands//////////////
    CMDD_BD, // Begin data(string) block
    CMDD_ED, // End data block
    CMDD_ID,  // Declare ID of data block

    NUM_CMDS,
};

const char *cmdstr[] = {
    [CMD_PUSH] = "PUSH",
    [CMD_BLKB] = "[",
    [CMD_BLKE] = "]",
    [CMD_LAM] = "L",
    [CMD_DROP] = "DROP",
    [CMD_TD] = ">D",
    [CMD_FD] = "D>",
    [CMD_D] = "D",
    [CMD_ND] = "ND",
    [CMD_APPL] = "A",
    [CMD_ENVV] = "ENVV",
    [CMDD_BD] = "{",
    [CMDD_ED] = "}",
    [CMDD_ID] = "ID"
};

typedef int64_t Raw;
typedef char Type;
enum {
    T_INT = 'i',
    T_FLT = 'f',
    T_STR = 's',

    T_CONS = 'c',
    T_PROC = 'p', // system function
    T_LAMB = 'l'  // user function
};

typedef struct {
    Type t;
    Raw raw;
} Data;

#define DATA(r) (Data){.t='i',.raw=(Raw)(r)}

typedef struct {
    size_t sz;
    Data *arr;
} Stack;

typedef struct {
    size_t sz;
    char *cmds;
} UserFunction;

typedef void (*SysFunction)(size_t);

typedef struct {
    Raw id;
    char *str;
} ReuseString;

typedef struct {
    Raw id;
    char *name;
} Symbol;

typedef struct {
    int is_sys;
    union {
	SysFunction sf;
	UserFunction uf;
    };
} Function;

#define LINK_TABLE_SZ 1
struct {
    const char *name;
    SysFunction fn;
    // Function signature difinition
    // Temporarely unused
    int argnum_min;
    int argnum_max;
} link_table[LINK_TABLE_SZ] = {
    {"format", s_format, 1, -1}
};

Stack data_stack = {0};
Stack stack = {0};
Data *env = NULL; 
//UserFunction *user_funcs = NULL;
//SysFunction *sys_funcs = NULL;
Function *funcs;
ReuseString *rstrs = NULL;
Symbol *symbols = NULL;
char *block = NULL;
size_t block_depth = 0;

size_t env_sz = 0;
//size_t ufuncs_sz = 0;
//size_t sfuncs_sz = 0;
size_t funcs_sz = 0;
size_t symbols_sz = 0;
size_t rstrs_sz = 0;
size_t blk_sz = 0;

/* Stack stuff ********************************/

void stack_push(Stack *st, Data d) {
    st->sz++;
    st->arr = realloc(st->arr, st->sz*sizeof(Data));
    st->arr[st->sz-1] = d;
}

Data stack_pop(Stack *st) {
    assert(st->sz != 0);
    Data d = st->arr[st->sz-1];
    st->sz--;
    st->arr = realloc(st->arr, st->sz*sizeof(Data));
    return d;
}
Data stack_top(Stack *st) {
    return st->arr[st->sz-1];
}

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
// CMDD_ID ID CMDD_BD 0 H 0 i CMDD_ED --> "Hi"
ReuseString load_one_data(char *code, size_t sz, size_t *ptr) {
    Raw id = 0;
    char *str = NULL;
    size_t str_sz = 0;
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

    
    // No actual data
    if (code[*ptr] != CMDD_BD) {
	printf("Read CMDD_ID but found no CMDD_BD right after!\n");
	exit(1);
    }

    ++*ptr;

    // Chars are prefixed with 0
    for (;!code[*ptr];++*ptr) {
	++*ptr;
	EOFCHK(sz, *ptr);
	ALLOCNEW(str, char, str_sz) = code[*ptr];
    }
    ALLOCNEW(str, char, str_sz) = 0;

    if (code[*ptr] != CMDD_ED) {
	printf("Read data block but found no CMD_ED(']') right after!\n");
	exit(1);
    }
    
    ++*ptr;

    return (ReuseString){id, str};
}

void load_data(char *code, size_t sz, size_t *ptr) {
    do {
	ALLOCNEW(rstrs, ReuseString, rstrs_sz) = load_one_data(code, sz, ptr);
	printf("%ld: %s\n", rstrs[rstrs_sz-1].id, rstrs[rstrs_sz-1].str);
    } while(rstrs[rstrs_sz-1].str != NULL);
}

// Use link table and the read symbols to
// Push function onto env with respective id
void link_symbols() {
    size_t max_id = 0;

    // Get max id of symbols
    for (size_t i = 0; i < symbols_sz; i++) {
	max_id = max_id <= (size_t)symbols[i].id ? (size_t)symbols[i].id : max_id;
    }

    // Allocate env
    env = realloc(env, env_sz * sizeof(Data));
    
    // Set to 0 
    memset(env, 0, env_sz * sizeof(Data));

    // Set env to function pointers at id
    for (size_t i = 0; i < symbols_sz; i++) {
	for (size_t j = 0; j < LINK_TABLE_SZ; j++) {
	    // Link! :D
	    if (!strcmp(symbols[i].name, link_table[j].name))
		env[symbols[i].id] = (Data){'p', (Raw)link_table[j].fn};
	}
    }
}

void print_code(char *code, size_t sz, size_t ptr);

void init_vm(char *code, size_t sz, size_t *ptr) {
    check_type(code, sz, ptr);
    load_symbols(code, sz, ptr);
    link_symbols();
    load_data(code, sz, ptr);
    print_code(code, sz, *ptr);
}

void free_symbols() {
    if (!symbols) return;
    for (size_t i = 0; i < symbols_sz; i++) {
	free(symbols[i].name);
    }
    free(symbols);
}

void free_rstrs() {
    if (!rstrs) return;
    for (size_t i = 0; i < symbols_sz; i++) {
	free(rstrs[i].str);
    }
    free(rstrs);
}

void free_ufunc(UserFunction *f) {
    free(f->cmds);
    f->sz = 0;
}

void free_funcs() {
    for (size_t i = 0; i < funcs_sz; i++) {
	if (!funcs[i].is_sys) free_ufunc(&funcs[i].uf);
    }
}

void deinit_vm() {
    free_symbols();
    free_rstrs();
    free_funcs();
    if (stack.arr) free(stack.arr);
    if (data_stack.arr) free(data_stack.arr);
}


Data read_data(char *code, size_t sz, size_t *ptr) {
    Raw d = 0;
    // BigEndian
    for (int i = 0; i < 8; ++*ptr, i++) {
	EOFCHK(sz, *ptr);
	d <<= 8;
	d |= code[*ptr];
    }

    return DATA(d);
}

// Push constant string onto a stack
void envv_str(Raw id) {
    for (size_t i = 0; i < rstrs_sz; i++) {
	if (rstrs[i].id == id)
	    stack_push(&stack, (Data){'s', (Raw)rstrs[i].str});
    }
    printf("ERROR: envv_str: str not found: %ld!\n", id);
    exit(1);
}

// Push data from env onto stack
void envv_env(Raw id) {
    if ((size_t)id >= env_sz) {
	printf("ENVV id doesn't exist: %lu\n", id);
    }
    stack_push(&stack, env[(size_t)id]);
}

// Dispatch between envv_env and envv_str
void exec_envv(Raw id) {
    // String id
    if (id < 0) return envv_str(id);
    // Just variable in the environment
    envv_env(id);
}

// Predefinitions
void exec_instr(char *code, size_t sz, size_t *ptr);
void block_instr(char *code, size_t sz, size_t *ptr);

// Apply system (predefined) function
void sys_apply(SysFunction fn, Raw argnum) {
    fn(argnum);
}

void user_apply(UserFunction *fn, Raw argnum) {
    //TODO: use it
    (void)argnum;
    size_t ptr = 0;
    for (;ptr < fn->sz;) {
	if (block_depth > 0) {
	    block_instr(fn->cmds, fn->sz, &ptr);
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
    Function *fun = (Function*)stack_pop(&stack).raw;

    if (stack.sz < (size_t)argnum) {
	printf("APPLY: expected %lu arguments on data stack, got %lu\n", argnum, stack.sz);
	return;
    }

    if (fun->is_sys == 1)
	return sys_apply(fun->sf,
			 argnum);
   
    user_apply(&fun->uf, argnum);
}

void exec_instr(char *code, size_t sz, size_t *ptr) {
    Data d = DATA(0);

    //printf("%lu:%s\n", *ptr, cmdstr[(int)code[*ptr]]);
    switch (code[*ptr]) {
    case CMD_PUSH:
	++*ptr;
	d = read_data(code, sz, ptr);
	stack_push(&stack, d);
	break;
    case CMD_ENVV:
	++*ptr;
	d = read_data(code, sz, ptr);
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
	if (data_stack.sz <= (size_t)d.raw) {
	    printf("ND: stack is not deep enough!\n");
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
	// Add to the list (will be deallocated)
	ALLOCNEW(funcs, UserFunction, funcs_sz) = (Function){
	    0,
	    .uf = (UserFunction) {
		blk_sz,
		block
	    }};
	
	// Push the lambda onto the stack
	stack_push(&stack,
		   (Data){'l', (Raw)&funcs[funcs_sz-1]});
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
	}

	if (code[i] < NUM_CMDS)
	    printf("%s ", cmdstr[(int)code[i]]);
	else printf("<> ");
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
	}
	exec_instr(code, sz, &ptr);
    }
    deinit_vm();
}

int main(int argc, char **argv) {
    if (argc != 2) {
	printf("Provide exactly 1 argument: filename");
	return -1;
    }

    size_t codesize = 0;
    char *code = readfile(argv[1], &codesize);
    if (code == NULL) return -1;

    vm(code, codesize);
    
    free(code);

    return 0;
}

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
    case 'p':
	printf("<system function>");
	break;
    case 'l':
	printf("<user function>");
    }
}

void s_format(size_t argnum) {
    if (!argnum) return;

    for(size_t i = 0; i < argnum; i++) {
	Data d = stack_pop(&stack);
	s_format_dispatch(d);
    }
}
