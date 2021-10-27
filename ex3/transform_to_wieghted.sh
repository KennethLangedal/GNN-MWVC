#!/bin/bash
#
# This script turns any graph from the Suitsparse Collection into a weighted version.
#

# Colors
GREEN='\033[0;32'
RED='\033[0;31m'
NC='\033[0m'

# Prefix
PRE='weighted_'

# transforms the graph
function transform_graph() {
    for path in $1/*.mtx; do
        local filename=$(basename $path)
        local new_filename="${PRE}${filename}"
        local new_path="$2${new_filename}"
        echo -e "${RED}$path${NC} --> $new_path"
        ../build/gen_weights < $path > $new_path
    done
}

# path to source folder
src_folder=$1
tgt_folder=$2

echo -e "${RED}$src_folder${NC} --> ${RED}$tgt_folder${NC}"

transform_graph $src_folder $tgt_folder


