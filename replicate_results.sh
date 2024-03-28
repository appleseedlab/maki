#!/usr/bin/bash

# Run Maki's evaluation
cd evaluation

if [ "$1" = "--all" ]; then
    # If the argument --full is passed, then copy the full evaluation program
    # list to the file evaluation_programs.py to perform the full evaluation
    cp evaluation_programs_all.py evaluation_programs.py
else
    # Otherwise, copy the evaluation program list containing only bc to the file
    # evaluation_programs.py to perform the demo evaluation
    cp evaluation_programs_only_bc.py evaluation_programs.py
fi

# Set up evaluation programs
python3 setup_programs.py all

# Analyze macro invocations in all programs and record performance data
mkdir -p time_data
python3 analyze_macro_invocations_in_programs.py ../build/lib/libcpp2c.so ./time_data/invocation_analysis_times.csv 8

# Analayze macro definitions in all programs and record performance data
python3 analyze_macro_definitions_in_programs.py ./time_data/definition_analysis_times.csv

# Generate CSV files behind figures
mkdir -p figure_data
python3 tabulate_analyses.py
mv all_raw_data.csv figure_data
python3 get_figure_2_data.py figure_data/all_raw_data.csv > ./figure_data/figure_2_data.csv
python3 get_figure_3_data.py figure_data/all_raw_data.csv > ./figure_data/figure_3_data.csv
python3 get_figure_4_data.py figure_data/all_raw_data.csv > ./figure_data/figure_4_data.csv

# Analyze performance data CSV files
python3 analyze_times.py ./time_data/invocation_analysis_times.csv ./time_data/definition_analysis_times.csv > ./time_data/total_time_analysis.csv
