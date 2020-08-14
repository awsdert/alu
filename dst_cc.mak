include char.mak
include func.mak

VC_CXX:=$(call ifin,$(CC),vc,vc,)
SYS_CXX:=$(call ifin,$(CC),cc,c++,)
GCC_CXX:=$(call ifin,$(CC),gcc,g++,)
CLANG_CXX:=$(call ifin,$(CC),clang,clang++,)
MINGW_CXX:=$(call ifin,$(CC),mingw-gcc,mingw-g++,)
MITSY_CXX:=$(call ifin,$(CC),mcc,m++,)

CXX:=$(VC_CXX) $(SYS_CXX) $(GCC_CXX) $(CLANG_CXX) $(MINGW_CXX) $(MITSY_CXX)
COP:=$(call ifin,$(CC),vc,/,-)
