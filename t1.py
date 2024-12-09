#!/bin/env python3
ALTERED = {
    '"': '"',
    '\\': '\\',
    'n': '\n'
}

def altered(char):
    if char in ALTERED:
        return ALTERED[char]
    print("\\%s is not a valid control sequence" % char)
    exit(1)

def skip_spaces(i, code):
    while code[i].isspace():
        i += 1
        if i >= len(code):
            break
    return i

def skip_comment(i, code):
    while code[i] != '\n':
        i += 1
        if i > len(code):
            break
    return i
        
def lex(code):
    lexems = []
    i = 0
    while i < len(code):
        # Skip spaces
        if code[i].isspace():
            i=skip_spaces(i, code)
            continue
        # Skip comments
        if code[i] == ';':
            i=skip_comment(i, code)
            continue
        # Parens
        if code[i] == '(':
            lexems.append(('LPAREN','('))
            i += 1
        elif code[i] == ')':
            lexems.append(('RPAREN',')'))
            i += 1
        # Easy quote
        elif code[i] == "'":
            lexems.append(('EQUOTE',"'"))
            i += 1
        # Backward quote
        elif code[i] == '`':
            lexems.append(('BQUOTE',"`"))
            i += 1
        # Char
        elif code[i] == '?':
            if i+1 >= len(code):
                print("Got EOF while parsing char")
                exit(1)
            ch = code[i+1]
            num = str(ord(ch))
            lexems.append(('NUMBER',num))
            i += 2
        # Comma
        elif code[i] == ',':
            # ,@ lexem
            #if i+1 < len(code):
            #    if code[i+1] == '@':
            #        lexems.append(('AT', ',@'))
            #        i += 2
            #        continue
            lexems.append(('COMMA',","))
            i += 1

        # Number
        elif code[i].isnumeric() or code[i] == '-':
            num = code[i]
            i += 1
            # Floats also
            has_dot = False
            while code[i].isnumeric() or (code[i] == '.' and not has_dot):
                num += code[i]
                i += 1
                if i >= len(code):
                    break
            # Not num
            if num == '-':
                lexems.append(('NAME',num))
            else:
                lexems.append(('NUMBER',num))
        # String
        elif code[i] == '"':
            string = ''
            alter = False
            i += 1
            while code[i] != '"' or alter:
                # \ stuff
                if alter:
                    string += altered(code[i])
                    alter = False
                else:
                    if code[i] == '\\':
                        alter = True
                    else:
                        string += code[i]
                i += 1
                if i >= len(code):
                    break
            # Skip '"'
            if not i >= len(code):
                i += 1
            lexems.append(('STR',string))
        # Name
        else:
            name = ''
            while not code[i].isspace() and not (code[i] == '(' or code[i] == ')'):
                name += code[i]
                i += 1
                if i >= len(code):
                    break
            if name.lower() == 'lambda':
                lexems.append(('LAMBDA',name))

            elif name.lower() == 'quote':
                lexems.append(('QUOTE',name))

            #elif name.lower() == 'defun':
            #    lexems.append(('DEFUN',name))

            elif name.lower() == 'defvar':
                lexems.append(('DEFVAR',name))

            #elif name.lower() == 'defmacro':
            #    lexems.append(('DEFMACRO',name))
                
            elif name.lower() == 'block':
                lexems.append(('BLOCK',name))

            #elif name.lower() == 'if':
            #    lexems.append(('IF',name))

            #elif name.lower() == 'cond':
            #    lexems.append(('COND',name))

            #elif name.lower() == 'or':
            #    lexems.append(('OR',name))

            #elif name.lower() == 'and':
            #    lexems.append(('AND',name))
                
            else:
                lexems.append(('NAME',name))
    return lexems


# Ast:
# + Ast type
# + Value (string)
# + Children (list)
class Ast:
    def __init__(self, t, ch=[], v=''):
        self.t = t
        self.ch = ch
        self.v = v

ATYP_ROOT, ATYP_BLOCK, ATYP_APPL, ATYP_LIST, ATYP_FUN, ATYP_ATOM = range(6)

ASTTYPE = {
    ATYP_ROOT: 'Root',
    ATYP_BLOCK: 'Block',
    ATYP_APPL: 'Function application',
    ATYP_LIST: 'List',
    ATYP_FUN: 'Function',
    ATYP_ATOM: 'Atom'
}

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


#class Env:
#    # df == definitions
#    # st == stack
#    # dst == data stack
#    def __init__(self, df={}, st=[], dst=[]):
#        self.df = df
#        self.st = st
#        self.dst = dst

        
# Stack machine commands
CMD_PUSH, CMD_BLKB, CMD_BLKE, CMD_LAM, CMD_DROP, CMD_TD, CMD_FD, CMD_D, CMD_ND, CMD_APPL, CMD_ENVV = range(11)

CMDSTR = {
    CMD_BLKB: '[',
    CMD_BLKE: ']',
    CMD_LAM:  'L',
    CMD_DROP: 'drop',
    CMD_TD:   ">D",
    CMD_ND:   '>ND',
    CMD_FD:   "D>",
    CMD_D:    'D',
    CMD_APPL: 'A',
}

def tr_expr(e):
    if e.t == ATYP_ATOM:
        return tr_atom(e)
    if e.t == ATYP_BLOCK:
        return tr_block(e)
    if e.t == ATYP_APPL:
        return tr_appl(e)
    if e.t == ATYP_FUN:
        return tr_fun(e)
    print(f"Unexpected AST type: {ASTTYPE[e.t]}")

def tr_atom(e):
    if e.v[0] == 'NUMBER':
        return [(CMD_PUSH, e.v[1])]
    # TODO: change later
    if e.v[0] == 'STR':
        return [(CMD_PUSH, e.v[1])]
    assert(e.v[0] == 'NAME')
    return [(CMD_ENVV, e.v[1])]

def tr_fun(e):
    argnum = len(e.ch[0].ch)
    body = tr_body(e.ch[1], e.ch[0].ch)
    return [CMD_BLKB] + body + [CMD_BLKE] + [(CMD_PUSH, argnum), CMD_LAM]

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

def dump(output):
    for cmd in output:
        if type(cmd) == tuple:
            if cmd[0] == CMD_PUSH:
                print(cmd[1], end=' ')
            elif cmd[0] == CMD_ENVV:
                print(f"({cmd[1]})", end=' ')
            continue
        print(CMDSTR[cmd], end=' ')
    print()

def main(files):
    for f in files:
        with open(f, "r") as c:
            dump(translate(parse(lex(c.read()))))

if __name__ == "__main__":
    from sys import argv
    if len(argv) < 2:
        print("Not enough arguments")
        exit(-1)
    main(argv[1:])
