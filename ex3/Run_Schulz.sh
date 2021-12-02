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

# TODO try with both options for reduction parameter 
# TODO run everything on 200 - 2000 mb range
# TODO we agree on a timeout of  1200 sec
#
# --reduction_style=dense
# --time_limit=1000
# ------------------------------------------
#
# ------------------------------------------
function run() {
    for path in $(find $1 -type f -size +$2 -size -$3 | grep '.mtx'); do
        transform $path $4
    done
}

# ------------------------------------------
#
# ------------------------------------------
function transform() {
    local path=$1
    filename=$(basename $path)
    tmpfile_graph=/tmp/$filename.graph
    tmpfile_result=/tmp/$filename.graph.result
    tmpfile_result_trans=/tmp/$filename.graph.result.trans 
    
    /home/daniels/GNN-MWVC/build/mtx_to_graph $path $tmpfile_graph
    result=$(timeout -k 1400s 1400s /home/daniels/KaMIS/build/wmis/branch_reduce $tmpfile_graph --output=$tmpfile_result --time_limit=1000 --reduction_style=dense)  

    echo "==========================================="
    echo -e $filename
    echo "-----------------"
    echo -e $path
    echo -e $tmpfile_graph
    echo -e $tmpfile_result 
    ls -lah $path | awk -F " " {'print $6'}
    echo "-----------------"
    echo $result
    echo "-----------------"
    
    red_time=$(echo $result | grep -oP '(?<=reduction_time )[0-9]+.[0-9]*')
    all_time=$(echo $result | grep -oP '(?<= time )[0-9]+.[0-9]*')
    mis_weight=$(echo $result | grep -oP '(?<= MIS_weight )[0-9]+.[0-9]*')
    mis_weight_check=$(echo $result | grep -oP '(?<= MIS_weight_check )[0-9]+.[0-9]*')
    red_time="${red_time:- -}"
    all_time="${all_time:- -}"
    mis_weight="${mis_weight:- -}"
    mis_weight_check="${mis_weight_check:- -}"

    echo -e "REDUCTION TIME: $red_time"
    echo -e "TIME          : $all_time"
    echo -e "MIS           : $mis_weight"
    echo -e "MIS_CHECK     : $mis_weight_check"
    echo -e $4
    echo "-----------------"

    result_mwvc=$(/home/daniels/GNN-MWVC/build/is_vc_converter $path $tmpfile_result $tmpfile_result_trans)
    result_mwvc_num=$(echo $result_mwvc | grep -oP '(?<= cost: )[0-9]+.[0-9]*') 

    echo -e "$filename,$red_time,$all_time,$mis_weight,$mis_weight_check,$result_mwvc" >> $2
    echo "+++++++++++++++++++++++++++++++++++++++++++"
    echo -e "RESULT $result_mwvc_num"
    echo "+++++++++++++++++++++++++++++++++++++++++++"

    rm $tmpfile_graph
    rm $tmpfile_result  
}

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

# path to source folder
src_folder=/global/D1/projects/mtx/suitesparse_weighted

# min size
min=200k

# max size
max=2000k

# path to result file
tgt_file=/home/daniels/GNN-MWVC/results/result_2.txt

# if [ "$#" -ne 4 ]; then
#     echo -e "${RED}Illegal number of parameters${NC}
#     usage: .Run_Schulz [SRC_FOLDER] [TGT_FOLDER] [MIN_SIZE] [MAX_SIZE]"
#     exit 2
# fi

echo -e "run for files in ${GREEN}$src_folder${NC} between ${RED}$min${NC} - ${RED}$max${NC}"

run $src_folder $min $max $tgt_file 


