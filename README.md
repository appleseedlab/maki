# Macro Analysis Tool

## Preface about the Data
All raw data is already available in the directories `evaluation/results` and `datasets`.
Program runtime data is present in the files `evaluation/macro_analyzer_raw_times.txt` and `evaluation/property_analyzer_raw_times.txt`.
This artifact enables you to reproduce this data on your own machine.

## Requirements

### Clang Plugin
```
sudo apt install llvm-14 clang-14 libclang-14-dev build-essential cmake
```

### Python Script
- Python 3.10.x
  ```
  sudo apt install software-properties-common -y
  sudo add-apt-repository ppa:deadsnakes/ppa
  sudo apt install python3.10
  ```
- numpy
  ```
  python3 -m pip install -U numpy
  ```

## Setting Up
Run the script `build.sh` to build the Clang plugin:
```
bash build.sh
```

## Running the Clang Plugin
To easily run the Clang plugin, run its wrapper script like so:
```
./build/bin/cpp2c filename
```
where `filename` is the name of the C file whose macro usage you would like to analyze.

## Running the Python Scripts and Evaluation
1. Navigate to the `evaluation` directory.
2. Run the script `setup_programs.py`.
   This will download, extract, configure, and build all programs listed in `evaluation_programs.py`.
   Note that each program has its own set of dependencies; we have attempted to list them all in the comments of `evaluation_programs.py`, but you may have to do some trial-and-error to determine each program's requirements.
   If you simply want to test the evaluation to see if it works, we recommend commenting out all programs in listed in `evaluation_programs.py` except for `bc`, since `bc` is one of the quickest to analyze.
3. To get the macro invocation analysis data for all programs, run:
   ```
   python3 analyze_macros_in_programs.py cpp2c_so_path num_threads
   ```
   where
   - `cpp2c_so_path` is the path to the built so file for the macro analyzer, and should be in `build/lib/libcpp2c.so`.
   - `num_threads` is the number of threads to use.
   
   Pipe the output of this command to a file to get the time required to analyze all macro invocations in each program.
4. To get the macro definition analysis data for all programs, run:
   ```
   python3 analyze_transformations_in_programs.py
   ```
   
   Pipe the output of this command to a file to get the time required to analyze all macro definitions in each program.
5. To tabulate the macro definition analysis data for all programs, run
   ```
   python3 tabulate_analyses.py
   ```
6. Now to get the data presented in the figures in the evaluation section, run the script associated with each figure:
    ```
   python3 get_figure_2_data.py all_raw_data.csv
   python3 get_figure_3_data.py all_raw_data.csv
   python3 get_figure_4_data.py all_raw_data.csv
   ```

   Note: Step 5 should have created the file `all_raw_data.csv`.
7. To get the data on program runtime, run:
   ```
   python3 analyze_times.py invocation_analysis_times_csv definition_analysis_times_csv
   ```
   where
   - `invocation_analysis_times_csv` is a CSV file containing the time required to analyze each program's macro invocations (see `datasets/macro_analyzer_raw_times`) for an example.
   - `definition_analysis_times_csv` is a CSV file containing the time required to analyze each program's macro definitions (see `datasets/property_analyzer_raw_times`) for an example.
  
   Unfortunately, we do not provide a helper script for creating these CSV files, and you will have to create them manually using the piped output from steps 3 and 4.
