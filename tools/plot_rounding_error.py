import pathlib

import numpy
import h5py
import plotly.graph_objects as go

from generate_benchmark_table import datasets, root_dir, data_dir


def read_shifting_error_data(collision_type, dataset):
    scene_to_rounding_data = {}
    for dir in data_dir.iterdir():
        if not dir.is_dir() or not (dir / collision_type).exists():
            continue
        if dir.name not in dataset:
            continue
        print(dir)
        for fname in (dir / collision_type).iterdir():
            if fname.suffix != ".hdf5" and fname.suffix != ".h5":
                continue
            if dir.name not in scene_to_rounding_data:
                scene_to_rounding_data[dir.name] = []
            with h5py.File(fname, 'r') as f:
                errors = numpy.empty(len(f.keys()))
                for i, key in enumerate(f.keys()):
                    errors[i] = f[f"{key}/shifted/error"][()]
                scene_to_rounding_data[dir.name].append(errors)
    for key in scene_to_rounding_data.keys():
        scene_to_rounding_data[key] = numpy.concatenate(
            scene_to_rounding_data[key])
    return scene_to_rounding_data


def main():
    for dataset in "handcrafted", "simulation":
        for collision_type in "vertex-face", "edge-edge":
            try:
                with numpy.load(data_dir / f"rounding-error-{dataset}-{collision_type}.npz") as npf:
                    all_errors = npf.f.arr_0
            except:
                scene_to_rounding_data = read_shifting_error_data(
                    collision_type, datasets[dataset])
                all_errors = numpy.concatenate(
                    [abs(errors) for name, errors in scene_to_rounding_data.items()])
                numpy.savez(
                    data_dir / f"rounding-error-{dataset}-{collision_type}.npz", all_errors)
                continue

            print(all_errors.min(), all_errors.max())
            if all_errors.shape[0] > 1e7:
                all_errors = numpy.random.choice(
                    all_errors, (all_errors.shape[0] // 10,), False)
                print(all_errors.min(), all_errors.max())
<<<<<<< HEAD
            fig = go.Figure(data=[go.Histogram(
                x=all_errors, histnorm="percent", nbinsx=50,
                marker_color='#8E3B65')])
=======

            fig = go.Figure(data=go.Histogram(
                x=all_errors, histnorm="percent",  # nbinsx=50,
                xbins=dict(
                    start=0,
                    end=1.7763568394002505e-15,
                    size=1.7763568394002505e-15 / 50), autobinx=False,
                marker_color='#8E3B65'))
>>>>>>> Added root_finder and phi timing
            fig.update_layout(
                # title="Error Induced by Rounding Vertices",
                # xaxis_title="Error",
                # yaxis_title="Percentage of Queries",
                yaxis={
                    "ticksuffix": "%",
                    "gridcolor": "rgba(0,0,0,0.4)",
                    "linecolor": "black",
                    "ticks": "outside",
                    "range": [0, 45 if dataset == "handcrafted" else 90]
                },
                xaxis={
                    "showexponent": "all",
                    "exponentformat": "e",
                    "linecolor": "black",
                    "ticks": "outside",
                    "range": [0, 1.7763568394002505e-15]
                },
                bargap=0.1,
                paper_bgcolor='rgba(0,0,0,0)',
                plot_bgcolor='rgba(0,0,0,0)',
                font=dict(
                    color="black",
                    family="Times New Roman",
                    size=20
                ),
                margin=dict(l=0, r=0, t=5, b=0),
                width=600,
                height=355
            )
            fig.write_image(f"{dataset}-{collision_type}-rounding-error.pdf")
            # fig.show()


if __name__ == "__main__":
    main()
