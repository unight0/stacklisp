#ifndef GLOBALS_H
#define GLOBALS_H

#include "defs.h"
#include "sysfuncs.h"

/* Data structures ****************************/
//Hehe, global
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

Stack data_stack = {0};
Stack stack = {0};
Data *env = NULL; 
UserFunction *ufuncs = NULL;
//SysFunction *sys_funcs = NULL;
Function *funcs;
ReuseString *rstrs = NULL;
Symbol *symbols = NULL;
Cons *pairs = NULL;

char *block = NULL;
size_t block_depth = 0;

size_t env_sz = 0;
size_t ufuncs_sz = 0;
//size_t sfuncs_sz = 0;
size_t funcs_sz = 0;
size_t symbols_sz = 0;
size_t rstrs_sz = 0;
size_t blk_sz = 0;
size_t pairs_sz = 0;


#endif
