# Run the benchmark for a directory

CCD_WRAPPER_ROOT=$(dirname "$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )")
if test -f "shift_queries"; then
    SHIFT_QUERIES_BIN=$(realpath shift_queries)
else
    SHIFT_QUERIES_BIN="${CCD_WRAPPER_ROOT}/build/release/shift_queries"
fi
echo "Using $SHIFT_QUERIES_BIN"

DATA_DIRS="$CCD_WRAPPER_ROOT/data"

COLLISON_TYPES=("vf" "ee")

for DATA_DIR in $DATA_DIRS/erleben*/ ; do
    echo $DATA_DIR
    dirs=("$DATA_DIR/vertex-face/" "$DATA_DIR/edge-edge/")
    for type in 0 1; do
        $SHIFT_QUERIES_BIN ${dirs[$type]} ${COLLISON_TYPES[$type]} $i
        echo
    done
done
