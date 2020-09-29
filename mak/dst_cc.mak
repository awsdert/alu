include char.mak
include func.mak

VC_CXX:=$(call ifin,$(CC),vc,vc,)
SYS_CXX:=$(call ifin,$(CC),cc,c++,)
GCC_CXX:=$(call ifin,$(CC),gcc,g++,)
CLANG_CXX:=$(call ifin,$(CC),clang,clang++,)
MINGW_CXX:=$(call ifin,$(CC),mingw-gcc,mingw-g++,)
MITSY_CXX:=$(call ifin,$(CC),mcc,m++,)

CXX:=$(VC_CXX)$(SYS_CXX)$(GCC_CXX)$(CLANG_CXX)$(MINGW_CXX)$(MITSY_CXX)
COP:=$(call ifin,$(CC),vc,/,-)
GDB:=$(call ifin,$(CC),vc,cdb,gdb)

F_shared:=$(call ifin,$(CC),vc,,-shared)
_F_Wl_rpath=-Wl,-rpath,$1
F_Wl_rpath=$(call ifin,$(CC),vc,,$(call _F_Wl_rpath,$1))
F_fPIC:=$(call ifin,$(CC),vc,,-fPIC)
F_fPIE:=$(call ifin,$(CC),vc,,-fPIE)
F_g_gdb:=$(call ifin,$(CC),vc,,-ggdb)
F_pg:=$(call ifin,$(CC),vc,,-pg)
F_pedantic:=$(call ifin,$(CC),vc,,-pedantic)
F_L:=$(COP)L
F_l:=$(COP)l
F_I:=$(COP)I
F_D:=$(COP)D
F_O:=$(COP)O

GPROF:=$(call ifin,$(CC),vc,echo Failed to profile,gprof)

DBG_FLAGS:=$(F_D) NDEBUG $(F_O)3
DBG_APP:=
DBG_SFX:=
PFL_FLAGS:=
PFL_APP:=
PFL_SFX:=

ifeq '$(filter profile,$(MAKECMDGOALS))' 'profile'
PFL_FLAGS:=$(F_pg)
PFL_APP:=$(GPROF)
PFL_SFX:=_p
endif

ifeq '$(filter debug,$(MAKECMDGOALS))' 'debug'
DBG_FLAGS:=$(F_g_gdb) $(F_D) _DEBUG $(F_O)0
DBG_APP:=$(GDB) $(COP)ex run
DBG_SFX:=_d
# Debugging takes precedence over profiling
PFL_FLAGS:=
PFL_APP:=
PFL_SFX:=
endif

ifeq '$(filter gede,$(MAKECMDGOALS))' 'gede'
DBG_FLAGS:=$(F_g_gdb) $(F_D) _DEBUG $(F_O)0
DBG_APP:=$(GDB) $(COP)ex run
DBG_SFX:=_d
# Debugging takes precedence over profiling
PFL_FLAGS:=
PFL_APP:=
PFL_SFX:=
endif

ifeq '$(filter check,$(MAKECMDGOALS))' 'check'
DBG_FLAGS:=$(F_g_gdb) $(F_D) _DEBUG $(F_O)0
DBG_APP:=$(GDB) $(COP)ex run
DBG_SFX:=_d
# Debugging takes precedence over profiling
PFL_FLAGS:=
PFL_APP:=
PFL_SFX:=
endif
