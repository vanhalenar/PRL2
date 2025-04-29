# VUT FIT - PRL projekt 2
# Timotej Halenár - xhalen00
# 29.4.2024


#!/bin/bash

# kontrola na pocet argumentu
if [ $# -ne 1 ]; then
  exit 1;
fi;

# pocet procesu == 0, nema cenu pokracovat
proc=$((${#1}*2-2))

if [ $proc -eq 0 ]; then
  exit 2;
fi;

# preklad
mpic++ --prefix /usr/local/share/OpenMPI -o vuv vuv.cpp

# spusteni aplikace (oversubscribe - vice procesu nez fyzicky k dispozici)
mpirun --oversubscribe --prefix /usr/local/share/OpenMPI -np $proc vuv "$1"

# uklid
rm -f vuv
