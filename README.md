# AIE Codegen

```bash
aie-codegen/
├── fal
│   ├── cmake
│   ├── CMakeLists.txt
│   ├── data
│   ├── doc
│   ├── examples
│   ├── README.md
│   └── src
├── license.txt
├── README.md
└── src
    ├── CMakeLists.txt
    ├── common
    ├── config.in
    ├── core
    ├── device
    ├── dma
    ├── events
    ├── global
    ├── interrupt
    ├── io_backend
    ├── locks
    ├── Makefile
    ├── Makefile.Linux
    ├── Makefile.rsc
    ├── memory
    ├── noc
    ├── npi
    ├── perfcnt
    ├── pl
    ├── pm
    ├── stream_switch
    ├── timer
    ├── trace
    ├── util
    └── aie_codegen.h
```
## aie-codegen/src
AIE Codegen for AIE2/AIE2P/AIE2PS/AIE4 generation devices

## aie-codegen/fal
Functional abstraction layer

## Getting Started

### Clone the Repository

Clone the repository with all submodules:

```bash
git clone <repository-url>
cd aie-codegen
git submodule update --init --recursive
```

This will download the main repository and initialize all required submodules, including the `aie-regdb` directory which contains essential register database files.

## Build Instructions

### Building with Makefile.Linux

The Makefile.Linux provides multiple backend options for different use cases:

1. **Default build** (no specific backend):
```bash
cd src
make -f Makefile.Linux
```

2. **Control Code backend**:
```bash
make -f Makefile.Linux controlcode
```

3. **To build for specific AIE version**:
```bash
make -f Makefile.Linux CFLAGS='-DXAIE_DEV_SINGLE_GEN=XAIE_DEV_GEN_AIE -DXAIE_FEATURE_PRIVILEGED_ENABLED -std=c99 '

make -f Makefile.Linux CFLAGS='-DXAIE_DEV_SINGLE_GEN=XAIE_DEV_GEN_AIEML -DXAIE_FEATURE_PRIVILEGED_ENABLED -std=c99 '

make -f Makefile.Linux CFLAGS='-DXAIE_DEV_SINGLE_GEN=XAIE_DEV_GEN_AIE2IPU -DXAIE_FEATURE_PRIVILEGED_ENABLED -std=c99 '

make -f Makefile.Linux CFLAGS='-DXAIE_DEV_SINGLE_GEN=XAIE_DEV_GEN_AIE2P -DXAIE_FEATURE_PRIVILEGED_ENABLED -std=c99 '

make -f Makefile.Linux CFLAGS='-DXAIE_DEV_SINGLE_GEN=XAIE_DEV_GEN_AIE2P_STRIX_A0 -DXAIE_FEATURE_PRIVILEGED_ENABLED -std=c99 '

make -f Makefile.Linux CFLAGS='-DXAIE_DEV_SINGLE_GEN=XAIE_DEV_GEN_AIE2P_STRIX_B0 -DXAIE_FEATURE_PRIVILEGED_ENABLED -std=c99 '

make -f Makefile.Linux CFLAGS='-DXAIE_DEV_SINGLE_GEN=XAIE_DEV_GEN_AIE2PS -DXAIE_FEATURE_PRIVILEGED_ENABLED -std=c99 '

make -f Makefile.Linux CFLAGS='-DXAIE_DEV_SINGLE_GEN=XAIE_DEV_GEN_AIE4_A -DXAIE_FEATURE_PRIVILEGED_ENABLED -std=c99 '

make -f Makefile.Linux CFLAGS='-DXAIE_DEV_SINGLE_GEN=XAIE_DEV_GEN_AIE4_GENERIC -DXAIE_FEATURE_PRIVILEGED_ENABLED -std=c99 '

make -f Makefile.Linux CFLAGS='-DXAIE_DEV_SINGLE_GEN=XAIE_DEV_GEN_AIE4 -DXAIE_FEATURE_PRIVILEGED_ENABLED -std=c99 '
```

#### Cleaning

```bash
make -f Makefile.Linux clean
```

### Building with CMake

CMake provides a more modern cross-platform build system with installation support:

1. Create a build directory:
```bash
mkdir -p build
cd build
```

2. Configure the project:
```bash
cmake ../src/
```

3. Build the library:
```bash
make
```

The build will produce `libaie_codegen.so` (shared library by default) in the `build/` directory.

#### CMake Build Options

- **Shared vs Static Library**: Set `AIE_CODEGEN_BUILD_SHARED` to `ON` (default) for shared library or `OFF` for static library:
  ```bash
  cmake -DAIE_CODEGEN_BUILD_SHARED=OFF ../src/
  ```

- **Socket Backend**: Enable socket backend support:
  ```bash
  cmake -DSOCKET_BACKEND=ON ../src/
  ```

- **Debug Backend**: Enable debug backend support:
  ```bash
  cmake -DDEBUG_BACKEND=ON ../src/
  ```

### CodeQL Static Analysis

The CMake build includes targets for running [CodeQL](https://codeql.github.com/) security analysis. CodeQL CLI must be installed and available in your `PATH`.

1. Build and generate codeql report
```bash
cd src
mkdir -p build && cd build
cmake .. -G Ninja
ninja codeql
```

This single command will:
- Create a CodeQL database by performing a clean rebuild
- Run the `cpp-security-extended` query suite against the database
- Generate a SARIF report at `build/codeql-results/codeql-results.sarif`
- Generate a human-readable text report at `build/codeql-results/codeql-results.txt`

#### CodeQL CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `CODEQL_DB_DIR` | `${CMAKE_CURRENT_BINARY_DIR}/codeql-db` | Directory for the CodeQL database |
| `CODEQL_RESULTS_DIR` | `${CMAKE_CURRENT_BINARY_DIR}/codeql-results` | Directory for SARIF and text reports |
| `CODEQL_QUERY_SUITE` | `codeql/cpp-queries:codeql-suites/cpp-security-extended.qls` | Query suite to run |

### Build System Comparison

| Feature | CMake | Makefile.Linux |
|---------|-------|----------------|
| Backend Support | Control Code only (default) | 5+ backends |
| Library Type | Shared or Static | Shared only |
| Platform Support | Linux, Windows | Linux only |
| Installation | Yes (system-wide) | No |
| SWIG Support | No | Yes |