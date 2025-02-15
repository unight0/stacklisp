
CCFLAGS= -Wall -Werror -Wpedantic -g #-DDEBUG_STACK
OUT=vm

$(OUT): vm.o utils.o sysfuncs.o stack.o
	gcc *.o -o $(OUT)

vm.o utils.o sysfuncs.o stack.o: vm.c utils.c sysfuncs.c stack.c
	gcc -c $(CCFLAGS) *.c
