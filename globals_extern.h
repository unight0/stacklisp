#ifndef GLOBALS_EXTERN_H
#define GLOBALS_EXTERN_H

#include <stdlib.h>
#include "defs.h"

/* Data structures ****************************/
//Hehe, global
//#define LINK_TABLE_SZ 5
//extern LinkTable link_table[LINK_TABLE_SZ];

extern const char *cmdstr[];

extern Stack data_stack;
extern Stack stack;
extern Data *env; 
//UserFunction *user_funcs = NULL;
//SysFunction *sys_funcs = NULL;
extern Function *funcs;
extern ReuseString *rstrs;
extern Symbol *symbols;
extern Cons *pairs;

extern char *block;
extern size_t block_depth;

extern size_t env_sz;
//size_t ufuncs_sz = 0;
//size_t sfuncs_sz = 0;
extern size_t funcs_sz;
extern size_t symbols_sz;
extern size_t rstrs_sz;
extern size_t blk_sz;
extern size_t pairs_sz;

#endif
