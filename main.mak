include func.mak
include dst_sys.mak
include dst_cc.mak

PRJ:=ALU
ALL_GOALS:=info objects build run

PRJ_SRC_DIR:=
PRJ_INC_DIR:=
PRJ_LIB_DIR:=
PRJ_BIN_DIR:=
PRJ_EXT_DIR:=cloned

FBSTDC_DIR:=$(PRJ_EXT_DIR)/fbstdc
FBSTDC_INC_DIR:=$(FBSTDC_DIR)/include

RPATH_FLAG=-Wl,-rpath=./

SRC_FLAGS:=-fPIC -shared -Wall -Wextra -I $(FBSTDC_INC_DIR)
LIB_FLAGS:=-fPIC -shared
BIN_FLAGS:=-fPIE -L . -l alu

PRJ_SRC_FILES:=test.c $(wildcard $(PRJ_SRC_DIR)alu*.c)
PRJ_INC_FILES:=$(wildcard $(PRJ_INC_DIR)*.h)
PRJ_OBJ_FILES:=$(PRJ_SRC_FILES:%.c=%$(DST_OBJ_SFX))

PRJ_BIN_OBJ_FILES:=test$(DST_OBJ_SFX)
PRJ_LIB_OBJ_FILES:=$(filter-out $(PRJ_BIN_OBJ_FILES),$(PRJ_OBJ_FILES))

PRJ_DST_BIN:=alu$(DST_BIN_SFX)
PRJ_DST_LIB:=$(DST_LIB_PFX)alu$(DST_LIB_SFX)

COMPILE_EXE=$(CC) $(BIN_FLAGS) $1 $(COP)o $2 $3 $(RPATH_FLAG)
COMPILE_DLL=$(CC) $(LIB_FLAGS) $1 $(COP)o $2 $3 $(RPATH_FLAG)
COMPILE_OBJ=$(CC) $(SRC_FLAGS) $1 $(COP)o $2 -c $3

$(info PRJ_SRC_FILES = '$(PRJ_SRC_FILES)')

$(call mkdir,$(PRJ_EXT_DIR))
$(call github_clone,$(FBSTDC_DIR),$(PRJ_EXT_DIR),awsdert/fbstdc)

$(info Checking 3rd Party libraries are upto date)
$(info $(call rebase,$(FBSTDC_DIR)))
$(info $(shell $(call rebase,$(FBSTDC_DIR))))
$(info Finished checking)

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

all: $(ALL_GOALS)
	@echo all

rebuild: clean build

clean:
	rm -f *.AppImage *.exe *.so *.dll *.o *.obj

run: build
	./$(PRJ_DST_BIN)

build: objects $(PRJ_DST_LIB) $(PRJ_DST_BIN)

objects: $(PRJ_OBJ_FILES)

%.AppImage: lib%.so test.o
	$(call COMPILE_EXE,,$@,test.o)

%.16.exe: %.16.dll test.16.obj
	$(call COMPILE_EXE,,$@,test.16.obj)

%.32.exe: %.32.dll test.32.obj
	$(call COMPILE_EXE,,$@,test.32.obj)

%.64.exe: %.64.dll test.64.obj
	$(call COMPILE_EXE,,$@,test.64.obj)
	
lib%.so: $(filter-out test.o,$(PRJ_SRC_FILES:%.c=%.o))
	$(call COMPILE_DLL,,$@,$^)

%.16.dll: $(filter-out test.16.obj,$(PRJ_SRC_FILES:%.c=%.16.obj))
	$(call COMPILE_DLL,,$@,$^)

%.32.dll: $(filter-out test.32.obj,$(PRJ_SRC_FILES:%.c=%.32.obj))
	$(call COMPILE_DLL,,$@,$^)

%.64.dll: $(filter-out test.64.obj,$(PRJ_SRC_FILES:%.c=%.64.obj))
	$(call COMPILE_DLL,,$@,$^)

%.o: %.c
	$(call COMPILE_OBJ,,$@,$<)

%.16.obj: %.c
	$(call COMPILE_OBJ,,$@,$<)

%.32.obj: %.c
	$(call COMPILE_OBJ,,$@,$<)

%.64.obj: %.c
	$(call COMPILE_OBJ,,$@,$<)
	
%.c: $(PRJ_INC_FILES)

%.mak.o:
	@echo Why '$<'?

.PHONY: info all clean objects build rebuild run

.FORCE:
