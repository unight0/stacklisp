#!/bin/env python3
from cmds import *

##Supplementary for lenattach###############
def lencmd(cmd):
    return (cmd, 1)

def lenchr(char):
    return (char, 1)

def lennum(i):
    return (i, 8)

def lenpushnum(i, p):
    if p == CMD_PUSHB:
        return (i, 1)
    if p == CMD_PUSHW:
        return (i, 2)
    if p == CMD_PUSHD:
        return (i, 4)
    if p == CMD_PUSHQ:
        return (i, 8)
    print("ERROR: unkown push:", p)
    exit(1)
############################################

# Attaches length to each piece of data
def lenattach(code):
    code2 = []
    i = 0
    while i < len(code):
        # if code[i] == CMD_PUSHB or code[i] == CMD_PUSHW\
        #    or code[i] == CMD_PUSHD or code[i] == CMD_PUSHQ:
        #     # Panic, this shouldn't happen
        #     if i == len(code)-1:
        #         print("#########PANIC##########")
        #         print("Code generation messed up: CMD_PUSH/CMD_ENVV at EOF")
        #         exit(-2)
        #     code2 += [lencmd(code[i]), lenpushnum(code[i+1], code[i])]
        #     i += 2
        if code[i] == CMD_ENVV or code[i] == CMD_PUSH:
            # Panic, this shouldn't happen
            if i == len(code)-1:
                print("#########PANIC##########")
                print("Code generation messed up: CMD_PUSH/CMD_ENVV at EOF")
                exit(-2)

            code2 += [lencmd(code[i]), lennum(code[i+1])]
            i+=2
            continue
            
        code2.append(lencmd(code[i]))
        i+=1
        
    return code2

# Same as lenattach(), but for data section
def lenattach_data(code):
    code2 = []
    i = 0

    while i < len(code):
        if code[i] == CMDR_ID:
            # Panic, this shouldn't happen
            if i == len(code)-1:
                print("#########PANIC##########")
                print("Code generation messed up: CMD_ID at EOF")
                exit(-2)

            # ID id
            code2 += [lencmd(CMDR_ID), lennum(code[i+1])]
            i += 2

            # Read c-string
            while code[i] != 0:
                code2 += [lenchr(code[i])]
                i += 1
            code2 += [lenchr(0)]
            i += 1
            continue

        print("UNEXPECTED", code[i])
        exit(1)
        
    return code2



# Convert code with lengths to bytes
def lencodebytes(code):
    b = []
    for e in code:
        if e[1] == 1:
            b.append(e[0])
            continue
        b += e[0].to_bytes(e[1], 'big', signed=True)
    return b

# Encodes symbols as bytes
def symbytes(syms):
    b = []
    
    for k in syms:
        b += k.encode('utf-8') + b'\0' + syms[k].to_bytes(4, 'big')
    return b
    
# Puts code with lengths into file
def fileput(filename, codebytes, symbytes):
    with open(filename, "wb") as f:
        # Virtual Machine Code
        f.write(b'VMC')
        # Symbol table begin
        f.write(b'SYM')
        f.write(bytearray(symbytes))
        # fake entry 'END':FFFFFFFF
        f.write(b'END\0\xFF\xFF\xFF\xFF')
        
        f.write(bytearray(codebytes))
