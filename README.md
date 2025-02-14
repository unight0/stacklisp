# Stacklisp

## What is this?
This is a simple pet project. Its goals are to:
1. Translate lisp code into stackmachine code
2. Run stackmachine code
3. (optional) Compile stackmachine code

## Current state
This project is not yet fully finished. T1 translates simple lisp code
into bytecode, writes it to file. VM has been implemented.

The current short-term goal is to expand the list of supported functions.
The next short-term goal is to expand supported syntax in t1.

## How to use?

```sh
# Translates into stackmachine code, outputs in readable format, writes the bytecode to out.bin
./t1 <file>
# Reads bytecode from file, outputs in readable format
./vm read <file>
# Executes bytecode
./vm run <file>
```
