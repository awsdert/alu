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
PRJ_BIN_OBJ_FILES:=$(PRJ_CHK_DIR)/test_alu
PRJ_APP_OBJ_FILES:=$(PRJ_CHK_OBJ_FILES) $(PRJ_BIN_OBJ_FILES)
PRJ_LIB_OBJ_FILES:=$(filter-out $(PRJ_APP_OBJ_FILES),$(PRJ_OBJ_FILES))

PRJ_LIB_NAME:=alu$(PRJ_SFX)
$(info PRJ_LIB_NAME=$(PRJ_LIB_NAME))

PRJ_DST_CHK:=check_$(PRJ_LIB_NAME)$(DST_BIN_SFX)
PRJ_DST_BIN:=test_$(PRJ_LIB_NAME)$(DST_BIN_SFX)
PRJ_DST_LIB:=$(DST_LIB_PFX)$(PRJ_LIB_NAME)$(DST_LIB_SFX)
PRJ_DST_OBJ:=$(PRJ_OBJ_FILES:%=%$(PRJ_SFX)$(DST_OBJ_SFX))
PRJ_TARGETS:=$(PRJ_DST_OBJ) $(PRJ_DST_LIB) $(PRJ_DST_BIN)

ERR_FLAGS:=$(COP)Wall $(COP)Wextra $(F_pedantic)
INC_FLAGS:=-I $(UNIC_INC_DIR) -I $(PRJ_INC_DIR)
SRC_FLAGS:=$(DBG_FLAGS) $(PFL_FLAGS) -fPIC $(ERR_FLAGS) $(INC_FLAGS)
LIB_FLAGS:=$(DBG_FLAGS) $(PFL_FLAGS) -fPIC -shared
BIN_FLAGS:=$(DBG_FLAGS) $(PFL_FLAGS) -fPIE $(COP)L $(PRJ_LIB_DIR)

COMPILE_EXE=$(CC) $(BIN_FLAGS) $1 $(COP)o $(PRJ_BIN_DIR)/$2 $(PRJ_CHK_DIR)/$3 $(RPATH_FLAG) $(COP)l $(PRJ_LIB_NAME)
COMPILE_DLL=$(CC) $(LIB_FLAGS) $1 $(COP)o $(PRJ_LIB_DIR)/$2 $3 $(RPATH_FLAG)
COMPILE_OBJ=$(CC) $(SRC_FLAGS) $1 $(COP)o $2 -c $3

$(info PRJ_SRC_FILES = '$(PRJ_SRC_FILES)')
$(info PRJ_BIN_OBJ_FILES = '$(PRJ_BIN_OBJ_FILES)')
$(info PRJ_LIB_OBJ_FILES = '$(PRJ_LIB_OBJ_FILES)')

$(call mkdir,$(PRJ_EXT_DIR))
$(call github_clone,$(UNIC_DIR),$(PRJ_EXT_DIR),awsdert/unic)

$(info Checking 3rd Party libraries are upto date)
$(info $(call merge,$(UNIC_DIR)))
$(info Finished checking)

$(info PRJ_DST_BIN=$(PRJ_DST_BIN))
$(info PRJ_DST_LIB=$(PRJ_DST_LIB))

VPATH:=$(PRJ_BIN_DIR):$(PRJ_LIB_DIR):$(VPATH)

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
	rm -f $(PRJ_BIN_DIR)/*.AppImage $(PRJ_BIN_DIR)/*.exe
	rm -f $(PRJ_LIB_DIR)/*.so $(PRJ_LIB_DIR)/*.dll
	rm -f $(PRJ_SRC_DIR)/*.o $(PRJ_SRC_DIR)/*.obj
	rm -f $(PRJ_CHK_DIR)/*.o $(PRJ_CHK_DIR)/*.obj

run: build
	cd $(PRJ_BIN_DIR) && $(DBG_APP) $(PFL_APP) $(PRJ_DST_BIN)
	
debug: build

profile: build

gede: debug
	gede --args $(PRJ_DST_BIN)

build: $(PRJ_DST_BIN)

objects: $(PRJ_DST_OBJ)

%.AppImage: lib$(PRJ_LIB_NAME).so $(PRJ_CHK_DIR)/%.o
	$(call COMPILE_EXE,,$@,$*.o)

%.exe: $(PRJ_LIB_NAME).dll $(PRJ_CHK_DIR)/%.obj
	$(call COMPILE_EXE,,$@,$*.obj)

%.32.exe: $(PRJ_LIB_NAME).32.dll $(PRJ_CHK_DIR)/%.32.obj
	$(call COMPILE_EXE,,$@,$*.32.obj)

%.64.exe: $(PRJ_LIB_NAME).64.dll $(PRJ_CHK_DIR)/%.64.obj
	$(call COMPILE_EXE,,$@,$*.64.obj)
	
lib%.so: $(PRJ_LIB_OBJ_FILES:%=%$(PRJ_SFX).o)
	$(call COMPILE_DLL,,$@,$^)

%.dll: $(PRJ_LIB_OBJ_FILES:%=%$(PRJ_SFX).obj)
	$(call COMPILE_DLL,,$@,$^)

%.32.dll: $(PRJ_LIB_OBJ_FILES:%=%$(PRJ_SFX).32.obj)
	$(call COMPILE_DLL,,$@,$^)

%.64.dll: $(PRJ_LIB_OBJ_FILES:%=%$(PRJ_SFX).64.obj)
	$(call COMPILE_DLL,,$@,$^)

%$(PRJ_SFX).o: %.c
	$(call COMPILE_OBJ,,$@,$<)

%$(PRJ_SFX).obj: %.c
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
