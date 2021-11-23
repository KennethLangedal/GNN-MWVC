#!/bin/bash
#
# This script transforms the weighted mtx 
# graphs into the graph format used in the 
# Sebastian Lamm Paper.

# Colors
GREEN='\033[0;32'
RED='\033[0;31m'
NC='\033[0m'

# Prefix
PRE='weighted_'
POST='.graph'

# transforms the graph
function transform_graphs() {
    for path in $1/*.mtx; do
        local filename=$(basename $path)
        local new_filename="${PRE}${filename}${POST}"
        local new_path="$2${new_filename}"
        echo -e "${RED}$path${NC} --> $new_path"
        #  "Usage: ./gen_weights [graph] [output graph] [min] [max] [seed]"
        ../build/mtx_to_graph $path $new_path
    done
}

# path to source folder
src_folder=$1
tgt_folder=$2

echo -e "${RED}$src_folder${NC} --> ${RED}$tgt_folder${NC}"

transform_graphs $src_folder $tgt_folder