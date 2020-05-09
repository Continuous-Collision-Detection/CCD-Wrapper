import pathlib

import numpy
import h5py
import plotly

root_dir = pathlib.Path(__file__).parents[1].resolve()
data_dir = root_dir / "data"


def read_shifting_error_data(collision_type):
    scene_to_rounding_data = {}
    for dir in data_dir.iterdir():
        if not dir.is_dir() or not (dir / collision_type).exists():
            continue
        if "erleben" not in dir.name:
            continue
        for fname in (dir / collision_type).iterdir():
            if fname.suffix != ".hdf5" and fname.suffix != ".h5":
                continue
            if dir.name not in scene_to_rounding_data:
                scene_to_rounding_data[dir.name] = []
            with h5py.File(fname, 'r') as f:
                for key in f.keys():
                    try:
                        scene_to_rounding_data[dir.name].append(
                            f[f"{key}/shifted/error"][()])
                    except:
                        breakpoint()
    for key in scene_to_rounding_data.keys():
        scene_to_rounding_data[key] = numpy.array(scene_to_rounding_data[key])
    return scene_to_rounding_data


def main():
    scene_to_rounding_data = read_shifting_error_data("vertex-face")
    for name, errors in scene_to_rounding_data.items():
        print("{}: ||err||∞={:g} average_err={:g}".format(
            name, numpy.linalg.norm(errors, numpy.inf), numpy.average(errors)))


if __name__ == "__main__":
    main()
