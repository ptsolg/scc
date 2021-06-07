# SCC

[![AppVeyor Build Status](https://ci.appveyor.com/api/projects/status/github/ptsolg/scc?svg=true)](https://ci.appveyor.com/project/ptsolg/scc)

SCC is a compiler for C99 that targets LLVM-IR.

## Features
* Transactional memory extension
* Can compile itself
* GCC-style command line arguments
* Optimizations!

## Dependencies
* LLVM
* Microsoft SDK
* Visual Studio

## Installation
* Clone this repository
* Run cmake
* Run `setup.ps1` to copy static libraries (make sure you have Visual Studio and Microsoft SDK installed)

## TODOs
* `_Bool`
* Global union initialization
* Standard library
* Support Unix
