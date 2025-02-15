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
                #print("ERROR: support for floating-point numbers hasn't been added yet!")
                #exit(1)
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

            elif name.lower() == 'if':
                lexems.append(('IF',name))

            #elif name.lower() == 'cond':
            #    lexems.append(('COND',name))

            #elif name.lower() == 'or':
            #    lexems.append(('OR',name))

            #elif name.lower() == 'and':
            #    lexems.append(('AND',name))
                
            else:
                lexems.append(('NAME',name))
    return lexems
