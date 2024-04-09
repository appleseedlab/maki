# Maki

An automated macro invocation analysis tool.

If you would like to see the artifact associated with the ICSE 2024 paper,
_Semantic Analysis of Macro Usage for Portability_, please go to this
[Zenodo link](https://zenodo.org/doi/10.5281/zenodo.7783131).

## Requirements

```bash
wget -qO- https://apt.llvm.org/llvm-snapshot.gpg.key | sudo tee /etc/apt/trusted.gpg.d/apt.llvm.org.asc
sudo apt install llvm-17 clang-17 libclang-17-dev build-essential cmake
```

## setting up

Run the script `build.sh` to build the Clang plugin:

```bash
bash build.sh
```

## Running the Clang Plugin

To easily run the Clang plugin, run its wrapper script like so:

```bash
./build/bin/cpp2c filename
```

where `filename` is the name of the C file whose macro usage you would like to
analyze.
