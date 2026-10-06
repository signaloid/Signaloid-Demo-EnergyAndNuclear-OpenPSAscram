[<img src="https://assets.signaloid.io/add-to-signaloid-cloud-logo-dark-latest.png#gh-dark-mode-only" alt="[Add to signaloid.io]" height="30">](https://signaloid.io/repositories?connect=https://github.com/signaloid/Signaloid-Demo-EnergyAndNuclear-OpenPSAscram#gh-dark-mode-only)
[<img src="https://assets.signaloid.io/add-to-signaloid-cloud-logo-light-latest.png#gh-light-mode-only" alt="[Add to signaloid.io]" height="30">](https://signaloid.io/repositories?connect=https://github.com/signaloid/Signaloid-Demo-EnergyAndNuclear-OpenPSAscram#gh-light-mode-only)


# PRA Fault Tree Uncertainty Propagation with Signaloid UxHw® Support

This application propagates the uncertainty in basic event failure
probabilities through four fault tree models from nuclear probabilistic risk
assessment (PRA), with support for Signaloid's distributional arithmetic
(UxHw) as an alternative to traditional Monte Carlo sampling. The models are
hand-coded in C from the redundancy patterns of the Fault Tree Handbook and
from the SCRAM input corpus. The repository has been structured to fit within
the
[Signaloid UxHw benchmarking template](https://github.com/signaloid/Signaloid-Demo-Benchmarking-C-Template.git),
allowing direct comparison between traditional Monte Carlo sampling and
Signaloid's single-pass uncertainty tracking on the Signaloid Cloud
Compute Engine (SCCE).



## Cloning the repository

```bash
git clone --recursive https://github.com/signaloid/Signaloid-Demo-EnergyAndNuclear-OpenPSAscram.git
```

To update all submodules:
```bash
git pull --recurse-submodules
git submodule update --remote --recursive
```

If you forgot to clone with `--recursive`:
```bash
git submodule update --init --recursive
```


## Quickstart

### Native build and run
```bash
make local-build              # Build the executable
make local-run                # Build and run all 4 outputs with timing

# Run individual outputs:
./demo-native-mc -M 10000 -S 0 -T    # SmallTree with 10000 MC samples and timing
./demo-native-mc -M 10000 -S 2       # HIPPS with 10000 MC samples
./demo-native-mc -M 10000 -S 0 -j    # SmallTree with JSON output
```

## Overview

In traditional Monte Carlo mode (`-M N`), the application draws N independent
samples of every basic event probability, evaluates the selected fault tree on
each, and reports the mean, variance, minimum and maximum of the N top-event
probabilities. `-M 1` is a one-sample Monte Carlo run, not the UxHw mode.

In UxHw single-shot mode (no `-M`, on the Signaloid Cloud Compute Engine), the
application sets each basic event probability to its full distribution once
and evaluates the fault tree in a single pass. Signaloid's uncertainty-tracking
hardware propagates the distributions through the gate arithmetic, so the
top-event probability is itself a distribution. Natively the same mode runs
against the compatibility layer and yields one random sample.

The uncertainty here is epistemic. The failure probability of each basic
event is not known exactly, and its distribution describes the spread of
estimates for it. This is the quantity that the Monte Carlo uncertainty
analysis in SCRAM and other Open-PSA tools samples.

## Fault tree models

The four models are small, so the point is the uncertainty propagation rather
than the model size. Full definitions, basic event parameters, and the gate
formulas are in [`docs/FAULT_TREE_MODELS.md`](docs/FAULT_TREE_MODELS.md).

| `-S` | Model | Top event | Basic events | Gates |
|------|-------|-----------|--------------|-------|
| 0 | SmallTree | `(e1 AND e2) OR (e3 AND e4)` | 4 | 3 |
| 1 | TwoTrain | `(Valve1 OR Pump1) AND (Valve2 OR Pump2)` | 4 | 3 |
| 2 | HIPPS | `2-of-3(S1, S2, S3) OR LogicSolver OR (Valve1 AND Valve2)` | 6 | 3 |
| 3 | SeriesSystem | `e1 OR e2 OR e3 OR e4 OR e5` | 5 | 1 |

Every basic event is lognormal, given as a mean, an error factor, and a 0.95
confidence level, following the convention of NUREG/CR-6823. Gate evaluation
is plain arithmetic with no branch on a probability value. The AND and OR
gates use each child once, so their distributional propagation is exact on
every core. The K-of-N gate reuses each child in several subset terms, so its
propagation is exact only on a core that tracks correlation. See
[Correlation handling](#correlation-handling).

## Output indices

| `-S` | Name | Description |
|------|------|-------------|
| 0 | SmallTree | Top-event probability of `(e1 AND e2) OR (e3 AND e4)` |
| 1 | TwoTrain | Top-event probability of the redundant two-train system |
| 2 | HIPPS | Top-event probability of the safety system with 2-of-3 voting |
| 3 | SeriesSystem | Top-event probability of the 5-event series |
| 4 | All outputs | Every model above (UxHw mode only) |

## Building and running

### Prerequisites (native build)
- GCC with C11 support
- GSL (GNU Scientific Library): `sudo apt install libgsl-dev`

### Native build and run
```bash
make local-build              # Build the executable
make local-run                # Build and run all 4 outputs with timing

# Run individual outputs:
./demo-native-mc -M 10000 -S 0 -T    # SmallTree with 10000 MC samples and timing
./demo-native-mc -S 0                # UxHw kernel (natively one random sample)
./demo-native-mc -S 4                # Every output, UxHw kernel
./demo-native-mc -M 10000 -S 0 -j    # JSON output
```

### Command-line options
```
Common options:
  -S <index>       Select output (0-3, see table above). 4, or no -S,
                   computes all outputs and is allowed in UxHw mode only
  -M <iterations>  Monte Carlo mode with N samples. N is required and has no
                   default. Without -M the UxHw kernel runs
  -i <path>        Accepted by the common parser but ignored. This demo has no
                   CSV inputs, every basic event is defined in the source
  -o <path>        Write the outputs to a CSV file
  -T               Enable timing
  -b               Benchmarking output format
  -j               JSON output mode
  -v               Verbose (accepted but not supported by this demo)
  -h               Help
```

This demo adds no options of its own.

## Architecture

### Execution modes
There is one kernel, `calculateOutput()` in `kernel.c`. `main.c` calls it once
in UxHw mode and once per iteration in Monte Carlo mode. Each call builds the
selected models, creates one distributional probability per basic event
through the UxHw API, and evaluates the tree.

- **Monte Carlo mode** (`-M N`): on each of N calls the compatibility layer backs the UxHw calls with GSL and returns a fresh sample of every basic event. `main.c` records the selected output per iteration, saves the samples to `data.out`, and the common print routines report their statistics.
- **UxHw single-shot mode** (no `-M`): one call. On SCCE every basic event is a full distribution, and so is every output.

### Module flow
```
pra-models.c (build tree)
  -> pra-distributions.c   (one UxHw distribution per basic event)
    -> pra-faultTree.c      (recursive AND, OR, K-of-N evaluation)
      -> outputVariables    (common.c printing, data.out in Monte Carlo mode)
```

### Correlation handling
The code makes no independence calls and leaves every correlation to the
core. The basic events are independent, and the AND and OR gates use each
child once, so the SmallTree, TwoTrain and SeriesSystem outputs are exact on
every core. The K-of-N gate in HIPPS sums four subset terms that share the
same three sensor probabilities, so those terms are correlated. On a core
that tracks correlation the HIPPS output matches Monte Carlo. On a core that
treats operands as independent, such as Athens with correlation tracking
disabled, the HIPPS mean is still exact but its spread is understated by
about 6 percent.



## References

This project bridges nuclear PRA fault tree analysis and Signaloid (hardware-accelerated distributional arithmetic). The models are hand-coded from the standard PRA references below and from the SCRAM input corpus, executed on Signaloid's SCCE which implements the Laplace microarchitecture for tracking probability distributions through computation.

### Fault tree analysis

1. **Vesely, W. E., Goldberg, F. F., Roberts, N. H., and Haasl, D. F.** (1981). *Fault Tree Handbook*. NUREG-0492. U.S. Nuclear Regulatory Commission. [nrc.gov](https://www.nrc.gov/reading-rm/doc-collections/nuregs/staff/sr0492/). The foundational reference for fault tree analysis methodology, including the AND, OR, and K-of-N gate types used in this implementation.

2. **Rauzy, A.** (1993). "New algorithms for fault trees analysis." *Reliability Engineering & System Safety*, 40(3), 203--211. DOI: [10.1016/0951-8320(93)90060-C](https://doi.org/10.1016/0951-8320(93)90060-C). Introduces the BDD-based methods for exact fault tree probability calculation used by the SCRAM analysis engine.

### PRA parameter estimation

3. **Atwood, C. L., LaChance, J. L., Martz, H. F., Anderson, D. J., Englehardt, M., Whitehead, D., and Wheeler, T.** (2003). *Handbook of Parameter Estimation for Probabilistic Risk Assessment*. NUREG/CR-6823, SAND2003-3348P. U.S. Nuclear Regulatory Commission. [nrc.gov](https://www.nrc.gov/reading-rm/doc-collections/nuregs/contract/cr6823/). Defines the lognormal parameterization (mean, error factor, confidence level) used for the basic event failure probabilities in this implementation.

### Open-PSA and SCRAM

4. **Open-PSA Initiative.** *Model Exchange Format (MEF) v2.0*, Chapter 5, Stochastic Layer. [open-psa.github.io/mef](https://open-psa.github.io/mef/). Specifies the standard for representing probabilistic risk models, including the random deviate types and fault tree gate definitions.

5. **Rakhimov, O.** *SCRAM: Command-line Risk Analysis Multi-tool*. [github.com/rakhimov/scram](https://github.com/rakhimov/scram). An open-source PRA tool implementing the Open-PSA MEF, and the source of the SmallTree model.

### Signaloid and uncertainty-tracking computation

6. **Tsoutsouras, V., Kaparounakis, O., Bilgin, B., Samarakoon, C., Meech, J., Heck, J., and Stanley-Marbell, P.** (2021). "The Laplace Microarchitecture for Tracking Data Uncertainty and Its Implementation in a RISC-V Processor." In *MICRO-54: 54th Annual IEEE/ACM International Symposium on Microarchitecture*, pp. 1254--1269. ACM. DOI: [10.1145/3466752.3480131](https://doi.org/10.1145/3466752.3480131). This is the foundational paper for Signaloid's technology, introducing the Laplace microarchitecture that tracks probability distributions through computation on a RISC-V processor and defining the ISA extensions that became the UxHw API.

7. **Bilgin, B. A., Elias, O. H., Selby, M., and Stanley-Marbell, P.** (2025). "Quantization of Probability Distributions via Divide-and-Conquer: Convergence and Error Propagation under Distributional Arithmetic Operations." arXiv preprint arXiv:2505.15283. DOI: [10.48550/arXiv.2505.15283](https://doi.org/10.48550/arXiv.2505.15283). Provides mathematical foundations for how Signaloid's processor discretizes continuous probability distributions, with error bounds for distributional arithmetic operations.

8. **Signaloid.** *UxHw API Documentation*. [docs.signaloid.io/docs/uxhw-api](https://docs.signaloid.io/docs/uxhw-api/). Documents the UxHw API functions (`UxHwDoubleLognormalDist` and the other distribution constructors) used to create the distributional values that the Signaloid C0 processor propagates through arithmetic.
