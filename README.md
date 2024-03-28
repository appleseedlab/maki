# Artifact Documentation

## Table of contents

- [Artifact Documentation](#artifact-documentation)
  - [Table of contents](#table-of-contents)
  - [Purpose](#purpose)
  - [Provenance](#provenance)
  - [Data](#data)
  - [Setup](#setup)
    - [Requirements](#requirements)
    - [Instructions](#instructions)
  - [Usage](#usage)
    - [Basic usage](#basic-usage)
    - [Testing](#testing)
    - [Replicating major paper results (kicking the tires)](#replicating-major-paper-results-kicking-the-tires)
    - [Resetting](#resetting)
    - [Replicating major paper results (all results)](#replicating-major-paper-results-all-results)
    - [Uninstalling](#uninstalling)

## Purpose

<!--
Purpose: a brief description of what the artifact does.
- Include a list of badge(s) the authors are applying for as well as the reasons
  why the authors believe that the artifact deserves that badge(s).
-->

This artifact provides the source code for Maki, a tool for analyzing macro
usage portability as described in this artifact's associated paper. This
artifact also contains a dataset with the paper's major results, and
instructions on how to build and run Maki to replicate these results.

We are applying for the **available** and **reusable** badges. We believe this
artifact deserves the available badge because it is publicly available on Zenodo
at https://doi.org/10.5281/zenodo.7783131 (DOI 10.5281/zenodo.7783131). We
believe this artifact deserves the reusable badge because it includes
instructions for reproducing all the paper's major results, along with a dataset
one may verify them against. This artifact also utilizes Docker to facilitate
reuse, as recommended in the ICSE 2024 Call for Artifact Submissions.

## Provenance

<!--
Provenance: where the artifact can be obtained, preferably with a link to the
paper’s preprint if publicly available.
-->

The artifact as reported in the original paper is available on Zenodo
(https://doi.org/10.5281/zenodo.7783131). A pre-print of the original paper
referencing this artifact can be found here:
https://pappasbrent.com/assets/Semantic_Analysis_of_Macro_Usage_for_Portability_-_Preprint.pdf.

## Data

<!--
Data (for artifacts which focus on data or include a nontrivial dataset): cover
aspects related to understanding the context, data provenance, ethical and legal
statements (as long as relevant), and storage requirements.
-->

The `datasets` directory contains the original data that all major results of
the paper are based on, and is approximately 2 megabytes in size. An explanation
of its directory structure and contents follows:

```
datasets/
├── figure_data - Contains all raw data for paper figures
│   ├── all_raw_data.csv - Contains all programs' macro definition properties.
│   ├── figure_2_chart_data.csv - Contains data for Figure 2 in the paper.
│   ├── figure_3_chart_data.csv - Contains data for Figure 3 in the paper.
│   └── figure_4_chart_data.csv - Contains data for Figure 4 in the paper.
├── linux_patch_submissions - Contains submitted Linux kernel patches.
│   ├── accepted/ - Contains submitted Linux patches that maintainers accepted.
│   └── rejected/ - Contains submitted Linux patches that maintainers rejected.
├── porting_enscript_and_m4 - Contains notes taken while hand-porting all macros
│   │                         in enscript and m4 to C.
│   ├── enscript_maki_output.csv - Contains Maki's analysis results for
│   │                              enscript, formatted as a CSV file.
│   ├── enscript_transformation_notes.md - Contains notes taken while hand-
│   │                                      porting macros in enscript to C.
│   ├── enscript_transformations.diff - Diff of changes before and after hand-
│   │                                   porting macros in enscript to C.
│   ├── m4_maki_output.csv - Contains notes taken while hand-porting macros in
│   │                        m4 to C.
│   ├── m4_transformation_notes.md - Contains notes taken while hand-porting
│   │                                macros in m4 to C.
│   └── m4_transformations.diff - Diff of changes before and after hand-porting
│                                 macros in m4 to C.
├── porting_linux_ipc_and_sound_atmel - Contains notes taken while hand-porting
│   │                                   all macros in the ipc and sound/atmel
│   │                                   Linux modules.
│   ├── linux_ipc_sound_atmel_transformation_notes.md - Contains notes taken
│   │                                                   while hand-porting all
│   │                                                   macros in the ipc and
│   │                                                   sound/atmel Linux
│   │                                                   modules.
│   └── linux_ipc_sound_atmel_transformations.diff - Diff of changes before and
│                                                    after hand-porting macros
│                                                    in the ipc and sound/atmel
│                                                    Linux modules.
└── random_sample_notes
│   ├── all.csv - Contains notes taken while manually verifying the properties
│   │             of all macros in the random sample.
│   ├── only_false_negatives.csv - Contains notes only on macros in the random
│   │                              sample for which Maki reported a false
│   │                              negative.
│   └── only_false_positives.csv - Contains notes only on macros in the random
│                                  sample for which Maki reported a false
│                                  positive.
├── time_data - Contains data on Maki's performance.
│   ├── definition_analysis_times.csv - Contains performance data for analyzing
│   │                                   macro definitions across all programs.
│   └── invocation_analysis_times.csv - Contains performance data for analyzing
│                                       macro invocations across all programs.
```

## Setup

<!--
Setup (for executable artifacts): provide clear instructions for how to prepare
the artifact for execution. This includes:
- Hardware: performance, storage, and device-type (e.g. GPUs) requirements.
- Software: Docker or VM requirements, or operating system & package
  dependencies if not provided as a container or VM. Providing a Dockerfile or
  image, or at least confirming the tool’s installation in a container is
  strongly encouraged. Any deviation from standard environments needs to be
  reasonably justified.
-->

### Requirements

- Hardware:
  - At least eight CPU cores and 8GB of RAM are recommended.
  - 1.94GB of free space if one only wishes to "kick the tires" and verify that
    Maki works by replicating only the analysis of the program `bc`. Replicating
    Maki's entire evaluation requires 620GB of storage space.
- Software: [Docker](https://docs.docker.com/get-docker/); tested with Docker
  24.0.6.

### Instructions

First, either:

- Build the Docker image:

  ```console
  docker build -t maki:1.0 .
  ```

- Or load the Docker image from the provided `tar` file:

  ```console
  docker load --input maki.tar
  ```

After building/loading the image, run the docker image as a container:

```console
docker run --name maki-container -it maki:1.0
```

Build Maki's associated Clang plugin:

```console
bash build_clang_plugin.sh
```

To restart and attach to the same container in the future without creating a new
one, run the following commands:

```console
docker start maki-container
docker attach maki-container
```

## Usage

<!--
Usage (for executable artifacts): provide clear instructions for how to
repeat/replicate/reproduce the main results presented in the paper. Include
both:
- A basic usage example or a method to test the installation. For instance, it
  may describe what command to run and what output to expect to confirm that the
  code is installed and operational.
- Detailed commands to replicate the major results from the paper.
-->

### Basic usage

First follow the instructions listed in the _[Setup](#setup)_ section, being
sure to build Maki's docker image, run it as a container, and build Maki's Clang
plugin frontend. Then one may run Maki's wrapper script, e.g.:

```
bash build/bin/cpp2c tests/addressed_arguments.c
```

Running the above command will tell Maki to analyze all macro invocations in the
source file `tests/addressed_arguments.c` and print the results to standard
output. The last line of this output should be a large JSON object containing
the properties of the last macro invocation in the file:

```
Invocation      {     "Name" : "ADDR_OF",     "DefinitionLocation" : "/maki/tests/addressed_arguments.c:3:9",     "InvocationLocation" : "/maki/tests/addressed_arguments.c:9:5",     "ASTKind" : "Expr",     "TypeSignature" : "int *(int)",     "InvocationDepth" : 0,     "NumASTRoots" : 1,     "NumArguments" : 1,     "HasStringification" : false,     "HasTokenPasting" : false,     "HasAlignedArguments" : true,     "HasSameNameAsOtherDeclaration" : false,     "IsExpansionControlFlowStmt" : false,     "DoesBodyReferenceMacroDefinedAfterMacro" : false,     "DoesBodyReferenceDeclDeclaredAfterMacro" : false,     "DoesBodyContainDeclRefExpr" : false,     "DoesSubexpressionExpandedFromBodyHaveLocalType" : false,     "DoesSubexpressionExpandedFromBodyHaveTypeDefinedAfterMacro" : false,     "DoesAnyArgumentHaveSideEffects" : false,     "DoesAnyArgumentContainDeclRefExpr" : true,     "IsHygienic" : true,     "IsDefinitionLocationValid" : true,     "IsInvocationLocationValid" : true,     "IsObjectLike" : false,     "IsInvokedInMacroArgument" : false,     "IsNamePresentInCPPConditional" : false,     "IsExpansionICE" : false,     "IsExpansionTypeNull" : false,     "IsExpansionTypeAnonymous" : false,     "IsExpansionTypeLocalType" : false,     "IsExpansionTypeDefinedAfterMacro" : false,     "IsExpansionTypeVoid" : false,     "IsAnyArgumentTypeNull" : false,     "IsAnyArgumentTypeAnonymous" : false,     "IsAnyArgumentTypeLocalType" : false,     "IsAnyArgumentTypeDefinedAfterMacro" : false,     "IsAnyArgumentTypeVoid" : false,     "IsInvokedWhereModifiableValueRequired" : false,     "IsInvokedWhereAddressableValueRequired" : false,     "IsInvokedWhereICERequired" : false,     "IsAnyArgumentExpandedWhereModifiableValueRequired" : false,     "IsAnyArgumentExpandedWhereAddressableValueRequired" : true,     "IsAnyArgumentConditionallyEvaluated" : false,     "IsAnyArgumentNeverExpanded" : false,     "IsAnyArgumentNotAnExpression" : false  }
```

### Copying evaluation results out of the Docker container

Run the following command on your host system to copy files out of the Docker
container to your host system:

```console
docker cp maki-container:/path/to/source-file /path/to/destination-file
```

For example, to copy the file `README.md` out of the container to the current
directory in the host system, run the following command:

```console
docker cp maki-container:/maki/README.md .
```

### Testing

The test suite for Maki's Clang plugin is located in the `tests` directory. At
the bottom of each test file, there are comments listing the macro invocation
properties that Maki is expected to predict for that file. The test suite is not
automated, so to run the test suite one must run Maki on each file manually
using the command shown in the section _[Basic Usage](#basic-usage)_.

### Replicating major paper results (kicking the tires)

Replicating all the results presented in the paper would require more than 17
days; therefore we provide reviewers with the option to replicate a small subset
of the results, which should only require about five minutes.

If one only wishes to "kick the tires" and verify that Maki's evaluation
framework works correctly, then they may run the following script:

```console
bash replicate_results.sh
```

By default, this will run Maki's evaluation on the program `bc` only. This
script will attempt to download a compressed copy of `bc`'s source code to
`evaluation/archived_programs`, extract it to `evaluation/extracted_programs`,
and run Maki's Clang frontend and Python analysis on it with eight processes.
Intermediate and final results will be placed in subdirectories and files within
the `evaluation` directory. The names of these subdirectories and files, along
with how to interpret them, are as follows:

- `evaluation/macro_invocation_analyses/`: Contains Maki's output for analyzing
  macro invocations in all programs. For each program, Maki will create a
  parallel directory structure, and for each C source file analyzed, Maki will
  create a file with the same name but with the extension `.cpp2c` containing
  the results of Maki's macro invocation analysis for that source file.

- `evaluation/macro_definition_analyses/`: Contains Maki's output for analyzing
  macro definitions in all programs. Maki will output its results for each
  program to a JSON with the same name as the analyzed program. This file
  contains a single JSON object whose keys are the machine-readable names of
  characteristics studied in our evaluation. Each key is mapped to a JSON object
  with the three fields: `total`, the total number of macro definitions that
  display the characteristic associated with that key, `olms`, the number of
  object-like macros that display the characteristic, and `flms`, the number of
  function-like macros that display the characteristic.

- `evaluation/figure_data/`: Contains CSV-formatted macro definition analysis
  data for all programs that was used to generate the figures in the original
  paper.

  - `evaluation/figure_data/all_raw_data.csv`: Contains all macro definition
    analyses for all programs in CSV form.
  - `evaluation/figure_data/figure_2_data.csv`: Contains the data used to
    generate Figure 2 in the original paper.
  - `evaluation/figure_data/figure_3_data.csv`: Contains the data used to
    generate Figure 3 in the original paper.
  - `evaluation/figure_data/figure_4_data.csv`: Contains the data used to
    generate Figure 4 in the original paper.

- `evaluation/time_data/`: Contains CSV files reporting Maki's performance data.
  - `evaluation/time_data/definition_analysis_times.csv`: Lists the time elapsed
    for Maki to analyze all macro definitions in each program.
  - `evaluation/time_data/invocation_analysis_times.csv`: Lists time elapsed for
    Maki to analyze all macro invocations in each program.
  - `evaluation/time_data/total_time_analysis.csv`: Presents the five-point
    summary of and total elapsed time to analyze all macro invocations and
    analyses in all programs.

### Resetting

If Maki's evaluation is cancelled or encounters an error before running to
completion, please remove all intermediate analysis files before attempting to
restart the evaluation. In particular, if one encounters the following error:

```
IndexError: index -1 is out of bounds for axis 0 with size 0
```

Then one must reset the `evaluation` directory to a clean state before trying to
replicate the evaluation again. We provide a script for doing this:

```console
bash reset_evaluation.sh
```

This script removes the following `evaluation` subdirectories:

- `archived_programs`
- `evaluation_programs`
- `extracted_programs`
- `figure_data`
- `macro_definition_analyses`
- `macro_invocation_analyses`
- `time_data`

### Replicating major paper results (all results)

The `evaluation` directory must be in a clean state before trying to replicate
all results reported in the original paper. One can run the script
`reset_evaluation.sh` as described in section [above](#resetting) to do this.
One may then perform Maki's full evaluation by passing the flag `--all` to the
script `replicate_results.sh` like so:

```console
bash replicate_results.sh --all
```

The script will download all 21 evaluation programs listed in
`evaluation/evaluation_programs_all.py` to `evaluation/archived_programs`, and
extract and build them in `evaluation/extracted_programs`. After building all
programs, Maki's Clang frontend will run on eight processes to analyze all macro
invocations in each of them. Maki will place its analyses for a program's macro
invocations in `evaluation/macro_invocation_analyses` as soon as it finishes
running. Next, Maki's python library will use the results from the previous step
to analyze all macro definitions for each program, and will place these results
in JSON files in `evaluation/macro_invocation_analyses`. Finally, Maki will
convert the data in these JSON files to CSV format, and place the results in
`evaluation/figure_data`. This directory contains data used to generate the
figures in the original paper. CSV files containing the time elapsed to analyze
each program's macro invocations and definitions will be placed in
`evaluation/time_data` as well.

As a reminder, it took us over 19 days to run Maki on all the programs in our
benchmark. We ran Maki with eight processes on all programs except Linux, for
which we used 32 processes. Our machine had 2 64-core CPUs and 512GB of RAM. If
one attempts to run Maki's full evaluation using fewer processes, or on a
machine with less available memory, then it will likely take longer to complete.

### Uninstalling

To completely uninstall Maki, first delete all its associated Docker containers:

```console
docker container rm maki-container
```

Then, delete Maki's Docker image:

```console
docker image rm maki:1.0
```

You may also want to clear your Docker installation's cache to remove any
lingering data related to Maki on your system:

```console
docker system prune -f
```

**Note**: This will tell Docker to delete all cached data that it is not
currently using, so be careful not to delete any cached data you may want to
keep!
