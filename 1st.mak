MAKECMDGOALS?=info

$(info MAKECMDGOALS=$(MAKECMDGOALS))

$(MAKECMDGOALS):
	$(MAKE) --no-print-directory $(COP)f main.mak $(MAKECMDGOALS)
