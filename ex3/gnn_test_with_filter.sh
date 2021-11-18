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

# path to result folder
tgt_folder=$2

# min size
min=$3

# max size
max=$4

# filename
filename=$tgt_folder/$5

echo -e "run for files between ${RED}$min${NC} and ${RED}$max${NC}"

touch $filename

for file in $(find $1 -type f -size +$3 -size -$4 | grep '.mtx'); do
    size=$(numfmt --to=iec-i --suffix=B --format="%.3f" $(stat -c%s "$file"))
    echo -e "$size \t $file"
    ../build/GNN_VC_experimental ../model/pure_model.txt $file $filename
done