ifin=$(if $(filter $1,$2),$3,$4)
ifout=$(if $(filter-out $1,$2),$3,$4)
# Ensure the directory exists
mkdir=$(info $(if $(wildcard $1),,$(shell mkdir $1)))
# github_clone( target_dir, common_clone_dir, github_dir )
github_clone=$(info $(if $(wildcard $1),,$(shell cd $2 && git clone https://github.com/$3)))
# $(shell merge( target_dir ))
merge=cd '$1' && git config pull.rebase false && git pull
# $(shell rebase( target_dir ))
rebase=cd '$1' && git config pull.rebase true && git pull
# $(shell jump( target_dir ))
jump=cd '$1' && git config pull.ff only && git pull
# $(shell pull( target_dir ))
pull=cd '$1' && git pull
# MSWIN_VARS( suffix )
MSWIN_VARS=$(WIN16_$1) $(WIN32_$1) $(WIN64_$1)
