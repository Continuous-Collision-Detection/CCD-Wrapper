#!/bin/bash
# Run the benchmark for a directory

CCD_WRAPPER_ROOT=$(dirname "$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )")

DATA_DIRS="$CCD_WRAPPER_ROOT/data"

COLLISON_TYPES=("vf" "ee")

for DATA_DIR in $DATA_DIRS/chain/; do
    dirs=("$DATA_DIR/vertex-face/" "$DATA_DIR/edge-edge/")
    for type in 0 1; do
        for file in ${dirs[$type]}/*.hdf5; do
            echo $file
	    screen -dmS "pack-$(basename $DATA_DIR)-$(basename $file)" python3 $CCD_WRAPPER_ROOT/tools/pack_collision_queries.py $(realpath $file)
        done
    done
done
