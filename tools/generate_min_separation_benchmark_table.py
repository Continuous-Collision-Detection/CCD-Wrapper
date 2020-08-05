import json
import pathlib

import pandas

from generate_benchmark_table import (
    method_names, method_abbreviations, datasets, data_dir)

method_names = [method_names[1], method_names[4]]
method_abbreviations = [method_abbreviations[1], method_abbreviations[4]]

min_distances = ["0.01", "1e-08", "1e-16", "1e-30", "1e-100"]


def load_benchmark_data(collision_type, dataset):
    data = {}
    for dir in data_dir.iterdir():
        if not dir.is_dir() or dir.name not in dataset:
            continue
        benchmark_path = dir / collision_type / "benchmark.json"
        if benchmark_path.exists():
            with open(benchmark_path) as f:
                data[dir.name] = json.load(f)
    return data


def create_benchmark_data_frame(data):
    dfs = {}
    data_labels = ["t", "FP", "FN"]
    for scene_name, scene_data in data.items():
        dfs[scene_name] = pandas.DataFrame(
            index=[float(d) for d in min_distances], columns=(
                ["n"] + data_labels * len(method_names)))
        for d in min_distances:
            row = [scene_data["num_queries"]]
            for method_name in method_names:
                row.append(scene_data[method_name][d]["avg_query_time"])
                row.append(scene_data[method_name][d]["num_false_positives"])
                row.append(scene_data[method_name][d]["num_false_negatives"])
            dfs[scene_name].loc[float(d)] = row

    cumulative_df = None
    for scene_name, df in dfs.items():
        df["t"] = (df["t"].to_numpy() * df["n"].to_numpy().reshape(-1, 1))
        if(cumulative_df is None):
            cumulative_df = df.copy()
        else:
            cumulative_df += df
    cumulative_df["t"] = (cumulative_df["t"].to_numpy()
                          / cumulative_df["n"].to_numpy().reshape(-1, 1))

    cumulative_df = cumulative_df.drop("n", axis=1)
    cumulative_df = cumulative_df.sort_index(ascending=False)
    cumulative_df.index = cumulative_df.index.map(lambda x: f"{x:.0e}")
    return cumulative_df


def print_latex_table(df):
    # {} & {} & MS-FPRF & {} &  & MS-RP &  \\ \midrule
    print(df.to_latex(
        float_format=(lambda x: f"{x:.2f}"),
        column_format=('l|ccc|ccc|ccc|ccc|ccc|ccc|ccc|ccc')))


def main():
    dfs = []
    for dataset in "handcrafted", "simulation":
        for collision_type in "vertex-face", "edge-edge":
            benchmark_data = load_benchmark_data(
                collision_type, datasets[dataset])
            dfs.append(create_benchmark_data_frame(benchmark_data))
    df = pandas.concat(dfs, axis=1, sort=False)
    print_latex_table(df)


if __name__ == "__main__":
    main()
