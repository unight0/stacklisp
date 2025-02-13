#!/bin/env python3
from lexer import *
from parser import *
from codegen import *
from binput import *

# Output translated code in a neat way
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



# Main section
def main(files):
    for f in files:
        contents = ""
        with open(f, "r") as c:
            contents = c.read()
            
        # Lex source code
        l = lex(contents)
        
        p = parse(l)
        
        # Do most of translation
        tr = translate(p)

        # Output neat version
        dump(tr)
        
        # Generate symbols
        symbls, tr = symblgen(tr)
        
        # Generate data[string] symbols, replace strings by IDs(references)
        dsym, tr = genrefs(tr)
        
        # Generate declarations for data IDs
        # Basically define strings at the top of the file, and then just reference to them by id
        # Note: during compilation, some preprocessing may be done to replace CMD_ENVV, (ID)
        # by (PTR) for optimization purposes
        datasection = gendata(dsym)
        
        # Final code
        code = datasection + tr

        print("----------------------------------------")
        print(code)
        # Don't forget symbol table!
        print(symbls)


        print(datasection)
        # Attach lengths to code
        lcode = lenattach_data(datasection) + lenattach(tr)

        # Get codebytes
        cb = lencodebytes(lcode)

        # Convert symbol table into bytes
        # Note (k, v) --> (v, k) for easier reading
        sb = symbytes(symbls)

        fileput("out.bin", cb, sb)

            

if __name__ == "__main__":
    from sys import argv
    if len(argv) < 2:
        print("Not enough arguments")
        exit(-1)
    main(argv[1:])
