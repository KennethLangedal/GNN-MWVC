#!/bin/bash
#
# This script runs the gnn_test over a subset of suitsparse.
#
# tmux new -s newSessionName    === create a new session
# tmux a -t newSessionName      === connect tosession 
# ctr + b , d                   === disconnect from session
# srun -p defq --exclusive --time 150:00:00 --pty bash scriptName —login
#

# Colors
GREEN='\033[0;32'
RED='\033[0;31m'
NC='\033[0m'

# path to source folder
src_folder=$1

# min size
min=$2

# max size
max=$3

echo -e "run for files between ${RED}$min${NC} and ${RED}$max${NC}"

skip=0

for file in $(find $1 -type f -size +$2 -size -$3 | grep '.mtx'); do
    if [ $skip -lt 59 ]
    then
        ((skip++))
        continue
    fi
    size=$(numfmt --to=iec-i --suffix=B --format="%.3f" $(stat -c%s "$file"))
    echo -e "$size \t $file"
    ../DynWVC2/Converter < $file > $(basename $file ".mtx").mwvc
    ../DynWVC2/DynWVC2 $(basename $file ".mtx").mwvc 0 1000 0 >> res_dyn2.txt
    rm $(basename $file ".mtx").mwvc
done