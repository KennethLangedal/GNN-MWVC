#!/bin/bash
#
#
# this script runs the weighhted independent 
# set code from the sebastian lamm paper. in 
# order to run the code we first have to:
#
#   1. transform our generated weighted mtx files 
#       into the appropioate format
#
#   2. run the branch and reduce code in kamis/build/wmis/branch_reduce
# 
#   3. transfrom the results back to vertex cover
#

function run() {
    for path in `ls -Sr $1/*.mtx.graph`; do
        filename=$(basename $path)
        echo "--------------------------------------------------------------------------------"
        echo -e $filename
        echo "++++++++++++++++++++++++++++++++" 
        ls -lah $path | awk -F " " {'print $5'}
        echo "++++++++++++++++++++++++++++++++"
        timeout -k 1000s 1000s /home/daniels/KaMIS/build/wmis/branch_reduce $path --output=/home/daniels/GNN-MWVC/results/${filename}.result
        echo "--------------------------------------------------------------------------------"
    done
}

src_folder=$1
tgt_folder=$2

run $src_folder $tgt_folder


