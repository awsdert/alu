MAKECMDGOALS?=info

$(info MAKECMDGOALS=$(MAKECMDGOALS))

$(MAKECMDGOALS):
	$(MAKE) -j 1 --no-print-directory $(COP)f main.mak $(MAKECMDGOALS)
