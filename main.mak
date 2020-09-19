include func.mak
include dst_sys.mak
include dst_cc.mak

PRJ:=ALU
PRJ_SFX:=$(DBG_SFX)$(PFL_SFX)
ALL_GOALS:=info objects build run debug gede clean rebuild rebuildall profile

PRJ_SRC_DIR:=
PRJ_INC_DIR:=
PRJ_LIB_DIR:=
PRJ_BIN_DIR:=
PRJ_EXT_DIR:=cloned

FBSTDC_DIR:=$(PRJ_EXT_DIR)/fbstdc
FBSTDC_INC_DIR:=$(FBSTDC_DIR)/include

RPATH_FLAG=-Wl,-rpath=./

PRJ_SRC_FILES:=test.c $(wildcard $(PRJ_SRC_DIR)alu*.c)
PRJ_INC_FILES:=$(wildcard $(PRJ_INC_DIR)*.h)
PRJ_OBJ_FILES:=$(PRJ_SRC_FILES:%.c=%)

PRJ_BIN_OBJ_FILES:=test
PRJ_LIB_OBJ_FILES:=$(filter-out $(PRJ_BIN_OBJ_FILES),$(PRJ_OBJ_FILES))

PRJ_DST_BIN:=alu$(PRJ_SFX)$(DST_BIN_SFX)
PRJ_DST_LIB:=$(DST_LIB_PFX)alu$(PRJ_SFX)$(DST_LIB_SFX)
PRJ_DST_OBJ:=$(PRJ_OBJ_FILES:%=%$(PRJ_SFX)$(DST_OBJ_SFX))
PRJ_TARGETS:=$(PRJ_DST_OBJ) $(PRJ_DST_LIB) $(PRJ_DST_BIN)

ERR_FLAGS:=$(COP)Wall $(COP)Wextra $(F_pedantic)
INC_FLAGS:=-I $(FBSTDC_INC_DIR)
SRC_FLAGS:=$(DBG_FLAGS) $(PFL_FLAGS) -fPIC $(ERR_FLAGS) $(INC_FLAGS)
LIB_FLAGS:=$(DBG_FLAGS) $(PFL_FLAGS) -fPIC -shared
BIN_FLAGS:=$(DBG_FLAGS) $(PFL_FLAGS) -fPIE $(COP)L .

COMPILE_EXE=$(CC) $(BIN_FLAGS) $1 $(COP)o $2 $3 $(RPATH_FLAG) $(COP)l alu$(PRJ_SFX)
COMPILE_DLL=$(CC) $(LIB_FLAGS) $1 $(COP)o $2 $3 $(RPATH_FLAG)
COMPILE_OBJ=$(CC) $(SRC_FLAGS) $1 $(COP)o $2 -c $3

$(info PRJ_SRC_FILES = '$(PRJ_SRC_FILES)')

$(call mkdir,$(PRJ_EXT_DIR))
$(call github_clone,$(FBSTDC_DIR),$(PRJ_EXT_DIR),awsdert/fbstdc)

$(info Checking 3rd Party libraries are upto date)
$(info $(call rebase,$(FBSTDC_DIR)))
$(info $(shell $(call rebase,$(FBSTDC_DIR))))
$(info Finished checking)

$(info PRJ_DST_BIN=$(PRJ_DST_BIN))
$(info PRJ_DST_LIB=$(PRJ_DST_LIB))

info: .FORCE
	@echo CC=$(CC)
	@echo CXX=$(CXX)
	@echo COP=$(COP)
	@echo PRJ=$(PRJ)
	@echo PRJ_DST_BIN=$(PRJ_DST_BIN)
	@echo PRJ_DST_LIB=$(PRJ_DST_LIB)
	@echo PRJ_SRC_FILES=$(PRJ_SRC_FILES)
	@echo PRJ_INC_FILES=$(PRJ_INC_FILES)
	@echo PRJ_OBJ_FILES=$(PRJ_OBJ_FILES)
	@echo PRJ_BIN_OBJ_FILES=$(PRJ_BIN_OBJ_FILES)
	@echo PRJ_LIB_OBJ_FILES=$(PRJ_LIB_OBJ_FILES)
	@echo Goals make can process for $(PROJECT):
	@echo $(TAB_CHAR) all $(ALL_GOALS)

all:
	make build
	make debug
	make profile

rebuildall: clean all

rebuild: clean build

clean:
	rm -f *.AppImage *.exe *.so *.dll *.o *.obj

run: $(PRJ_TARGETS)
	$(DBG_APP) $(PFL_APP) ./$(PRJ_DST_BIN)
	
debug: $(PRJ_TARGETS)

profile: $(PRJ_TARGETS)

gede: debug
	gede --args ./$(PRJ_DST_BIN)

build: $(PRJ_TARGETS)

objects: $(PRJ_DST_OBJ)

%$(PRJ_SFX).AppImage: libalu$(PRJ_SFX).so $(PRJ_BIN_OBJ_FILES:%=%$(PRJ_SFX).o)
	$(call COMPILE_EXE,,$@,$(PRJ_BIN_OBJ_FILES:%=%$(PRJ_SFX).o))

%$(PRJ_SFX).16.exe: alu$(PRJ_SFX).16.dll $(PRJ_BIN_OBJ_FILES:%=%$(PRJ_SFX).16.obj)
	$(call COMPILE_EXE,,$@,$(PRJ_BIN_OBJ_FILES:%=%$(PRJ_SFX).16.obj))

%$(PRJ_SFX).32.exe: alu$(PRJ_SFX).32.dll $(PRJ_BIN_OBJ_FILES:%=%$(PRJ_SFX).32.obj)
	$(call COMPILE_EXE,,$@,$(PRJ_BIN_OBJ_FILES:%=%$(PRJ_SFX).32.obj))

%$(PRJ_SFX).64.exe: alu$(PRJ_SFX).64.dll $(PRJ_BIN_OBJ_FILES:%=%$(PRJ_SFX).64.obj)
	$(call COMPILE_EXE,,$@,$(PRJ_BIN_OBJ_FILES:%=%$(PRJ_SFX).64.obj))
	
lib%$(PRJ_SFX).so: $(PRJ_LIB_OBJ_FILES:%=%$(PRJ_SFX).o)
	$(call COMPILE_DLL,,$@,$^)

%$(PRJ_SFX).16.dll: $(PRJ_LIB_OBJ_FILES:%=%$(PRJ_SFX).16.obj)
	$(call COMPILE_DLL,,$@,$^)

%$(PRJ_SFX).32.dll: $(PRJ_LIB_OBJ_FILES:%=%$(PRJ_SFX).32.obj)
	$(call COMPILE_DLL,,$@,$^)

%$(PRJ_SFX).64.dll: $(PRJ_LIB_OBJ_FILES:%=%$(PRJ_SFX).64.obj)
	$(call COMPILE_DLL,,$@,$^)

%$(PRJ_SFX).o: %.c
	$(call COMPILE_OBJ,,$@,$<)

%$(PRJ_SFX).16.obj: %.c
	$(call COMPILE_OBJ,,$@,$<)

%$(PRJ_SFX).32.obj: %.c
	$(call COMPILE_OBJ,,$@,$<)

%$(PRJ_SFX).64.obj: %.c
	$(call COMPILE_OBJ,,$@,$<)
	
%.c: $(PRJ_INC_FILES)

%.mak.o:
	@echo Why '$<'?

.PHONY: all $(ALL_GOALS)

.FORCE:
