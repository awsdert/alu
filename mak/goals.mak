include func.mak

GOALS2EXEC:=$(filter %.run,$(MAKECMDGOALS:.exec=.run))
GOALS4GEDE:=$(filter %.gede,$(MAKECMDGOALS))
GOALS4VALGRIND:=$(filter %.valgrind,$(MAKECMDGOALS))
OTHERGOALS:=$(filter-out %.run %.gede %.valgrind,$(MAKCMDGOALS))

PRJ_GOALS:=$(GOALS2EXEC:.run=) $(GOALS4GEDE:.gede=)
PRJ_GOALS+=$(GOALS4VALGRIND:.valgrind=) $(OTHERGOALS)

$(info PRJ_GOALS=$(PRJ_GOALS))
