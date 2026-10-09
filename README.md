# GT Advance 3 decompilation

Reconstruction of *GT Advance 3: Pro Concept Racing* (GBA, MTO/THQ).
The goal is readable game source and cataloged data that build a ROM
**byte-identical** to the US reference.

## Status

The full independent C ROM is **not complete**.

The reference build (`make`) produces a byte-identical ROM from reconstructed
assembly, verified generated data regions, and zero padding.
The `make matching-ready` target
compiles the C corpus with pinned `old_agbcc` and verifies selected C functions
in an independent executable slice.

The manifest selects **1,089 C functions / 50,212 bytes** within the
**188,760-byte** executable slice. The remaining executable bytes are
reconstructed assembly. Remaining work includes C matching, the complete
independent link.
The [integrated data regions](docs/data-integration.md) cover all cataloged
data through content end `0x7B04C4` (7,873,388 bytes; padding excluded),
regenerated from private editable inputs.
See [build status and completion criteria](docs/decompilation-roadmap.md).

## Repository map

| Path | Contents |
| --- | --- |
| `src/`, `include/` | Reconstructed C and hardware contracts. |
| `asm/` | Reference assembly, ROM layout, and the private data-tail input. |
| [Build verification](docs/matching_workflow.md) | Verification commands, ownership, and byte-matching constraints. |
| [Compiler documentation](docs/compiler_status.md) | Compiler configuration and runtime provenance. |
| [ROM map](docs/rom_map.md) | Memory layout and data inventories. |
| `docs/findings/` | Compiler behavior and worked matching constraints. |
| [Tools](tools/README.md) | Build, analysis, extraction, and emulator utilities. |
| `baserom.sha256` | Pinned US reference-ROM hash. |

## Setup and verification

Use Python **3.11 or newer**. Supply your own US ROM as `baserom.gba`; it is
ignored by Git. The build needs GNU Make, Git, a host C/C++ compiler, Clang,
ARM GCC/binutils (`arm-none-eabi`), `shasum`, and Python `pyelftools`.

On Ubuntu 24.04:

```sh
sudo apt-get update
sudo apt-get install build-essential git clang python3 python3-venv \
    binutils-arm-none-eabi gcc-arm-none-eabi libnewlib-arm-none-eabi libdigest-sha-perl
```

The [Ubuntu ARM compiler package](https://packages.ubuntu.com/noble/gcc-arm-none-eabi)
provides the modern compiler used for source-equivalence checks.

On macOS, install Apple's Command Line Tools and [Homebrew](https://brew.sh/),
then install Python and the [ARM toolchain](https://formulae.brew.sh/cask/gcc-arm-embedded):

```sh
xcode-select --install  # once, if Command Line Tools are not already installed
brew install python@3.11
brew install --cask gcc-arm-embedded
```

Use `python3.11` below if `python3 --version` still reports an older Python.
Apple's Command Line Tools provide Git, Make, and Clang; macOS includes `shasum`.

From the repository root, create an isolated Python environment and provision
the matching compiler from the pinned `pret/agbcc` revision:

```sh
python3 -m venv .venv
. .venv/bin/activate
python3 -m pip install -r requirements.txt
make toolchain
make matching-ready
make fast-check
make ci-check
```

`make matching-ready` includes the reference build and checks the pinned US
ROM hash. `make` alone runs that reference build and hash verification.
`make progress` runs the full matching gate and refreshes the progress snapshot
when build inputs change; see [CONTRIBUTING.md](CONTRIBUTING.md).

`make toolchain` builds `old_agbcc`, the comparison `agbcc` binary, and their
headers under `build/toolchains/agbcc/`. The reference assembly build does not
require these compilers. See the [tool index](tools/README.md) for details.
Install Python dependencies in a virtual environment if required by your
Python installation. Pillow is optional for asset PNG export and treemaps;
emulator bindings are provisioned separately by `tools/build_mgba_python.sh`.

The runtime-provenance check also uses the vendored
[GCC 2.95.3 runtime source](tools/third_party/gcc-2.95.3/README.md).
Its original license and notices remain separate from this project's license.

`make clean` removes build outputs, including the auxiliary `build-code/`,
`build-slice/`, `build-c/`, and `build-s/` trees, while preserving
`build/toolchains/`. `make distclean` removes those outputs and the compiler tree.
Do not commit the private ROM, extracted assets, or generated binaries.
The reference-build hash and executable-slice match do not establish a complete
independent C ROM.

## Contributing and CI

See [CONTRIBUTING.md](CONTRIBUTING.md) for contribution and verification steps.
Hosted CI runs lightweight, ROM-free checks. [Progress reporting](docs/ci.md)
documents the locally verified decomp.dev snapshot and one-time registration.
`objdiff.json` is an unconfigured placeholder; see the
[optional viewer setup](tools/README.md#optional-assembly-viewers).

## License

The reconstruction, tooling, and documentation are dedicated to the public
domain under [CC0 1.0 Universal](LICENSE). This does not apply to the original
game or third-party material. See [NOTICE](NOTICE) for ownership and license
boundaries. No complete ROM image or extracted asset files are distributed here.
The cartridge-header constants and Nintendo boot logo are excluded from CC0;
see [NOTICE](NOTICE).
