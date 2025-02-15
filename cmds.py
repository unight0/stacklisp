#!/bin/env python


# Stack machine commands
CMD_PUSH, CMD_BLKB, CMD_BLKE, CMD_LAM, CMD_DROP, CMD_TD, CMD_FD, CMD_D, CMD_ND, CMD_APPL, CMD_ENVV, CMDR_BD, CMDR_ED, CMDR_ID, CMD_IF, CMD_PUSHB, CMD_PUSHW, CMD_PUSHD, CMD_PUSHQ = range(19) # Only used in raw code

CMDSTR = {
    CMD_BLKB: '[',
    CMD_BLKE: ']',
    CMD_LAM:  'L',
    CMD_DROP: 'DROP',
    CMD_TD:   ">D",
    CMD_ND:   '>ND',
    CMD_FD:   "D>",
    CMD_D:    'D',
    CMD_APPL: 'A',
    CMD_IF:   'IF'
}
