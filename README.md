# aie-codegen

**aie-codegen** is a C library (with optional C++ headers) that implements low-level programming and configuration for AMD **AI Engine** tiles across multiple device generations. It is the software layer that turns register-level operations—DMA, locks, interrupts, stream switches, trace, power/reset, and related subsystems—into callable APIs used by runtimes, drivers, and validation stacks.

If you are new here, read this file top to bottom once; it explains what the repository contains, how it fits into the wider AI Engine stack, and how to build and install the supported way (**CMake only**).

---

## Who uses this repository?

- **Platform and driver teams** linking `libaie_codegen` into Linux or Windows components that must configure AI Engine hardware.
- **Runtime and tooling** that emits or consumes **control code** (binary sequences that program the array) and needs a stable, versioned register abstraction.
- **Validation and CI** (including downstream harnesses such as **AigCtrlCodeValidationSuite**) that replay or stress control-code paths.

You do not need to be on a specific product team to understand the layout: the C sources under `src/` are the engine; the C++ layer under `fal/` is an optional higher-level API built on top of that engine; `aie-regdb/` supplies register metadata consumed by the build.

---

## Repository layout (what each part is for)

### `src/` — core C library (`aie_codegen`)

This is the main deliverable: a **C11** library named `aie_codegen` (CMake target `aie_codegen::aie_codegen`). It is organized by subsystem, for example:

