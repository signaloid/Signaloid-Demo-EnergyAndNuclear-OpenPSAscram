# Source Code

## `main.c`
Entry point. Parses the command line, runs the kernel once in UxHw® mode or
once per iteration in Monte Carlo mode, and prints or saves the selected
outputs through the common routines.

## `kernel.c` and `kernel.h`
The calculation kernel, `calculateOutput()`. For each selected model it
builds the fault tree, creates one distributional probability per basic
event, and evaluates the tree to the top-event probability.

## `pra-faultTree.c` and `pra-faultTree.h`
Fault tree data structures and the recursive gate-by-gate evaluation of AND,
OR, and K-of-N gates. All operations are plain arithmetic with no branches on
probability values and no independence calls. The correlation between the
subset terms of a K-of-N gate is left to the core.

## `pra-distributions.c` and `pra-distributions.h`
Conversion of the Open-PSA lognormal parameters (mean, error factor,
confidence level) to the underlying normal (mu, sigma), and creation of the
basic event distributions through the UxHw API.

## `pra-models.c` and `pra-models.h`
The four hard-coded fault tree models, SmallTree, TwoTrain, HIPPS, and
SeriesSystem, built from statically allocated nodes. See
[`docs/FAULT_TREE_MODELS.md`](../docs/FAULT_TREE_MODELS.md) for their
structure and parameters.

## `utilities.c` and `utilities.h`
Demo-specific command-line handling and the output index enum, layered on the
common arguments handled by `common.c`. This demo adds no options of its own.

## `common.c` and `common.h`
Utility methods for parsing, setting, and reporting the usage of command-line
arguments common to all of our C/C++ demo applications, as well as other
methods commonly used across them, such as CSV and JSON I/O. These source
files are symlinks to the original files contained in the repository
[Signaloid-Demo-CommonUtilityRoutines](https://github.com/signaloid/Signaloid-Demo-CommonUtilityRoutines),
which is included as a submodule in `submodules/common`.

## `uxhw.c` and `uxhw.h`
Methods that implement the probabilistic versions of the methods in the UxHw
API (e.g., `UxHwDoubleLognormalDist`) using the GNU Scientific Library (GSL)
random number generators. This allows building our C/C++ demo applications
natively (i.e., on conventional architectures) and running native Monte Carlo
evaluations without modifying the source code. These source files are symlinks
to the original files contained in the repository
[Signaloid-Demo-UxHwCompatibilityForNativeExecution](https://github.com/signaloid/Signaloid-Demo-UxHwCompatibilityForNativeExecution),
which is included as a submodule in `submodules/compat`.

## `config.mk`
The source list shared by the Signaloid cloud build and the native build. New
source files must be added here.

## Building natively
Use the top-level `Makefile`, which reads `config.mk`, adds `uxhw.c` and links
GSL:

```bash
make local-build
```
