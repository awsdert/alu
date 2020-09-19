include mak/func.mak

CC?=vc
COP:=$(call ifin,$(CC),vc,/,-)

MAKECMDGOALS?=info

$(MAKECMDGOALS):
	#MAKECMDGOALS=$(MAKECMDGOALS)
	cd mak && $(MAKE) -j 1 --no-print-directory $(COP)f main.mak $(MAKECMDGOALS)
