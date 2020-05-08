#pragma once
#include "tw.cpp"

int treedepth_lb(const Graph& g) { return treewidth_lb(g) + 1; }
