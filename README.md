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

Run the script `build.sh` to build the Clang plugin:

```bash
bash build.sh
```

## Running the Clang Plugin

To easily run the Clang plugin, run its wrapper script like so:

```bash
./build/bin/maki filename
```

where `filename` is the name of the C file whose macro usage you would like to
analyze.

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
    -DMAKI_ENABLE_TESTING=ON \
    -DLLVM_EXTERNAL_LIT=<lit_path> \
    -DFILECHECK_PATH=<filecheck_path>
cmake --build build/ -t check-maki --parallel
```

Where `<lit_path>` and `<filecheck_path>` are the paths to your `lit` Python
script and `FileCheck` binary, respectively.

## Contributing

Developers must format their code using
[ClangFormat](https://clang.llvm.org/docs/ClangFormat.html) before committing
them for review. Assuming you have already followed the [general setup
instructions](#setting-up) to install LLVM and Clang, you can download
ClangFormat with the following command:

```bash
sudo apt install clang-format-17
```
