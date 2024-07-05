# Maki

An automated macro invocation analysis tool.

If you would like to see the artifact associated with the ICSE 2024 paper,
_Semantic Analysis of Macro Usage for Portability_, please go to this
[Zenodo link](https://zenodo.org/doi/10.5281/zenodo.7783131).

## Requirements

The following instructions assume an Ubuntu 22.04.4 operating system:

- [CMake](https://cmake.org/). Follow the instructions on the CMake
  [downloads page](https://cmake.org/download/).

- [LLVM and Clang](https://apt.llvm.org/). Run the following commands:

  ```bash
  wget -qO- https://apt.llvm.org/llvm-snapshot.gpg.key | sudo tee /etc/apt/trusted.gpg.d/apt.llvm.org.asc
  sudo apt install llvm-17 clang-17 libclang-17-dev build-essential cmake
  ```

## Setting up

Build the Clang plugin, e.g.,

```bash
cmake -S . -B build/ -G Ninja
cmake --build build/
```

## Running the Clang Plugin

To easily run the Clang plugin, run its wrapper script like so:

```bash
./build/bin/maki filename
```

where `filename` is the name of the C file whose macro usage you would like to
analyze.

### Flags

Maki offers the following flags to modify its behavior:

| Flag                  | Effect                                                                                                                  | Example invocation                           |
|-----------------------|-------------------------------------------------------------------------------------------------------------------------|----------------------------------------------|
| `--builtin-macros`    | Maki will analyze macro definitions or invocations of compiler builtin macros (this is the default)                     | `maki -fplugin-arg-maki---builtin-macros`    |
| `--no-builtin-macros` | Maki will not analyze macro definitions or invocations of compiler builtin macros                                       | `maki -fplugin-arg-maki---no-builtin-macros` |
| `--system-macros`     | Maki will analyze macro definitions or invocations in system headers (this is the default)                              | `maki -fplugin-arg-maki---system-macros`     |
| `--no-system-macros`  | Maki will not analyze macro definitions or invocations in system headers                                                | `maki -fplugin-arg-maki---no-system-macros`  |
| `--invalid-macros`    | Maki will analyze macro definitions or invocations at invalid locations, e.g. on the command line (this is the default) | `maki -fplugin-arg-maki---invalid-macros`    |
| `--no-invalid-macros` | Maki will not analyze macro definitions or invocations at invalid locations                                             | `maki -fplugin-arg-maki---no-invalid-macros` |

## Testing

Maki's test suite is located in the `tests/Tests` directory and automated with
[LLVM LIT](https://llvm.org/docs/CommandGuide/lit.html) and
[`FileCheck`](https://llvm.org/docs/CommandGuide/FileCheck.html).

### Testing dependencies

- The Python `lit` script from [PyPi](https://pypi.org/project/lit/):

  ```bash
  python3 -m pip install lit
  ```

- `FileCheck` as one of LLVM's dev dependencies:

  ```bash
  sudo apt install llvm-dev
  ```

- `jq` from your package manager, e.g.,

  ```bash
  sudo apt install jq
  ```

### Running the test suite

From the project root, run the following command to configure Maki with testing
enabled and to run its test suite:

```bash
cmake -S . -B build/ \
    -DMAKI_ENABLE_TESTING=ON 
cmake --build build/ -t check-maki --parallel
```

Note: If lit and FileCheck are not in your PATH, you can specify their
locations manually with the following command:

```bash
cmake -S . -B build/ \
    -DMAKI_ENABLE_TESTING=ON \
    -DLIT_PATH=<lit_path> \
    -DFILECHECK_PATH=<filecheck_path> \
```

Where `<lit_path>` and `<filecheck_path>` are the paths to your `lit` Python
script and `FileCheck` binary, respectively.

### Using the provided Docker image to test

Build the Docker image with the following command in the project root:

```bash
docker build -t maki:1.0 .
```

Run the Docker image:

```bash
docker run -it maki:1.0
```

Build the plugin:

```bash
cmake -S . -B build/ -G Ninja
cmake --build build/
```

Run the test suite:

```bash
cmake -S . -B build/ \
    -DMAKI_ENABLE_TESTING=ON
cmake --build build/ -t check-maki --parallel
```

Note: If you are on an AMD64 architecture, please follow these steps:

1. Add the following line to the beginning of the Dockerfile:
```#!/bin/bash```

2. Change the now second line of the docker file from:
```FROM ubuntu:22.04```
to:
```FROM --platform=linux/amd64 ubuntu:22.04```

3. Build the Docker image with the following command:
```docker build --platform linux/amd64  -t maki:1.0 .```

4. Run the Docker image:
```docker run --platform linux/amd64 --name maki-container -it maki:1.0```

## Contributing

Developers must format their code using
[ClangFormat](https://clang.llvm.org/docs/ClangFormat.html) before committing
them for review. Assuming you have already followed the [general setup
instructions](#setting-up) to install LLVM and Clang, you can download
ClangFormat with the following command:

```bash
sudo apt install clang-format-17
```
