# alu
Library modeled after the Arithmetic Logic Unit built into CPUs

# About
Bignum type library with the goal of being efficient at it's core

#Compiling
Currently these are the important variants that can be run from the source
directories

##Check info being found/constructed
make
make info

##Check source code with cppcheck
make lint

##Just build the executable
make check
make test

##Build & run the executable
make check.run
make test.run

##Build & run under gede (GUI for gdb)
make check.gede
make test.gede

##Build & run under valgrind
make check.valgrind
make test.valgrind

#Packaging
This project is not got a stable API yet so don't, instead just look for
variables in the makefiles or mak/*.mak files that would interfere with
packaging once it is stable and raise an issue, I'll focus on it once I have
marked the project as beta instead of it's current alpha state
