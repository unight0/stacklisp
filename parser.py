#!/bin/env python3
from asttypes import *

# Ast:
# + Ast type
# + Value (string)
# + Children (list)
class Ast:
    def __init__(self, t, ch=[], v=''):
        self.t = t
        self.ch = ch
        self.v = v


def expect(i, l, t):
    if i >= len(l):
        print("Unexpected EOF")
        exit(-2)
        
    if l[i][0] != t:
        print(f"Unexpected lexem at {i}:{l[i][1]}; expected {t}",)
        exit(-3)
    return i + 1

def take_rlist(i, l) -> (Ast, int):
    rlist = []
    while l[i][0] != 'RPAREN':
        r, i = take_expr(i, l)
        rlist.append(r)
    i = expect(i, l, 'RPAREN')
    return rlist, i

def take_block(i, l) -> (Ast, int):
    i = expect(i, l, 'BLOCK')
    e = Ast(ATYP_BLOCK)
    e.ch, i = take_rlist(i, l)
    return e, i

def take_fun(i, l) -> (Ast, int):
    i = expect(i, l, 'LAMBDA')
    args, i = take_list(i, l)
    body, i = take_expr(i, l)
    i = expect(i, l, 'RPAREN')
    return Ast(ATYP_FUN, [args, body]), i

def take_list(i, l) -> (Ast, int):
    e = Ast(ATYP_LIST)
    i = expect(i, l, 'LPAREN')
    e.ch, i = take_rlist(i, l)
    return e, i
    
def take_appl(i, l) -> (Ast, int):
    e = Ast(ATYP_APPL)
    fun, i = take_expr(i, l)
    e.ch, i = take_rlist(i, l)
    e.ch = [fun] + e.ch
    return e, i

def take_expr(i, l) -> (Ast, int):
    if l[i][0] == 'NUMBER' or l[i][0] == 'STR' or l[i][0] == 'NAME':
        return Ast(ATYP_ATOM,[],l[i]), i+1
    
    i = expect(i, l, 'LPAREN')
    if l[i][0] == 'BLOCK':
        return take_block(i, l)
    elif l[i][0] == 'LAMBDA':
        return take_fun(i, l)
        
    return take_appl(i, l)

def parse(lexems):
    root = Ast(ATYP_ROOT)

    i = 0
    while i < len(lexems):
        e, i = take_expr(i, lexems)
        root.ch.append(e)

    return root

def dump_ast(ast):
    if ast.t == ATYP_ATOM: print(ast.v); return
    print(ASTTYPE[ast.t],end=' ')
    if len(ast.ch) == 0: return
    print('[')
    for c in ast.ch:
        dump_ast(c)
    print(']')

