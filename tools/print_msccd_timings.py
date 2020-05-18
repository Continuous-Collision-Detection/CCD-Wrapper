import json
import pathlib
import itertools

from generate_benchmark_table import method_names, datasets, root_dir, data_dir


def read_benchmark_data(collision_type, dataset):
    benchmarks = {}
    for dir in data_dir.iterdir():
        if not dir.is_dir() or dir.name not in dataset:
            continue
        benchmark_path = dir / collision_type / "benchmark.json"
        if not benchmark_path.exists():
            continue
        with open(dir / collision_type / "benchmark.json") as f:
            benchmarks[dir.name] = json.load(f)

    timings = {}
    for name, benchmark in benchmarks.items():
        double_msccd = benchmark[method_names[7]]
        timings[name] = dict(
            num_queries=benchmark["num_queries"],
            root_finder_percent=double_msccd["root_finder_percent"],
            phi_percent=double_msccd["phi_percent"]
        )
    return timings


def print_average_timing(timings):
    num_queries = 0
    total_root_finder_percent = 0
    total_phi_percent = 0
    for timing in timings.values():
        num_queries += timing["num_queries"]
        total_root_finder_percent += (timing["num_queries"]
                                      * timing["root_finder_percent"])
        total_phi_percent += timing["num_queries"] * timing["phi_percent"]
    print("Avg. root finder time percentage: {:g}%".format(
        total_root_finder_percent / num_queries))
    print("Avg. ϕ time percentage: {:g}%".format(
        total_phi_percent / num_queries))


def main():
    for dataset in "handcrafted", "simulation":
        for collision_type in "vertex-face", "edge-edge":
            timings = read_benchmark_data(collision_type, datasets[dataset])
            print_average_timing(timings)


if __name__ == "__main__":
    main()
