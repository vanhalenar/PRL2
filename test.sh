# VUT FIT - PRL projekt 2
# Timotej Halenár - xhalen00
# 29.4.2024


#!/bin/bash

# kontrola na pocet argumentu
if [ $# -ne 1 ]; then
  exit 1;
fi;


proc=$((${#1}*2-2))
# proc=1

# pocet procesu == 0, nema cenu pokracovat
if [ $proc -eq 0 ]; then
  echo "${1}: 0"
  exit 0;
fi;

# preklad
mpic++ --prefix /usr/local/share/OpenMPI -o vuv vuv.cpp

# spusteni aplikace (oversubscribe - vice procesu nez fyzicky k dispozici)
mpirun --oversubscribe --prefix /usr/local/share/OpenMPI -np $proc vuv "$1"

# uklid
rm -f vuv
