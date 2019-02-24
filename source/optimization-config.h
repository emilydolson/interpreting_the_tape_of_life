#ifndef TOY_CONFIG_H
#define TOY_CONFIG_H

#include "config/config.h"

EMP_BUILD_CONFIG( OptimizationConfig,
  GROUP(DEFAULT_GROUP, "General Settings"),
  VALUE(RUN_MODE, size_t, 0, "What mode are we running in? \n0: Native experiment\n1: Analyze mode"),
  VALUE(RANDOM_SEED, int, -1, "Random number seed (negative value for based on time)"),
  VALUE(POP_SIZE, size_t, 1000, "Total population size"),
  VALUE(GENERATIONS, size_t, 5000, "How many generations should we run evolution?"),
  VALUE(PROBLEM, size_t, 0, "Which problem?"),
  VALUE(HINT_GRID_RES, size_t, 32, "Lexicase/eco-ea grid will be HINT_GRID_RES x HINT_GRID_RES in size."),
  GROUP(SELECTION_GROUP, "Selection Settings"),
  VALUE(SELECTION_METHOD, size_t, 0, "Which selection method are we using? \n0: Tournament\n1: Lexicase\n2: Eco-EA (resource)\n3: MAP-Elites\n4: Roulette\n5: Random drift"),
  VALUE(ELITE_SELECT__ELITE_CNT, size_t, 1, "How many elites get free reproduction passes?"),
  VALUE(TOURNAMENT_SIZE, size_t, 4, "How big are tournaments when using tournament selection or any selection method that uses tournaments?"),
  VALUE(LEXICASE_EPSILON, double, 0.0, "Defines epsilon value for lexicase selection (score_ceil-score_floor)*THIS=epsilon"),
  VALUE(RESOURCE_SELECT__RES_AMOUNT, double, 50.0, "Initial resource amount (for all resources)"),
  VALUE(RESOURCE_SELECT__RES_INFLOW, double, 50.0, "Resource in-flow (amount)"),
  VALUE(RESOURCE_SELECT__OUTFLOW, double, 0.05, "Resource out-flow (rate)"),
  VALUE(RESOURCE_SELECT__FRAC, double, 0.0025, "Fraction of resource consumed."),
  VALUE(RESOURCE_SELECT__MAX_BONUS, double, 5.0, "What's the max bonus someone can get for consuming a resource?"),
  VALUE(RESOURCE_SELECT__COST, double, 0.0, "Cost of using a resource?"),
  GROUP(MUTATION_GROUP, "Mutation Settings"),
  VALUE(MUTATION_STD, double, 0.1, "For each dimension: dimenstion std = MUTATION_STD*(upperbound[dim]-lowerbound[dim])"),
  GROUP(DATA_GROUP, "Data Collection Settings"),
  VALUE(SYSTEMATICS_INTERVAL, size_t, 100, "Interval to record systematics summary stats."),
  VALUE(FITNESS_INTERVAL, size_t, 100, "Interval to record fitness summary stats."),
  VALUE(POP_SNAPSHOT_INTERVAL, size_t, 5000, "Interval to take a full snapshot of the population."),
  VALUE(DATA_DIRECTORY, std::string, "./", "Location to dump data output."),
)

#endif
