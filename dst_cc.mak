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

F_shared:=$(call ifin,$(CC),vc,,-shared)
F_Wl_rpath:=$(call ifin,$(CC),vc,,-Wl,rpath)
F_fPIC:=$(call ifin,$(CC),vc,,-fPIC)
F_fPIE:=$(call ifin,$(CC),vc,,-fPIE)
F_g_gdb:=$(call ifin,$(CC),vc,,-ggdb)
F_L:=$(COP)L
F_l:=$(COP)l
F_I:=$(COP)I
F_D:=$(COP)D

DBG_FLAGS:=$(F_D) NDEBUG
DBG_SFX:=

# FIXME: Map -ggdb more appropriatly
ifeq '$(filter debug,$(MAKECMDGOALS))' 'debug'
DBG_FLAGS:=$(F_g_gdb) $(F_D) _DEBUG
DBG_SFX:=_d
endif

ifeq '$(filter gede,$(MAKECMDGOALS))' 'gede'
DBG_FLAGS:=$(F_g_gdb) $(F_D) _DEBUG
DBG_SFX:=_d
endif
