include func.mak

GOALS2EXEC:=$(filter %.run,$(MAKECMDGOALS:.exec=.run))
GOALS4GEDE:=$(filter %.gede,$(MAKECMDGOALS))
OTHERGOALS:=$(filter-out %.run %.gede,$(MAKCMDGOALS))

PRJ_GOALS:=$(GOALS2EXEC:.run=) $(GOALS4GEDE:.gede=) $(OTHERGOALS)

$(info PRJ_GOALS=$(PRJ_GOALS))
