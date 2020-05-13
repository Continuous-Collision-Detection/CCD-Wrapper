import json
import pathlib
import pickle

import pandas
from googleapiclient.discovery import build
from google_auth_oauthlib.flow import InstalledAppFlow
from google.auth.transport.requests import Request

all_method_names = [
    "Float", "RootParity", "RationalRootParity", "BSC", "TightCCD",
    "FloatMinSeparation", "ExactRationalMinSeparation",
    "ExactDoubleMinSeparation"]
all_method_abbrev = [
    "FPRF", "RP", "RRP", "BSC", "TCCD", "MS-FPRF", "Ours R", "Ours"]
method_names_to_abbrev = dict(zip(all_method_names, all_method_abbrev))

name_to_row = dict(zip(["unit-tests",
                        "erleben-spikes",
                        "erleben-spike-wedge",
                        "erleben-wedges",
                        "erleben-spike-hole",
                        "erleben-spike-crack",
                        "erleben-wedge-crack",
                        "erleben-sliding-spike",
                        "erleben-sliding-wedge",
                        "erleben-cube-internal-edges",
                        "erleben-cube-cliff-edges",
                        "mat-twist", 
                        "golf-ball",
                        "cow-heads",
                        "chain"
                        ], range(3, 14)))

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
        sheet.values().update(
            spreadsheetId=SPREADSHEET_ID,
            range=f"{sheet_name}!B{row:d}:{last_col}{row:d}",
            valueInputOption="RAW",
            body={
                "majorDimension": "ROWS",
                "values": [df.loc[name].tolist()]
            }).execute()


def print_latex_table(df):
    row_labels = ["t", "FP", "FN"]
    condensed_df = pandas.DataFrame(
        index=row_labels, columns=all_method_abbrev)
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
    print(condensed_df.to_latex(
        float_format=(lambda x: f"{x:.2f}"), column_format=('l|' + 'c' * (condensed_df.shape[1] - 1) + '|c')))


def read_benchmark_data(collision_type, method_names):
    benchmarks = {}
    for dir in data_dir.iterdir():
        if not dir.is_dir() or dir.name == "mat-twist":
            continue
        benchmark_path = dir / collision_type / "benchmark.json"
        if not benchmark_path.exists():
            continue
        with open(dir / collision_type / "benchmark.json") as f:
            benchmarks[dir.name] = json.load(f)

    data_labels = [
        "t", "FP", "FN"]
    df = pandas.DataFrame(index=benchmarks.keys(), columns=(
        ["# of Queries"] + data_labels * len(method_names)))
    for name, benchmark in benchmarks.items():
        row = [benchmark["num_queries"]]
        for method_name in method_names:
            row.append(benchmark[method_name]["avg_query_time"])
            row.append(benchmark[method_name]["num_false_positives"])
            row.append(benchmark[method_name]["num_false_negatives"])
        df.loc[name] = row
    return df


def main():
    vf_df = read_benchmark_data("vertex-face", all_method_names)
    ee_df = read_benchmark_data("edge-edge", all_method_names)

    print_latex_table(vf_df)
    print("\\\\")
    print_latex_table(ee_df)

    # write_to_google_sheet(vf_df, "Vertex-Face")
    # write_to_google_sheet(ee_df, "Edge-Edge")

    # TODO: Read and write minimum distance benchmark data
    # vf_df = read_benchmark_data(
    #     "vertex-face", [all_method_names[-1]] + all_method_names[-3:-1])
    # ee_df = read_benchmark_data(
    #     "edge-edge", [all_method_names[-1]] + all_method_names[-3:-1])
    #
    # write_to_google_sheet(vf_df, "Min Distance Vertex-Face")
    # write_to_google_sheet(ee_df, "Min Distance Edge-Edge")


if __name__ == "__main__":
    main()
