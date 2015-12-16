source .config
size `find . -name *.o`> ./doc/budgets/${CONFIG_OS}_budgets.`date '+%y.%m.%d'` 
