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
ALL_GOALS=lint bin_dir build check clean debug gede info lib_dir libalu_32dll
ALL_GOALS+= libalu_64dll libalu_dll libalu_so objects profile rebuild rebuildall
ALL_GOALS+= run test

UNIC_DIR:=$(PRJ_EXT_DIR)/unic
UNIC_INC_DIR:=$(UNIC_DIR)/include

RPATH_FLAG=$(call F_Wl_rpath,$(PRJ_LIB_DIR))

PRJ_CHK_FILES:=$(wildcard $(PRJ_CHK_DIR)/*.c)
PRJ_SRC_FILES:=$(wildcard $(PRJ_SRC_DIR)/*.c)
PRJ_INC_FILES:=$(wildcard $(PRJ_INC_DIR)/*.h)

PRJ_LIB_NAME:=alu$(PRJ_SFX)
$(info PRJ_LIB_NAME=$(PRJ_LIB_NAME))
PRJ_BIN_NAME:=$(call ifin,$(PRJ_GOALS),check,check,test)

PRJ_DST_BIN:=$(PRJ_BIN_NAME)_$(PRJ_LIB_NAME)$(DST_BIN_SFX)
PRJ_DST_LIB:=$(DST_LIB_PFX)$(PRJ_LIB_NAME)$(DST_LIB_SFX)
PRJ_TARGETS:=$(PRJ_DST_OBJ) $(PRJ_DST_LIB) $(PRJ_DST_BIN)

ERR_FLAGS:=$(COP)Wall $(COP)Wextra $(F_pedantic)
INC_FLAGS:=-I $(UNIC_INC_DIR) -I $(PRJ_INC_DIR)
SRC_FLAGS:=$(DBG_FLAGS) $(PFL_FLAGS) -fPIC $(ERR_FLAGS) $(INC_FLAGS) $(COP)D UNIC_FALLBACK
DLL_FLAGS:=$(DBG_FLAGS) $(PFL_FLAGS) -fPIC -shared
EXE_FLAGS:=$(DBG_FLAGS) $(PFL_FLAGS) -fPIE $(COP)L $(PRJ_LIB_DIR)
LIB_FLAGS:=$(COP)l $(PRJ_LIB_NAME) $(call ifin,$(PRJ_GOALS),check,$(CHK_FLAGS))

COMPILE_EXE=$(CC) $(EXE_FLAGS) $1 $(COP)o $(PRJ_BIN_DIR)/$2 $(PRJ_CHK_DIR)/$3 $(RPATH_FLAG) $(LIB_FLAGS)
COMPILE_DLL=$(CC) $(DLL_FLAGS) $1 $(COP)o $(PRJ_LIB_DIR)/$2 $3 $(RPATH_FLAG)
COMPILE_OBJ=$(CC) $(SRC_FLAGS) $1 $(COP)o $2 -c $3

LIBALU_SO_OBJECTS:=$(PRJ_SRC_FILES:%.c=%$(PRJ_SFX).o)
LIBALU_DLL_OBJECTS:=$(PRJ_SRC_FILES:%.c=%$(PRJ_SFX).obj)
LIBALU_32DLL_OBJECTS:=$(PRJ_SRC_FILES:%.c=%$(PRJ_SFX).32.obj)
LIBALU_64DLL_OBJECTS:=$(PRJ_SRC_FILES:%.c=%$(PRJ_SFX).64.obj)

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
	@echo LIBALU_SO_OBJECTS=$(LIBALU_SO_OBJECTS)
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

%.run: %
	$(DBG_APP) $(PFL_APP) $(PRJ_BIN_DIR)/$(PRJ_DST_BIN)
	
test: build

check: build
	
%.gede: %
	gede --args $(PRJ_BIN_DIR)/$(PRJ_DST_BIN)

%.valgrind: %
	valgrind $(PRJ_BIN_DIR)/$(PRJ_DST_BIN)
	
debug: build

profile: build

build: $(PRJ_DST_BIN)

objects: $(PRJ_DST_OBJ)

lint: $(PRJ_SRC_FILES) $(PRJ_CHK_FILES)
	cppcheck --language=c --std=c89 --enable=warning,style --template=gcc $^

bin_dir: $(PRJ_BIN_DIR)/
lib_dir: $(PRJ_LIB_DIR)/
libalu_so: lib$(PRJ_LIB_NAME).so lib_dir
libalu_dll: $(PRJ_LIB_NAME).dll lib_dir
libalu_32dll: $(PRJ_LIB_NAME).32.dll lib_dir
libalu_64dll: $(PRJ_LIB_NAME).64.dll lib_dir

%/:
	mkdir $*

%.AppImage: $(PRJ_CHK_DIR)/%.o bin_dir libalu_so
	$(call COMPILE_EXE,,$@,$*.o)

%.exe: $(PRJ_CHK_DIR)/%.obj bin_dir libalu_dll
	$(call COMPILE_EXE,,$@,$*.obj)

%.32.exe: $(PRJ_CHK_DIR)/%.32.obj bin_dir libalu_32dll
	$(call COMPILE_EXE,,$@,$*.32.obj)

%.64.exe: $(PRJ_CHK_DIR)/%.64.obj bin_dir libalu_64dll
	$(call COMPILE_EXE,,$@,$*.64.obj)
	
%.so: $(LIBALU_SO_OBJECTS) lib_dir
	$(call COMPILE_DLL,,$@,$(LIBALU_SO_OBJECTS))

%.dll: $(LIBALU_DLL_OBJECTS) lib_dir
	$(call COMPILE_DLL,,$@,$(LIBALU_DLL_OBJECTS))

%.32.dll: $(LIBALU_32DLL_OBJECTS) lib_dir
	$(call COMPILE_DLL,,$@,$(LIBALU_32DLL_OBJECTS))

%.64.dll: $(LIBALU_64DLL_OBJECTS) lib_dir
	$(call COMPILE_DLL,,$@,$(LIBALU_64DLL_OBJECTS))

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
