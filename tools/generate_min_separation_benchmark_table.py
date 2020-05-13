import json
import pathlib

import pandas

from generate_benchmark_table import (
    method_names, method_abbreviations, datasets, root_dir, data_dir)

method_names = [method_names[5], method_names[7]]
method_abbreviations = [method_abbreviations[5], method_abbreviations[7]]


def load_benchmark_data(collision_type, dataset):
    data = {}
    for dir in data_dir.iterdir():
        if not dir.is_dir() or dir.name not in dataset:
            continue
        benchmark_path = dir / collision_type / "min-separation-benchmark.json"
        if benchmark_path.exists():
            with open(benchmark_path) as f:
                data[dir.name] = json.load(f)
    return data


def create_benchmark_data_frame(data):
    dfs = {}
    data_labels = ["t", "FP", "FN"]
    indices = None
    for scene_name, scene_data in data.items():
        if indices is None:
            indices = [key for key in scene_data if key !=
                       "num_queries" and key != "collision_type"]
        dfs[scene_name] = pandas.DataFrame(
            index=[float(index) for index in indices], columns=(
                ["n"] + data_labels * len(method_names)))
        for index in indices:
            row = [scene_data["num_queries"]]
            for method_name in method_names:
                row.append(scene_data[index][method_name]["avg_query_time"])
                row.append(
                    scene_data[index][method_name]["num_false_positives"])
                row.append(
                    scene_data[index][method_name]["num_false_negatives"])
            dfs[scene_name].loc[float(index)] = row

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
    # {} & {} & MS-FPRF & {} &  & Ours &  \\ \midrule
    print(df.to_latex(
        float_format=(lambda x: f"{x:.2f}"),
        column_format=('l|ccc|ccc')))


def main():
    for dataset in ("handcrafted", ):  # , "simulation":
        for collision_type in "vertex-face", "edge-edge":
            benchmark_data = load_benchmark_data(
                collision_type, datasets[dataset])
            df = create_benchmark_data_frame(benchmark_data)
            print("{} Dataset -- {} CCD/MSCCD".format(
                dataset.title(), collision_type.title()))
            print_latex_table(df)
            print("\\\\[1.5em]")


if __name__ == "__main__":
    main()
