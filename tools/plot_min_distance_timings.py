import json
import pathlib
import pickle

import plotly.graph_objects as go

root_dir = pathlib.Path(__file__).parents[1].resolve()
data_dir = root_dir / "data" / "erleben-cube-cliff-edges"


def read_timing_data(collision_type):
    timing_path = data_dir / collision_type / "min_distance_timings.json"
    with open(timing_path) as f:
        data = json.load(f)
        return data["min_distances"], data["timings"]


def main():
    for collision_type in ("vertex-face", "edge-edge",):
        min_distances, timings = read_timing_data(collision_type)
        fig = go.Figure(
            data=go.Scatter(x=min_distances, y=timings, line=dict(color="#8E3B65", width=6)))
        fig.update_layout(
            # title=f"{collision_type}",
            # xaxis_title="Separation Distance",
            # yaxis_title="Average Query Time",
            yaxis={
                "ticksuffix": "μs",
                "ticks": "outside",
                "gridcolor": "rgba(0,0,0,0.4)",
                "linecolor": "black",
                "range": [0, max(timings)]
            },
            xaxis={
                "showexponent": "all",
                "exponentformat": "e",
                "ticks": "outside",
                # "gridcolor": "rgba(0,0,0,0.4)",
                "linecolor": "black"
            },
            xaxis_type="log",
            paper_bgcolor="rgba(0,0,0,0)",
            plot_bgcolor="rgba(0,0,0,0)",
            font=dict(
                color="black",
                family="Times New Roman",
                size=20
            ),
            margin=dict(l=0, r=0, t=0, b=0),
            width=600,
            height=355
        )
        fig.write_image(f"{collision_type}-min-distance-timings.pdf")
        # fig.show()


if __name__ == "__main__":
    main()
