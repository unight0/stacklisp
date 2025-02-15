#!/bin/env python3
from asttypes import *
from cmds import *


def tr_expr(e):
    if e.t == ATYP_ATOM:
        return tr_atom(e)
    if e.t == ATYP_BLOCK:
        return tr_block(e)
    if e.t == ATYP_APPL:
        return tr_appl(e)
    if e.t == ATYP_FUN:
        return tr_fun(e)
    if e.t == ATYP_IF:
        return tr_if(e)
    print(f"Unexpected AST type: {ASTTYPE[e.t]}")

def tr_atom(e):
    if e.v[0] == 'NUMBER':
        return [(CMD_PUSH, int(e.v[1]))]
    # TODO: change later
    if e.v[0] == 'STR':
        return [(CMD_PUSH, e.v[1])]
    assert(e.v[0] == 'NAME')
    return [(CMD_ENVV, e.v[1])]

# Translate lambda into bytecode
# --> [ cmd1 cmd2 ... ] argnum L
def tr_fun(e):
    argnum = len(e.ch[0].ch)
    body = tr_body(e.ch[1], e.ch[0].ch)
    return [CMD_BLKB] + body + [CMD_BLKE] + [(CMD_PUSH, argnum), CMD_LAM]

# Translate if statement into bytecode
# --> [ otherwise... ] 0 L [ then... ] 0 L cond IF
def tr_if(e):
    cond = tr_expr(e.ch[0])
    then = tr_expr(e.ch[1])
    otherwise = tr_expr(e.ch[2])

    return\
[CMD_BLKB] + otherwise + [CMD_BLKE] + [(CMD_PUSH, 0), CMD_LAM] +\
[CMD_BLKB] + then + [CMD_BLKE] + [(CMD_PUSH, 0), CMD_LAM] +\
cond + [CMD_IF]

# NOTE: called only from tr_fun()
# Replaces every occurence of a variable with a
# 'get' from a data stack
def tr_body(e, args):
    return substargs(tr_expr(e), args)

def substargs(e, args):
    ad = makedict(args)
    i = 0
    r = []
    while i < len(e):
        if type(e[i]) == tuple:
            if e[i][0] == CMD_ENVV:
                if e[i][1] in ad:
                    r += [(CMD_PUSH, ad[e[i][1]]), CMD_D]
                    i += 1
                    continue
        r.append(e[i])
        i += 1

    return r

def makedict(lst):
    d = {}
    for (i,e) in enumerate(lst):
        d[e.v[1]] = i
    return d
    
def tr_appl(e):
    f = tr_expr(e.ch[0])
    r = []
    for a in e.ch[1:]:
        r += tr_expr(a)
        
    argc = len(e.ch[1:])

    # Ignore this monstrosity
    if argc == 1:
        return r + [CMD_TD] + f + [(CMD_PUSH, argc), CMD_APPL]
    return r + [(CMD_PUSH, argc), CMD_ND] + f + [(CMD_PUSH, argc), CMD_APPL]

def tr_block(e):
    r = [CMD_BLKB]
    for c in e.ch:
        r.append(tr_expr(c))
    return r + [CMD_BLKE]

def translate(ast):
    assert(ast.t == ATYP_ROOT)
    r = []
    for e in ast.ch:
        r += tr_expr(e)
    return r

def dict_has_val(d, v):
    for k in d:
        if d[k] == v: return k
    return -1

# Generates symbol table
def symblgen(t):
    symbls = {}
    r = []
    i = 0
    for e in t:
        match e:
            case (cmd, data):
                if cmd != CMD_ENVV:
                    r.append((cmd, data))
                    continue
                if not data in symbls:
                    symbls[data] = i
                    i += 1
                r.append((CMD_ENVV, symbls[data]))
                continue
            case data:
                r.append(data)
    return (symbls, r)



# Separates data (strings) from code, generates references to
# it and replaces push(str) to ENVV
def genrefs(t):
    d = {}
    r = -1
    nt = []

    for e in t:
        if type(e) != tuple:
            nt.append(e)
            continue

        cmd, data = e
        
        if cmd != CMD_PUSH or type(data) != str:
            nt += [cmd, data]
            continue

        # Already in the dictionary
        if (k:=dict_has_val(d, data)) != -1:
            nt += [CMD_ENVV, k]
            continue

        # Add new entry
        d[r] = data
        nt += [CMD_ENVV, r]
        r -= 1
        
    return (d, nt)


# ['a', 'b', 'c'] -> [0, ord('a'), 0, ord('b'), 0, ord('c')]
# Note: 0 is prefix before a char;
# Needed because the CMD_ED must be recognized
def genchars(l):
    nl = []
    for c in l:
        nl += [0, ord(c)]
    return nl

# Takes dictionary, creates data[string] section in the beginning of the code
# Note: string data is in form of [CMDR_ID, (id), CMDR_BD, ..., CMD_ED]
def gendata(d):
    dt = []
    for k in d:
        v = d[k]
        dt += [CMDR_ID, k]
        dt += [CMDR_BD] + genchars(list(v)) + [CMDR_ED]
    return dt

# (CMD_ENVV, STR) -> CMD_ENVV, ID (from symbltable)
# (CMD_ENVV, NUM) -> CMD_ENVV, NUM
def link(t, symbltable):
    nt = []
    for e in t:
        match e:
            case (cmd, data):
                if cmd!= CMD_ENVV: continue
                nt += [CMD_ENVV, symbltable[data]]
                continue
        nt += [e]

    return nt



# (CMD_PUSH, N) -> CMD_PUSH, N
def naked_push(t):
    nt = []
    for e in t:
        if type(e) == tuple:
            if e[0] == CMD_PUSH:
                nt += [e[0], e[1]]
                continue
        nt += [e]
    return nt
