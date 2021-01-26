include func.mak

GOALS2EXEC:=$(filter %.run,$(MAKECMDGOALS:.exec=.run))
GOALS2DBUG:=$(filter %.debug,$(MAKECMDGOALS))
GOALS4GEDE:=$(filter %.gede,$(MAKECMDGOALS))
GOALS4SPEED:=$(filter %.fast,$(MAKECMDGOALS))
GOALS4VALGRIND:=$(filter %.valgrind,$(MAKECMDGOALS))
OTHERGOALS:=$(filter-out %.run %.gede %.valgrind %.fast,$(MAKCMDGOALS))

PRJ_GOALS:=$(GOALS2EXEC:.run=) $(GOALS2DBUG:.debug=)
PRJ_GOALS+=$(GOALS4GEDE:.gede=) $(GOALS4SPEED:.fast=)
PRJ_GOALS+=$(GOALS4VALGRIND:.valgrind=) $(OTHERGOALS)

$(info PRJ_GOALS=$(PRJ_GOALS))
