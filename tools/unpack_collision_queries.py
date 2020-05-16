import sys
import pathlib

import h5py

fname = pathlib.Path(sys.argv[1])

collision_type = "_".join(fname.name.split("-")[:2])

with h5py.File(fname, 'r') as f:
    points = f["points"][()]

with h5py.File(fname, 'w') as f:
    for i in range(points.shape[0] // 8):
        grp = f.create_group(f"{collision_type}-{i:07d}")
        dset = grp.create_dataset("points", (8, 3), dtype='d')
        dset[...] = points[i * 8:(i + 1) * 8]
