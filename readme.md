# PACE 2020
This repository contains solvers for treedepth, submitted from team xuzijian629 at [PACE 2020](https://pacechallenge.org/2020/td/).

Authors: Zijian Xu, Dejun Mao and Vorapong Suppakitpaisarn from The University of Tokyo

UPD: We were ranked in [5th place](https://pacechallenge.org/2020/results/) in the competition.

# Algorithm
We enumerate minimal separtors to construct treedepth decomposition in a top-down way. A quite effective pruning is done by computing exact treedepth of many small components throughout the input graph.
## Solver description
A detailed explanation of our algorithm is provided [here](https://github.com/xuzijian629/pace2020/blob/master/submissions/solver-description-xuzijian629.pdf).

# Installation
- GCC version 5.4.0 or later

Our solver compiles source code, after checking the size of the input graph. So, just run `src/solver < input.gr`.

# Input format
Input graph format is specified [here](https://pacechallenge.org/2020/td/).

# Directories
- Source code for the solver is stored in `src/`.
- Other directories such as `benchmark/` and `test/` are for authors' personal use (local experiments).

# LICENSE
[MIT LICENSE](https://github.com/xuzijian629/pace2020/blob/master/LICENSE.md)