| Area | Role |
|------|------|
| `global/`, `device/` | Device instance setup, generation-specific globals, and chip integration glue. |
| `core/`, `dma/`, `locks/`, `memory/` | Tile programming: cores, DMA descriptors, locks, data memory. |
| `stream_switch/`, `noc/`, `routing/` | Data movement and interconnect-oriented configuration. |
| `interrupt/`, `events/`, `trace/`, `timer/`, `perfcnt/` | Observability, interrupts, performance, and trace. |
| `pm/`, `npi/`, `pl/` | Power/reset/tile control, NPI access, PL-facing interfaces. |
| `io_backend/` | **How** register writes are performed: control-code generation, socket transport, simulation hooks, debug paths, and related infrastructure. CMake options select which backends are compiled in (see [CMake options](#cmake-options)). |
| `common/` | Shared helpers (transactions, secure I/O helpers, instrumentation buffers, etc.). |

The umbrella header is `src/aie_codegen.h`, which pulls in the public subsystem headers under the `aie_codegen_inc/` include layout used at install time.

**Supported AI Engine generations** include, among others, first-generation AIE, AI Engine ML (AIE-ML), AIE2 IPU, AIE2P (including Strix variants), AIE2PS, and AIE4-family devices. Generation-specific logic lives in parallel translation units (for example `*_aieml.c`, `*_aie4.c`) selected and linked as part of the unified library.

### `aie-regdb/` — register database (Git submodule)

Submodule: [Xilinx/aie-regdb](https://github.com/Xilinx/aie-regdb) on GitHub.

This tree holds **machine-oriented register definitions** (for example global parameter headers) that must stay in sync with silicon. **Always clone with submodules** (see [Getting the sources](#getting-the-sources)). If `aie-regdb/` is empty, your clone is incomplete and the library will not build correctly.

### `fal/` — Functional Abstraction Layer (**xaiefal**, C++)

**FAL** is a C++ header-oriented layer that provides **resource-oriented APIs** on top of the C driver—for example trace, performance counters, stream-switch resources, and profiling helpers. It is documented and built separately; see [`fal/README.md`](fal/README.md). CMake installs FAL headers alongside `aie_codegen` when you install the package (see `src/cmake/AieCodegenInstall.cmake`).

Touch **`fal/`** when you are exposing or consuming **C++** runtime services. Touch **`src/`** when you are changing register programming, backends, or generation-specific behavior.

### `tools/` — small validation utilities

Standalone scripts used during bring-up and validation of generated artifacts (for example consistency checks on `.asm` output). See [`tools/README.md`](tools/README.md).

### `driver/` — example programs

`driver/examples/` contains small standalone C programs (transaction reserialization, instruction-buffer validation) that link against `libaie_codegen.so` to exercise the driver directly. These are built locally with `make -f Makefile.Linux` rather than the CMake project; see [`driver/examples/README.md`](driver/examples/README.md) for the exact build/run steps.

### `.github/workflows/` — CI

GitHub Actions workflows run builds, static analysis, and integration checks. Pull requests may trigger jobs that clone and run external suites (for example **AigCtrlCodeValidationSuite**); see [Integration testing](#integration-testing).

### `license.txt`

Software is distributed under the **MIT** license ( SPDX-License-Identifier: MIT in individual files). See `license.txt` for the full text.

---

## Prerequisites

- **CMake** 3.21 or newer  
- **C compiler** with **C11** support (GCC or Clang on Linux; MSVC supported for Windows builds—see CMake options for MSVC-specific toggles)  
- **Ninja** (recommended generator; `ninja-build` package on many Linux distributions)

Optional:

- **CodeQL CLI** on `PATH` if you intend to run the `codeql` CMake target (see [Static analysis (CodeQL)](#static-analysis-codeql)).

---

## Building with CMake (supported path)

The CMake project lives under **`src/`**. From the **repository root**, a typical developer workflow uses an out-of-tree build directory and **Ninja**:

```bash
cmake -S src -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

This produces `libaie_codegen.so` on Linux (shared library by default) under `build/`, or the equivalent on Windows when using the Visual Studio or Ninja multi-config generators.

### Install

After a successful build:

```bash
cmake --install build --prefix /path/to/prefix
```

Installed artifacts include the library, `aie_codegen.h`, headers under `aie_codegen_inc/`, FAL headers under `xaiefal/`, and a CMake package so downstream projects can use `find_package(aie_codegen)` and link `aie_codegen::aie_codegen`.

### CMake options

| Option | Default | When to change it |
|--------|---------|-------------------|
| `AIE_CODEGEN_BUILD_SHARED` | `ON` | Set `OFF` to build a **static** archive instead of a shared library (common for firmware-style integration or single-binary delivery). |
| `CONTROLCODE_BACKEND` | `ON` | Leave on for normal control-code generation paths. Turn off only if you are deliberately building a reduced backend set and understand the implications. |
| `SOCKET_BACKEND` | `OFF` | Enable when the build must support the **socket** I/O path (remote or socket-mediated access to the backend). |
| `DEBUG_BACKEND` | `OFF` | Enable when you need the **debug** I/O backend compiled in. |
| `AIE_CODEGEN_ENABLE_WERROR` | `ON` (non-MSVC) | Set `OFF` locally if you need to compile while warnings are still being cleaned up (not recommended for merge-ready code). |

MSVC-only options (`AIE_CODEGEN_MSVC_RELEASE_PDB`, `AIE_CODEGEN_ENABLE_SOURCELINK`) are documented in `src/CMakeLists.txt` for compliance and symbol publishing scenarios.

### Compiler selection

You can pass the usual CMake variables when configuring, for example:

```bash
cmake -S src -B build -G Ninja -DCMAKE_C_COMPILER=clang
```

---

## Feature trimming (binary size)

Compile-time feature groups are controlled from C headers (not CMake switches). See the documentation in `src/global/xaie_feature_config.h` for macros such as `XAIE_FEATURE_APP_BASIC`, `XAIE_FEATURE_PRIVILEGED`, and `XAIE_FEATURE_ALL`. Choosing a smaller group reduces code size when you know which subsystems your integration needs.

---

## Integration testing

### AigCtrlCodeValidationSuite

Some CI workflows clone and execute **AigCtrlCodeValidationSuite**, an external repository used to validate control-code generation and related flows. The canonical clone URL and branch override mechanism are described in [`.github/workflows/Aigctrl_code_validation_workflow.yml`](.github/workflows/Aigctrl_code_validation_workflow.yml). Maintainers can point validation at a fork or topic branch using the `AigCtrlCodeValidationSuite-repo:` and `AigCtrlCodeValidationSuite-branch:` lines in a pull request body, as documented in that workflow.

### Other workflows

Browse [`.github/workflows/`](.github/workflows/) for Windows builds, Coverity, transaction validation, FAL tests, and product-specific sanity pipelines.

---

## Static analysis (CodeQL)

If the CodeQL CLI is installed, the build exposes a `codeql` target that creates a database, runs the `cpp-security-extended` query suite, and writes SARIF plus a text summary (see `src/cmake/AieCodegenCodeQL.cmake` and `tools/sarif_to_text.py`). Example:

```bash
cmake -S src -B build -G Ninja
cmake --build build --target codeql
```

Tune output locations with `CODEQL_DB_DIR`, `CODEQL_RESULTS_DIR`, and `CODEQL_QUERY_SUITE` if needed (defaults are under the build directory).

---

## Version

The CMake project version is declared in `src/CMakeLists.txt` (`project(aie_codegen ... VERSION ...)`) and tracks the **soname / package** version of the library.

---

## Contributing

- Match existing copyright headers and **SPDX** tags in touched files.  
- Prefer **CMake** builds for local verification; keep README instructions aligned with CMake so onboarding stays consistent (internal-only legacy Makefile flows are not documented here by design).  
- Run relevant CI workflows or local builds before opening a pull request.

---

## See also

- [`fal/README.md`](fal/README.md) — building and testing **xaiefal**  
- [`tools/README.md`](tools/README.md) — helper scripts under `tools/`  
- [`driver/examples/README.md`](driver/examples/README.md) — building/running the example programs under `driver/`  
- [`aie-regdb/README.md`](aie-regdb/README.md) — submodule purpose (register DB)  
- [`MIGRATION_GUIDE.md`](MIGRATION_GUIDE.md) — migrating from **aie-rt** to **aie-codegen** (API changes, removed lite-driver APIs, AIE2PS support)  
- Internal tracking: **AIESW-33699** — README cleanup scope and rationale  
