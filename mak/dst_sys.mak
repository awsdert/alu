include char.mak
include func.mak

DST_SYS?=linux
LINUX_DST_BIN_SFX:=$(call ifin,$(DST_SYS),linux,.AppImage,)
WIN16_DST_BIN_SFX:=$(call ifin,$(DST_SYS),win32,.16.exe,)
WIN32_DST_BIN_SFX:=$(call ifin,$(DST_SYS),win32,.32.exe,)
WIN64_DST_BIN_SFX:=$(call ifin,$(DST_SYS),win64,.64.exe,)
DST_BIN_SFX=$(strip $(LINUX_DST_BIN_SFX) $(call MSWIN_VARS,BIN_SFX))

LINUX_DST_LIB_PFX:=$(call ifin,$(DST_SYS),linux,lib,)
WIN16_DST_LIB_PFX:=
WIN32_DST_LIB_PFX:=
WIN64_DST_LIB_PFX:=
DST_LIB_PFX=$(strip $(LINUX_DST_LIB_PFX) $(call MSWIN_VARS,LIB_PFX))

LINUX_DST_LIB_SFX:=$(call ifin,$(DST_SYS),linux,.so,)
WIN16_DST_LIB_SFX:=$(call ifin,$(DST_SYS),win32,.16.dll,)
WIN32_DST_LIB_SFX:=$(call ifin,$(DST_SYS),win32,.32.dll,)
WIN64_DST_LIB_SFX:=$(call ifin,$(DST_SYS),win64,.64.dll,)
DST_LIB_SFX=$(strip $(LINUX_DST_LIB_SFX) $(call MSWIN_VARS,LIB_SFX))

LINUX_DST_OBJ_SFX:=$(call ifin,$(DST_SYS),linux,.o,)
WIN16_DST_OBJ_SFX:=$(call ifin,$(DST_SYS),win32,.16.obj,)
WIN32_DST_OBJ_SFX:=$(call ifin,$(DST_SYS),win32,.32.obj,)
WIN64_DST_OBJ_SFX:=$(call ifin,$(DST_SYS),win64,.64.obj,)
DST_OBJ_SFX=$(strip $(LINUX_DST_OBJ_SFX) $(call MSWIN_VARS,OBJ_SFX))

LINUX_DST_DEF:=$(call ifin,$(DST_SYS),linux,,)
WIN16_DST_DEF:=$(call ifin,$(DST_SYS),win32,_WIN16 UNIC_FALLBACK,)
WIN32_DST_DEF:=$(call ifin,$(DST_SYS),win32,_WIN32 UNIC_FALLBACK,)
WIN64_DST_DEF:=$(call ifin,$(DST_SYS),win64,_WIN64 UNIC_FALLBACK,)
DST_DEF=$(strip $(LINUX_DST_DEF) $(call MSWIN_VARS,DEF))

.LIBPATTERNS:=$(DST_LIB_PFX)%.$(DST_LIB_SFX) $(DST_LIB_PFX)%.a
