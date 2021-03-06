include char.mak
include func.mak
include goals.mak

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
F_santize-address:=$(call ifin,$(CC),vc,,-fsanitize=address)
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
SPD_FLAGS:=
SPD_APP:=
SPD_SFX:=
CHK_FLAGS:=

ifeq '$(filter profile,$(PRJ_GOALS))' 'profile'
PFL_FLAGS:=$(F_pg)
PFL_APP:=$(GPROF)
PFL_SFX:=_p
endif

ifneq '$(GOALS4SPEED)' ''
SPD_FLAGS:=$(F_O)3
SPD_APP:=
SPD_SFX:=_O3
# Build is meant for speed, profiling goes against that
PFL_FLAGS:=
PFL_APP:=
PFL_SFX:=
endif

ifneq '$(GOAL2DBUG))' ''
DBG_FLAGS:=$(F_g_gdb) $(F_D) _DEBUG $(F_O)
DBG_APP:=$(GDB) $(COP)ex run
DBG_SFX:=_d
# Debugging takes precedence over profiling & speed
PFL_FLAGS:=
PFL_APP:=
PFL_SFX:=
endif

ifneq '$(GOALS4GEDE)' ''
DBG_FLAGS:=$(F_g_gdb) $(F_D) _DEBUG $(F_O)0
DBG_APP:=$(GDB) $(COP)ex run
DBG_SFX:=_d
# Debugging takes precedence over profiling & speed
PFL_FLAGS:=
PFL_APP:=
PFL_SFX:=
SPD_FLAGS:=
SPD_APP:=
SPD_SFX:=
endif

ifneq '$(GOALS4VALGRIND)' ''
DBG_FLAGS:=$(F_g_gdb) $(F_D) _DEBUG $(F_O)0
DBG_APP:=$(GDB) $(COP)ex run
DBG_SFX:=_d
# Debugging takes precedence over profiling & speed
PFL_FLAGS:=
PFL_APP:=
PFL_SFX:=
SPD_FLAGS:=
SPD_APP:=
SPD_SFX:=
endif

ifeq '$(filter check,$(PRJ_GOALS))' 'check'
# Check needs libraries
# If this doesn't compile for you then add $(F_l) subunit between check and
# $(F_l) rt, not yet figured out how to detect this situation
CHK_FLAGS:=$(F_l) check  $(F_l) rt $(F_l) pthread $(F_l) m
endif
