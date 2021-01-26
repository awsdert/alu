include mak/func.mak

CC?=vc
COP:=$(call ifin,$(CC),vc,/,-)

MAKECMDGOALS?=info

OPTIONGOALS:=$(filter -%,$(MAKECMDGOALS))
OPTIONGOALS+=-j 1 --no-print-directory
ACTUALGOALS:=$(filter-out -%,$(MAKECMDGOALS))

$(info MAKECMDGOALS=$(MAKECMDGOALS))
$(info OPTIONGOALS=$(OPTIONGOALS))
$(info ACTUALGOALS=$(ACTUALGOALS))

$(ACTUALGOALS):
	cd mak && $(MAKE) $(COP)f main.mak $(OPTIONGOALS) $@
