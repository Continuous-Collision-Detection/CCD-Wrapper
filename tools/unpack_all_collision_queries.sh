CCD_WRAPPER_ROOT=$(dirname "$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )")

DATA_DIRS="$CCD_WRAPPER_ROOT/data"

for DATA_DIR in $DATA_DIRS/chain/; do
    echo $DATA_DIR
    for dir in "$DATA_DIR/vertex-face/" "$DATA_DIR/edge-edge/"; do
        for file in $dir/*.hdf5; do
            python3 "$CCD_WRAPPER_ROOT/tools/unpack_collision_queries.py" "$file" &
        done
    done
done

