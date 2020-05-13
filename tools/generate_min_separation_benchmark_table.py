import json
import pathlib

import pandas

method_names = ["FloatMinSeparation", "ExactDoubleMinSeparation"]
method_abbreviations = ["MS-FPRF", "Ours"]
method_names_to_abbrev = dict(zip(method_names, method_abbreviations))

benchmarks = {
    "handcrafted": [
        "unit-tests", "erleben-spikes", "erleben-spike-wedge", "erleben-wedges",
        "erleben-spike-hole", "erleben-spike-crack", "erleben-wedge-crack",
        "erleben-sliding-spike", "erleben-sliding-wedge",
        "erleben-cube-internal-edges", "erleben-cube-cliff-edges"
    ],
    "simulation": ["golf-ball", "twisting-mat", "cow-heads", "chain"]
}

root_dir = pathlib.Path(__file__).parents[1].resolve()
data_dir = root_dir / "data"


def load_benchmark_data(collision_type, benchmark):
    benchmark_data = {}
    for dir in data_dir.iterdir():
        if not dir.is_dir() or dir.name not in benchmark:
            continue
        benchmark_path = dir / collision_type / "min-separation-benchmark.json"
        if benchmark_path.exists():
            with open(benchmark_path) as f:
                benchmark_data[dir.name] = json.load(f)
    return benchmark_data


def create_benchmark_data_frame(data):
    dfs = {}
    data_labels = [
        "Avg. Query Time", "# of False Positives", "# of False Negatives"]
    indices = None
    for scene_name, scene_data in data.items():
        if indices is None:
            indices = [key for key in scene_data if key !=
                       "num_queries" and key != "collision_type"]
        dfs[scene_name] = pandas.DataFrame(
            index=[float(index) for index in indices], columns=(
                ["# of Queries"] + data_labels * len(method_names)))
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
        df["Avg. Query Time"] = (
            df["Avg. Query Time"].to_numpy() * df["# of Queries"].to_numpy().reshape(-1, 1))
        if(cumulative_df is None):
            cumulative_df = df.copy()
        else:
            cumulative_df += df
    cumulative_df["Avg. Query Time"] = (
        cumulative_df["Avg. Query Time"].to_numpy()
        / cumulative_df["# of Queries"].to_numpy().reshape(-1, 1))

    cumulative_df = cumulative_df.drop("# of Queries", axis=1)
    cumulative_df = cumulative_df.sort_index(ascending=False)
    return cumulative_df


def print_latex_table(df):
    print(df.to_latex(
        float_format=(lambda x: f"{x:.2f}"),
        column_format=('l' + 'c' * (df.shape[1]))))


def main():
    for collision_type in "vertex-face", "edge-edge":
        benchmark_data = load_benchmark_data(
            collision_type, benchmarks["handcrafted"])
        df = create_benchmark_data_frame(benchmark_data)

        print_latex_table(df)
        print("\\\\")


if __name__ == "__main__":
    main()
