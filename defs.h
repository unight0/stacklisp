#ifndef DEFS_H
#define DEFS_H

#include <stdlib.h>

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

    CMD_IF,  // If
    /*
    CMD_PUSHB, // Push byte
    CMD_PUSHW, // Push word
    CMD_PUSHD, // Push dword
    CMD_PUSHQ, // Push qword
    */
    NUM_CMDS,
};

/*
#define ANY_PUSH(a) ((a) == CMD_PUSHB || (a) == CMD_PUSHW || (a) == CMD_PUSHD || (a) == CMD_PUSHQ || (a) == CMD_PUSH)
#define PUSH_LEN(a) (a) == CMD_PUSHB ? 1 : (a) == CMD_PUSHW ? 2 : (a) == CMD_PUSHD ? 4 : (a) == CMD_PUSHQ ? 8 : 0
*/

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

// TODO: remove
typedef struct {
    int is_sys;
    union {
	SysFunction sf;
	UserFunction uf;
    };
} Function;

typedef struct {
    Data car;
    Data cdr;
} Cons;

typedef struct {
    const char *name;
    SysFunction fn;
    // Function signature difinition
    // Temporarely unused
    int argnum_min;
    int argnum_max;
} LinkTable;

#endif
