import json
import pathlib
import pickle
import itertools

import pandas
from googleapiclient.discovery import build
from google_auth_oauthlib.flow import InstalledAppFlow
from google.auth.transport.requests import Request

method_names = [
    "FloatPointRootFinder",
    "MinSeperationRootFinder",
    "RootParity",
    "RationalRootParity",
    "MinSeparationRootParity",
    "RationalMinSeparationRootParity",
    "BSC",
    "TightCCD",
    "UnivariateIntervalRootFinder",
    "MultivariateIntervalRootFinder",
    "TightIntervals"]
method_abbreviations = [
    "FPRF", "MS-FPRF", "RP", "RRP", "MS-RP", "RMS-RP", "BSC", "TCCD", "UIRF",
    "MIRF", "TI"]

datasets = {
    "handcrafted": [
        "unit-tests", "erleben-spikes", "erleben-spike-wedge", "erleben-wedges",
        "erleben-spike-hole", "erleben-spike-crack", "erleben-wedge-crack",
        "erleben-sliding-spike", "erleben-sliding-wedge",
        "erleben-cube-internal-edges", "erleben-cube-cliff-edges"
    ],
    "simulation": ["mat-twist", "golf-ball", "cow-heads", "chain"]
}

name_to_row = dict(
    zip(itertools.chain.from_iterable(datasets.values()), range(3, 18)))

root_dir = pathlib.Path(__file__).parents[1].resolve()
data_dir = root_dir / "data"


def open_google_sheet():
    SCOPES = ["https://www.googleapis.com/auth/spreadsheets"]

    creds = None
    token_path = root_dir / "token.pickle"
    if token_path.exists():
        with open(token_path, "rb") as token:
            creds = pickle.load(token)
    if not creds or not creds.valid:
        if creds and creds.expired and creds.refresh_token:
            creds.refresh(Request())
        else:
            flow = InstalledAppFlow.from_client_secrets_file(
                root_dir / "credentials.json", SCOPES)
            creds = flow.run_local_server(port=0)
        with open(token_path, "wb") as token:
            pickle.dump(creds, token)

    service = build("sheets", "v4", credentials=creds)

    # Call the Sheets API
    return service.spreadsheets()


def colnum_string(n):
    c = ""
    while n > 0:
        n, remainder = divmod(n - 1, 26)
        c = chr(ord("A") + remainder) + c
    return c


def write_to_google_sheet(df, sheet_name):
    SPREADSHEET_ID = "1CFZOTkemt4_D5MZiBlS2L3MJcTtFMS6jS1qumrNfjR8"
    # SPREADSHEET_ID = "1U1xQsmc4w_AbC_d_mWuTswoOOXO1MQOSyqRXDHuOIcM"
    sheet = open_google_sheet()
    last_col = colnum_string(df.shape[1] + 1)
    for name, row in name_to_row.items():
        if name not in df.index:
            continue
        sheet.values().update(
            spreadsheetId=SPREADSHEET_ID,
            range=f"{sheet_name}!B{row:d}:{last_col}{row:d}",
            valueInputOption="RAW",
            body={
                "majorDimension": "ROWS",
                "values": [df.loc[name].tolist()]
            }).execute()


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

    data_labels = [
        "t", "FP", "FN"]
    df = pandas.DataFrame(index=benchmarks.keys(), columns=(
        ["# of Queries"] + data_labels * (len(method_names) + 1)))
    for name, benchmark in benchmarks.items():
        row = [benchmark["num_queries"]]
        for method_name in method_names:
            if "1e-08" in benchmark[method_name]:
                data = benchmark[method_name]["1e-08"]
            else:
                data = benchmark[method_name]
            row.append(data["avg_query_time"])
            row.append(data["num_false_positives"])
            row.append(data["num_false_negatives"])
            if method_name == "ExactDoubleMinSeparation":
                row.append(benchmark[method_name]["0"]["avg_query_time"])
                row.append(benchmark[method_name]["0"]["num_false_positives"])
                row.append(benchmark[method_name]["0"]["num_false_negatives"])
        df.loc[name] = row
    return df


def print_latex_table(df):
    row_labels = ["t", "FP", "FN"]
    condensed_df = pandas.DataFrame(
        index=row_labels, columns=method_abbreviations + ["Our dagger"])
    num_queries = df["# of Queries"].to_numpy()
    for row_label in row_labels:
        if row_label == "t":
            # Weight the averages by the number of queries and reaverage
            condensed_df.loc[row_label] = (
                df[row_label].to_numpy() * num_queries.reshape(-1, 1)
            ).sum(axis=0) / num_queries.sum()
        else:
            condensed_df.loc[row_label] = df[row_label].sum(
            ).to_numpy().astype(int)

    condensed_df["RRP"].loc["FP"] = condensed_df["RRP"].loc["FN"] = "-"
    print(condensed_df.to_latex(
        float_format=(lambda x: f"{x:.2f}"), column_format=("l|cccccc|ccc")))


def main():
    for dataset in "handcrafted", "simulation":
        for collision_type in "vertex-face", "edge-edge":
            df = read_benchmark_data(collision_type, datasets[dataset])
            print("{} Dataset -- {} CCD/MSCCD ($d=10^{{-8}}$)".format(
                dataset.title(), collision_type.title()))
            print_latex_table(df)
            print("\\\\[1.5em]")

            # write_to_google_sheet(df, collision_type.title(), dataset.title())


if __name__ == "__main__":
    main()
