# Fault Tree Models

This demo evaluates the top-event probability of four fault trees drawn from
nuclear probabilistic risk assessment (PRA). Each basic event has an uncertain
failure probability. The same evaluation runs either as local Monte Carlo
(GSL) or as a single-pass distributional computation on the Signaloid Cloud
Compute Engine (SCCE), and the two should agree.

The uncertainty is epistemic. A basic event's failure probability is not known
exactly, and the distribution describes the spread of estimates for it. This
is the same quantity that the Monte Carlo uncertainty analysis in SCRAM and
other Open-PSA tools samples.

| `-S` | Model | Top event | Basic events | Gates |
|------|-------|-----------|--------------|-------|
| 0 | SmallTree | `(e1 AND e2) OR (e3 AND e4)` | 4 | 3 |
| 1 | TwoTrain | `(Valve1 OR Pump1) AND (Valve2 OR Pump2)` | 4 | 3 |
| 2 | HIPPS | `2-of-3(S1, S2, S3) OR LogicSolver OR (Valve1 AND Valve2)` | 6 | 3 |
| 3 | SeriesSystem | `e1 OR e2 OR e3 OR e4 OR e5` | 5 | 1 |
| 4 | All of the above | | | |

## Basic event distributions

Every basic event is lognormal and is given as a mean, an error factor `EF`,
and a confidence level, following the convention of NUREG/CR-6823 and the
Open-PSA Model Exchange Format. The confidence level is 0.95 for every event
in this demo. The parameters of the underlying normal distribution are

```
sigma = ln(EF) / z_p
mu    = ln(mean) - sigma^2 / 2
```

where `z_p` is the `p`-quantile of the standard normal, 1.6449 for
`p = 0.95`. The conversion is in `praLognormalMefToMuSigma()` in
`src/pra-distributions.c`, and the distribution is created with
`UxHwDoubleLognormalDist(mu, sigma)`. The code also supports point, uniform,
and normal basic events, but no shipped model uses them.

## Gate evaluation

Gates are evaluated recursively from the leaves up, following the Fault Tree
Handbook (NUREG-0492). For child probabilities `p_i`

| Gate | Probability |
|------|-------------|
| AND | `product(p_i)` |
| OR | `1 - product(1 - p_i)` |
| K-of-N | Sum over every subset of at least K children of `product(p_i)` over the subset times `product(1 - p_i)` over its complement |

These formulas are exact for independent basic events in a tree-structured
fault tree. Every operation is plain arithmetic with no branch on a
probability value. The AND and OR gates use each child once, so their
distributional propagation is exact on every core. The K-of-N gate reuses
each child in several subset terms, which are therefore correlated. The code
makes no independence calls and leaves that correlation to the core. On a
core that tracks correlation the HIPPS top event matches Monte Carlo. On a
core that treats operands as independent the HIPPS mean is still exact but
its spread is understated by about 6 percent.

## Model 0, `SmallTree`

Derived from the SCRAM input corpus, `input/SmallTree/SmallTree.xml`. Two
AND gates under an OR gate.

| Basic event | Mean | Error factor |
|-------------|------|--------------|
| `e1` | 2.0e-5 | 3 |
| `e2` | 1.0e-5 | 3 |
| `e3` | 2.0e-5 | 3 |
| `e4` | 1.0e-5 | 3 |

In the XML, `lambda4` refers to `lambda2`, so `e4` shares the parameters of
`e2`. The top-event probability is of order 4e-10, the smallest of the four
models.

## Model 1, `TwoTrain`

The classic PRA redundancy pattern from NUREG-0492. Two identical trains fail
independently, and the system fails only if both do. Each train fails if its
valve or its pump fails.

| Basic event | Mean | Error factor |
|-------------|------|--------------|
| `Valve1` | 1.0e-3 | 3 |
| `Pump1` | 1.0e-4 | 10 |
| `Valve2` | 1.0e-3 | 3 |
| `Pump2` | 1.0e-4 | 10 |

The pumps carry the wide error factor of 10. The top-event probability is of
order 1e-6.

## Model 2, `HIPPS`

A high integrity pressure protection system. The system fails if the sensor
subsystem fails, the logic solver fails, or the final element fails. The
sensor subsystem is a 2-of-3 voting gate, and the final element is two valves
in parallel.

| Basic event | Mean | Error factor |
|-------------|------|--------------|
| `Sensor1` | 5.0e-3 | 3 |
| `Sensor2` | 5.0e-3 | 3 |
| `Sensor3` | 5.0e-3 | 3 |
| `LogicSolver` | 1.0e-4 | 5 |
| `FinalValve1` | 1.0e-3 | 3 |
| `FinalValve2` | 1.0e-3 | 3 |

This is the most demanding model. The K-of-N gate enumerates all eight
subsets of its three children, and the model has the most basic events. The
top-event probability is of order 2e-4.

## Model 3, `SeriesSystem`

Five basic events under a single OR gate. The output is well characterized,
which makes this model the cleanest check of Monte Carlo against the
distributional result.

| Basic event | Mean | Error factor |
|-------------|------|--------------|
| `e1` | 1.0e-3 | 3 |
| `e2` | 2.0e-3 | 5 |
| `e3` | 5.0e-4 | 3 |
| `e4` | 1.0e-3 | 10 |
| `e5` | 3.0e-3 | 3 |

The top-event probability is of order 7e-3, the largest of the four models.
