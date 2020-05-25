#!/bin/bash
# Run the benchmark for a directory

CCD_WRAPPER_ROOT=$(dirname "$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )")
if test -f "generate_ground_truth"; then
    GROUND_TRUTH_BIN=$(realpath generate_ground_truth)
else
    GROUND_TRUTH_BIN="${CCD_WRAPPER_ROOT}/build/release/generate_ground_truth"
fi
echo "Using $GROUND_TRUTH_BIN"

DATA_DIRS="$CCD_WRAPPER_ROOT/data"

COLLISON_TYPES=("vf" "ee")

for DATA_DIR in $DATA_DIRS/*/ ; do
    if [ $(basename $DATA_DIR) == "unit-tests" ]; then
        continue
    fi
    dirs=("$DATA_DIR/vertex-face/" "$DATA_DIR/edge-edge/")
    for type in 0 1; do
        for file in ${dirs[$type]}/*.hdf5; do
            screen -dmS "$(basename $DATA_DIR)-${COLLISON_TYPES[$type]}" $GROUND_TRUTH_BIN ${file} ${COLLISON_TYPES[$type]}
        done
    done
done
