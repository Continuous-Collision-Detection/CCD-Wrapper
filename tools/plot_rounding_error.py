import pathlib

import numpy
import h5py
import plotly.graph_objects as go

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
    for collision_type in ("vertex-face", "edge-edge",):
        scene_to_rounding_data = read_shifting_error_data(collision_type)
        all_errors = numpy.concatenate(
            [abs(errors) for name, errors in scene_to_rounding_data.items()])
        fig = go.Figure(data=[go.Histogram(
            x=all_errors, histnorm="percent", nbinsx=50, marker_color='#8E3B65')])
        fig.update_layout(
            # title="Error Induced by Rounding Vertices",
            # xaxis_title="Error",
            # yaxis_title="Percentage of Queries",
            yaxis={
                "ticksuffix": "%",
                "gridcolor": "rgba(0,0,0,0.4)",
                "linecolor": "black",
                "ticks": "outside",
            },
            xaxis={
                "showexponent": "all",
                "exponentformat": "e",
                "linecolor": "black",
                "ticks": "outside"
            },
            bargap=0.1,
            paper_bgcolor='rgba(0,0,0,0)',
            plot_bgcolor='rgba(0,0,0,0)',
            font=dict(
                color="black",
                family="Times New Roman",
                size=20
            ),
            margin=dict(l=0, r=0, t=0, b=0),
            width=600,
            height=355
        )
        fig.write_image(f"{collision_type}-rounding-error.pdf")


if __name__ == "__main__":
    main()
