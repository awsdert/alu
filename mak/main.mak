TOP_DIR:=../
PRJ_MAK_DIR:=$(TOP_DIR)mak
PRJ_CHK_DIR:=$(TOP_DIR)tests
PRJ_SRC_DIR:=$(TOP_DIR)src
PRJ_INC_DIR:=$(TOP_DIR)include
PRJ_LIB_DIR:=$(TOP_DIR)lib
PRJ_BIN_DIR:=$(TOP_DIR)bin
PRJ_EXT_DIR:=$(TOP_DIR)cloned

include $(PRJ_MAK_DIR)/func.mak
include $(PRJ_MAK_DIR)/dst_sys.mak
include $(PRJ_MAK_DIR)/dst_cc.mak

PRJ:=ALU
PRJ_SFX:=$(DBG_SFX)$(PFL_SFX)
ALL_GOALS:=info objects build run debug gede clean rebuild rebuildall profile

UNIC_DIR:=$(PRJ_EXT_DIR)/unic
UNIC_INC_DIR:=$(UNIC_DIR)/include

RPATH_FLAG=-Wl,-rpath=$(PRJ_LIB_DIR)

PRJ_CHK_FILES:=$(wildcard $(PRJ_CHK_DIR)/*.c)
PRJ_SRC_FILES:=$(PRJ_CHK_FILES) $(wildcard $(PRJ_SRC_DIR)/*.c)
PRJ_INC_FILES:=$(wildcard $(PRJ_INC_DIR)/*.h)
PRJ_OBJ_FILES:=$(PRJ_SRC_FILES:%.c=%)

PRJ_CHK_OBJ_FILES:=$(PRJ_CHK_DIR)/check_alu
PRJ_BIN_OBJ_FILES:=$(PRJ_CHK_DIR)/test
PRJ_APP_OBJ_FILES:=$(PRJ_CHK_OBJ_FILES) $(PRJ_BIN_OBJ_FILES)
PRJ_LIB_OBJ_FILES:=$(filter-out $(PRJ_APP_OBJ_FILES),$(PRJ_OBJ_FILES))

PRJ_DST_CHK:=$(PRJ_BIN_DIR)/check_alu$(PRJ_SFX)$(DST_BIN_SFX)
PRJ_DST_BIN:=$(PRJ_BIN_DIR)/alu$(PRJ_SFX)$(DST_BIN_SFX)
PRJ_DST_LIB:=$(PRJ_LIB_DIR)/$(DST_LIB_PFX)alu$(PRJ_SFX)$(DST_LIB_SFX)
PRJ_DST_OBJ:=$(PRJ_OBJ_FILES:%=%$(PRJ_SFX)$(DST_OBJ_SFX))
PRJ_TARGETS:=$(PRJ_DST_OBJ) $(PRJ_DST_LIB) $(PRJ_DST_BIN)

ERR_FLAGS:=$(COP)Wall $(COP)Wextra $(F_pedantic)
INC_FLAGS:=-I $(UNIC_INC_DIR) -I $(PRJ_INC_DIR)
SRC_FLAGS:=$(DBG_FLAGS) $(PFL_FLAGS) -fPIC $(ERR_FLAGS) $(INC_FLAGS)
LIB_FLAGS:=$(DBG_FLAGS) $(PFL_FLAGS) -fPIC -shared
BIN_FLAGS:=$(DBG_FLAGS) $(PFL_FLAGS) -fPIE $(COP)L .

COMPILE_EXE=$(CC) $(BIN_FLAGS) $1 $(COP)o $2 $3 $(RPATH_FLAG) $(COP)l alu$(PRJ_SFX)
COMPILE_DLL=$(CC) $(LIB_FLAGS) $1 $(COP)o $2 $3 $(RPATH_FLAG)
COMPILE_OBJ=$(CC) $(SRC_FLAGS) $1 $(COP)o $2 -c $3

$(info PRJ_SRC_FILES = '$(PRJ_SRC_FILES)')

$(call mkdir,$(PRJ_EXT_DIR))
$(call github_clone,$(UNIC_DIR),$(PRJ_EXT_DIR),awsdert/unic)

$(info Checking 3rd Party libraries are upto date)
$(info $(call merge,$(UNIC_DIR)))
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
	cd ../ && $(MAKE) build
	cd ../ && $(MAKE) debug
	cd ../ && $(MAKE) profile

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

$(PRJ_BIN_DIR)/%$(PRJ_SFX).AppImage: libalu$(PRJ_SFX).so $(PRJ_BIN_OBJ_FILES:%=%$(PRJ_SFX).o)
	$(call COMPILE_EXE,,$@,$(PRJ_BIN_OBJ_FILES:%=%$(PRJ_SFX).o))

$(PRJ_BIN_DIR)/%$(PRJ_SFX).16.exe: alu$(PRJ_SFX).16.dll $(PRJ_BIN_OBJ_FILES:%=%$(PRJ_SFX).16.obj)
	$(call COMPILE_EXE,,$@,$(PRJ_BIN_OBJ_FILES:%=%$(PRJ_SFX).16.obj))

$(PRJ_BIN_DIR)/%$(PRJ_SFX).32.exe: alu$(PRJ_SFX).32.dll $(PRJ_BIN_OBJ_FILES:%=%$(PRJ_SFX).32.obj)
	$(call COMPILE_EXE,,$@,$(PRJ_BIN_OBJ_FILES:%=%$(PRJ_SFX).32.obj))

$(PRJ_BIN_DIR)/%$(PRJ_SFX).64.exe: alu$(PRJ_SFX).64.dll $(PRJ_BIN_OBJ_FILES:%=%$(PRJ_SFX).64.obj)
	$(call COMPILE_EXE,,$@,$(PRJ_BIN_OBJ_FILES:%=%$(PRJ_SFX).64.obj))
	
$(PRJ_LIB_DIR)/lib%$(PRJ_SFX).so: $(PRJ_LIB_OBJ_FILES:%=%$(PRJ_SFX).o)
	$(call COMPILE_DLL,,$@,$^)

$(PRJ_LIB_DIR)/%$(PRJ_SFX).16.dll: $(PRJ_LIB_OBJ_FILES:%=%$(PRJ_SFX).16.obj)
	$(call COMPILE_DLL,,$@,$^)

$(PRJ_LIB_DIR)/%$(PRJ_SFX).32.dll: $(PRJ_LIB_OBJ_FILES:%=%$(PRJ_SFX).32.obj)
	$(call COMPILE_DLL,,$@,$^)

$(PRJ_LIB_DIR)/%$(PRJ_SFX).64.dll: $(PRJ_LIB_OBJ_FILES:%=%$(PRJ_SFX).64.obj)
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

$(PRJ_MAK_DIR)/%.mak.o:
	@echo Why '$<'?

.PHONY: all $(ALL_GOALS)

.FORCE:
