# Interpreting the Tape of Life: Ancestry-based Analyses Provide Insights and Intuition about Evolutionary Dynamics

This repository contains the code, configuration files, analyses, and extra documentation
for the paper "Interpreting the Tape of Life: 
Ancestry-based Analyses Provide Insights and Intuition about Evolutionary Dynamics"
(to appear in the Artificial Life Journal).

**Navigation:**

<!-- TOC -->

- [Overview](#overview)
  - [Authors](#authors)
  - [Abstract](#abstract)
  - [Repository Contents](#repository-contents)
- [Study Systems](#study-systems)
  - [Niching Competition Benchmark Problems](#niching-competition-benchmark-problems)
  - [Avida Digital Evolution Platform](#avida-digital-evolution-platform)
- [Analyses](#analyses)
- [References](#references)

<!-- /TOC -->

## Overview

In this work, we propose a suite of diagnostic analysis techniques that operate on
lineages and phylogenies in digital evolution experiments. We demonstrate the proposed
suite of analysis techniques in two well-studied contexts: (1) a set of two-dimensional,
real-valued optimization problems from [the GECCO Competition on Niching Methods](https://github.com/mikeagn/CEC2013/), and (2) a set of qualitatively different environments
in the [Avida Digital Evolution Platform](https://avida.devosoft.org/).

Below are the set of lineage and phylogeny analysis techniques discussed in this work. 
Many of these metrics/analysis techniques are drawn from biology, while others are
more specific to digital systems (as they require high-resolution data collection
that can be infeasible in biological systems).

- Lineage-based metrics
  - Lineage length
  - Mutation accumulation
  - Phenotypic volatility
- Phylogeny-based metrics
  - Depth of the most-recent common ancestor (MRCA depth)
  - Phylogenetic richness
  - Phylogenetic divergence
  - Phylogenetic regularity
- Visualizations
  - State sequence visualizations
  - Fitness landscape overlays
  - Phylogenetic trees
  - Muller plots 

For more details on each metric/visualization (including further reading for each),
refer to the paper.

### Authors

- [Emily Dolson](http://emilyldolson.com/)
- [Alexander Lalejini](https://lalejini.com)
- [Steven Jorgensen](https://stevenjorgensen.com/)
- Charles Ofria

### Abstract

> Fine-scale evolutionary dynamics can be challenging to tease out when focused on the broad brush strokes of whole populations over long time spans. We propose a suite of diagnostic analysis techniques that operate on lineages and phylogenies in digital evolution experiments with the aim of improving our capacity to quantitatively explore the nuances of evolutionary histories in digital evolution experiments. We present three types of lineage measurements: lineage length, mutation accumulation, and phenotypic volatility. Additionally, we suggest the adoption of four phylogeny measurements from biology: phylogenetic richness, phylogenetic divergence, phylogenetic regularity, and depth of the most-recent common ancestor. In addition to quantitative metrics, we also discuss several existing data visualizations that are useful for understanding lineages and phylogenies: state sequence visualizations, fitness landscape overlays, phylogenetic trees, and Muller plots. We examine the behavior of these metrics (with the aid of data visualizations) in two well-studied computational contexts: (1) a set of two-dimensional, real-valued optimization problems under a range of mutation rates and selection strengths, and (2) a set of qualitatively different environments in the Avida Digital Evolution Platform.  These results confirm our intuition about how these metrics respond to various evolutionary conditions and indicate their broad value.

### Repository Contents

## Study Systems

### Niching Competition Benchmark Problems

To gain a broad understanding of our metrics, we applied them to four two-dimensional, real-valued
benchmark optimization problems from the GECCO Competition on Niching Methods: 

![alt-text](./figs/landscapes.png)

- (A) Himmelblau
- (B) Six-Humped Camel Back 
- (C) Shubert
- (D) Composition Function 2 

For each test problem, the X and Y coordinates offered by a given organism are translated by the function into a fitness value. Because of their low dimensionality, we can fully visualize each problem's actual fitness landscape, allowing us to directly view how our ancestry-based metrics respond to the actual paths evolved lineages take through the fitness landscape under different conditions. 

We used the implementations of these problems at [https://github.com/mikeagn/CEC2013](https://github.com/mikeagn/CEC2013) (C++ for fitness calculations during evolution, Python for post-hoc analysis).

### Avida Digital Evolution Platform

Avida is a well-established artificial life system that has been used to study a wide range of evolutionary dynamics.
See (Ofria and Wilke, 2004) for more details on Avida.

The canonical implementation of Avida can be found [here](https://avida.devosoft.org/).
Because some of our metrics required extra data tracking not found in Avida by default,
the Avida implementation used in this work can be found [here](https://github.com/emilydolson/avida-empirical).

We applied our metrics/visualizations to four different environments in Avida:

- (1) a **minimal environment** where selection is entirely focused on the efficiency
  at which organisms replicate
- (2) the **logic-9 environment** where organisms are rewarded for performing all nine
  non-trivial one- and two-input boolean logic functions: NAND, NOT, OR-NOT, AND, OR, AND-NOT, NOR, XOR, and EQUALS.
- (3) the **limited-resource environment** where organisms are rewarded for performing
  nine logic tasks; however, each task is associated with a limited pool of resources.
  When an organism performs a logic task, it collects the appropriate task-associated
  resource in proportion to that resource's current availability and is rewarded
  based on how many resources it has collected.
- (4) the **simple changing environment** cycles between rewarding and punishing
  the NAND and NOT tasks.

For more details about each environment, refer to our paper.

## Analyses

http://lalejini.com/interpreting_the_tape_of_life/web/chg_env_lineage.html


## References

Li, X., Engelbrecht, A., and Epitropakis, M. G. (2013).  Benchmark Functions for CEC’2013 Special Session and Competition on
Niching Methods for Multimodal Function Optimization. Technical report, RMIT University, Australia.

Ofria, C. and Wilke, C. O. (2004).  Avida: A software platform for research in computational evolutionary biology.
Artificial Life, 10(2):191–229.