# argvnaut
A lightweight command line parser for modern C++

[![Build and Tests](https://github.com/giuliom/argvnaut/actions/workflows/CI.yml/badge.svg)](https://github.com/giuliom/argvnaut/actions/workflows/CI.yml)

## Features

- Header-only, C++20
- Positional arguments (required or optional), flags, and typed options (string, integer, float, boolean)
- Values are validated against their declared type at parse time
- `--name value`, `--name=value`, and short `-n value` syntax
- `--` end-of-options separator and negative numbers as values
- Generated `usage()` / `help()` text and parse error messages

## Quick start

```cpp
#include <argvnaut.h>
#include <iostream>

int main(int argc, char** argv)
{
    ArgvNaut::Parser parser("myprog");
    parser.addPositional("input", 1, true, "Input file");
    parser.addFlag("verbose", "v", "Enable verbose output");
    parser.addOption("count", "n", ArgvNaut::OptionType::INTEGER, "1", false, "Number of iterations");

    if (!parser.parse(argc, argv)) {
        std::cerr << "Error: " << parser.error() << "\n" << parser.usage() << "\n";
        return 1;
    }

    std::string input = parser.getString("input").value();
    bool verbose = parser.getFlag("verbose");
    int count = parser.getInt("count").value();

    std::cout << parser.help(); // generated help text
    return 0;
}
```

## Building

The library is header-only: add the `include/` directory to your include paths, or link the
`ArgvnautLib` CMake interface target. Tests are only built when argvnaut is the top-level
project (override with `-DARGVNAUT_BUILD_TESTS=ON/OFF`):

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build
```
