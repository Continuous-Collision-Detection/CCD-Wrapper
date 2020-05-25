import sys
import pathlib

import numpy
import h5py


def pack_queries(fname: pathlib.Path):
    with h5py.File(fname, 'r') as f:
        points = numpy.vstack([f[f"{key}/points"][()] for key in f.keys()])
        results = numpy.vstack([f[f"{key}/result"][()] for key in f.keys()])
        rounded_points = numpy.vstack(
            [f[f"{key}/shifted/points"][()] for key in f.keys()])
        rounding_error = numpy.vstack(
            [f[f"{key}/shifted/error"][()] for key in f.keys()])
        rounded_results = numpy.vstack(
            [f[f"{key}/shifted/result"][()] for key in f.keys()])

    fname = fname.parent / (fname.stem + '-packed.hdf5')
    settings = dict(compression="gzip", compression_opts=9)
    with h5py.File(fname, 'w') as f:
        f.create_dataset(
            "points", points.shape, dtype=points.dtype, **settings)[...] = points
        f.create_dataset(
            "result", results.shape, dtype=results.dtype, **settings)[...] = results
        rounded_grp = f.create_group(f"rounded")
        rounded_grp.create_dataset(
            "points", rounded_points.shape, dtype=rounded_points.dtype, **settings
        )[...] = rounded_points
        rounded_grp.create_dataset(
            "result", rounded_results.shape, dtype=rounded_results.dtype, **settings
        )[...] = rounded_results
        rounded_grp.create_dataset(
            "error", rounding_error.shape, dtype=rounding_error.dtype, **settings
        )[...] = rounding_error


def main():
    if sys.argv[1] == "all":
        root_dir = pathlib.Path(__file__).parents[1].resolve()
        data_dir = root_dir / "data"
        for fname in data_dir.glob("**/*.hdf5"):
            if "packed" in str(fname.name):
                continue
            print(fname)
            pack_queries(fname)
    else:
        pack_queries(pathlib.Path(sys.argv[1]))


if __name__ == "__main__":
    main()
