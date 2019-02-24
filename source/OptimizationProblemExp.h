#ifndef TOY_PROBLEM_EXP_H
#define TOY_PROBLEM_EXP_H

// @includes
#include <iostream>
#include <string>
#include <utility>
#include <fstream>
#include <sys/stat.h>
#include <algorithm>
#include <functional>
#include <ctime>

#include "base/Ptr.h"
#include "base/vector.h"
#include "control/Signal.h"
#include "Evolve/World.h"
#include "Evolve/Resource.h"
#include "Evolve/SystematicsAnalysis.h"
#include "Evolve/World_output.h"
#include "hardware/InstLib.h"
#include "tools/BitVector.h"
#include "tools/Random.h"
#include "tools/random_utils.h"
#include "tools/math.h"
#include "tools/string_utils.h"

#include "optimization-config.h"

#include "cec2013.h"

constexpr size_t RUN_ID__EXP = 0;
constexpr size_t RUN_ID__ANALYSIS = 1;

constexpr size_t DIMENSIONS = 2;

constexpr size_t SELECTION_METHOD_ID__TOURNAMENT = 0;
constexpr size_t SELECTION_METHOD_ID__LEXICASE = 1;
constexpr size_t SELECTION_METHOD_ID__ECOEA = 2;
constexpr size_t SELECTION_METHOD_ID__MAPELITES = 3;
constexpr size_t SELECTION_METHOD_ID__ROULETTE = 4;
constexpr size_t SELECTION_METHOD_ID__DRIFT = 5;

const emp::vector<std::string> MUTATION_TYPES = {"normal", "normal_x", "normal_y"};

// Available problems: (1-indexed in cpp library)
// 0) F4 (2D)  --> Index: 4
// 1) F5 (2D)  --> Index: 5
// 2) F6 (2D)  --> Index: 6
// 3) F7 (2D)  --> Index: 7
// 4) F8 (2D)  --> Index: 10
// 5) F9 (2D)  --> Index: 11
// 6) F10 (2D) --> Index: 12
// 7) F11 (2D) --> Index: 13
const emp::vector<size_t> PROBLEM_MAP = {4,5,6,7,10,11,12,13};
const emp::vector<std::string> PROBLEM_DESC = {"F4 (2D)","F5 (2D)","F6 (2D)","F7 (2D)","F8 (2D)","F9 (2D)","F10 (2D)","F11 (2D)"};


namespace emp {
  /// ==ROULETTE== Selection (aka Fitness-Proportional Selection) chooses organisms to
  /// reproduce based on their current fitness.
  /// @param world The emp::World object with the organisms to be selected.
  /// @param count How many organims should be selected for replication? (with replacement)
  template<typename ORG, typename DATA_TYPE>
  void NastyRouletteSelect(World<ORG, DATA_TYPE> & world, size_t count, typename World<ORG, DATA_TYPE>::fun_calc_fitness_t & fit_fun) {
    emp_assert(count > 0);

    Random & random = world.GetRandom();

    // Load fitnesses from current population.
    IndexMap fitness_index(world.GetSize());
    for (size_t id = 0; id < world.GetSize(); id++) {
      fitness_index.Adjust(id, fit_fun(world.GetOrg(id)));
      // fitness_index.Adjust(id, world.CalcFitnessID(id));
    }

    for (size_t n = 0; n < count; n++) {
      const double fit_pos = random.GetDouble(fitness_index.GetWeight());
      const size_t parent_id = fitness_index.Index(fit_pos);
      const size_t offspring_id = world.DoBirth( world.GetGenomeAt(parent_id), parent_id ).GetIndex();
      if (world.IsSynchronous() == false) {
        fitness_index.Adjust(offspring_id, fit_fun(world.GetOrg(offspring_id)));
        // fitness_index.Adjust(offspring_id, world.CalcFitnessID(offspring_id));
      }
    }
  }
}

class OptimizationProblemExp {
public:
  using genome_t = emp::vector<double>;


  struct Agent {
    size_t agent_id;
    genome_t genome;

    Agent(const genome_t & gen) : agent_id(0), genome(gen) { ; }
    Agent(const Agent && in) : agent_id(in.agent_id), genome(in.genome) { ; }
    Agent(const Agent & in) : agent_id(in.agent_id), genome(in.genome) { ; }

    size_t GetID() const { return agent_id; }
    void SetID(size_t id) { agent_id = id; }

    void SetGenome(const genome_t & _gen) { genome = _gen; }

    genome_t & GetGenome() { return genome; }

    double x() const { return genome[0]; }
    double y() const { return genome[1]; }

    void Print() {
      std::cout << "ID: " << agent_id << std::endl;
      std::cout << "Genome:";
      for (size_t i = 0; i < genome.size(); ++i) { std::cout << " " << genome[i]; }
      std::cout << std::endl;
    }

  };

  struct Phenotype {
    double score;
    double transformed_score;             ///< Score transformed into form directly usable by selection scheme (not always used).
    emp::vector<double> distances;        ///< Distance to each keypoint.
    emp::vector<double> testcase_scores;
  };

  using phenotype_t = emp::vector<double>;
  using mut_count_t = std::unordered_map<std::string, double>;
  using data_t = emp::mut_landscape_info<phenotype_t>;
  using world_t = emp::World<Agent, data_t>;
  using genotype_t = world_t::genotype_t;

protected:
  size_t RUN_MODE;
  int RANDOM_SEED;
  size_t POP_SIZE;
  size_t GENERATIONS;
  size_t PROBLEM;

  size_t HINT_GRID_RES;

  size_t SELECTION_METHOD;
  size_t ELITE_SELECT__ELITE_CNT;
  size_t TOURNAMENT_SIZE;
  double LEXICASE_EPSILON;
  double RESOURCE_SELECT__RES_AMOUNT;
  double RESOURCE_SELECT__RES_INFLOW;
  double RESOURCE_SELECT__OUTFLOW;
  double RESOURCE_SELECT__FRAC;
  double RESOURCE_SELECT__MAX_BONUS;
  double RESOURCE_SELECT__COST;

  double MUTATION_STD;    ///< Given as ratio of dimension domain.

  size_t SYSTEMATICS_INTERVAL;
  size_t FITNESS_INTERVAL;
  size_t POP_SNAPSHOT_INTERVAL;
  std::string DATA_DIRECTORY;

  emp::Ptr<emp::Random> random;
  emp::Ptr<CEC2013> eval_function;

  size_t update;
  size_t best_agent_id;

  emp::vector<std::function<double(Agent &)>> fit_set;
  emp::vector<emp::Resource> resources;                 ///< Resources for emp::ResourceSelect.
  emp::vector<emp::vector<double>> key_points;
  emp::vector<double> mid_point;
  emp::vector<double> ubounds;
  emp::vector<double> lbounds;
  emp::vector<double> mut_std;
  double max_dist;
  double min_score;

  double score_ceil;
  double score_floor;
  double adjusted_lexicase_epsilon;

  emp::vector<Phenotype> agent_phen_cache;
  mut_count_t last_mutation;

  emp::Ptr<world_t> world;

  // -- Signals! ---
  emp::Signal<void(void)> do_begin_run_setup_sig;   ///< Triggered at begining of run.
  emp::Signal<void(void)> do_pop_init_sig;          ///< Triggered during run setup. Defines way population is initialized.
  emp::Signal<void(void)> do_evaluation_sig;        ///< Triggered during run step. Should trigger population-wide agent evaluation.
  emp::Signal<void(void)> do_selection_sig;         ///< Triggered during run step. Should trigger selection (which includes selection, reproduction, and mutation).
  emp::Signal<void(void)> do_world_update_sig;      ///< Triggered during run step. Should trigger world->Update(), and whatever else should happen right before/after population turnover.
  emp::Signal<void(void)> do_analysis_sig;          ///< Triggered if in analysis mode. Should trigger appropriate analyses.

  // Systematics-specific signals.
  emp::Signal<void(size_t)> do_pop_snapshot_sig;                       ///< Triggered if we should take a snapshot of the population (as defined by POP_SNAPSHOT_INTERVAL). Should call appropriate functions to take snapshot.
  emp::Signal<void(size_t pos, double)> record_fit_sig;                ///< Trigger signal before organism gives birth.
  emp::Signal<void(size_t pos, const phenotype_t &)> record_phen_sig;  ///< Trigger signal before organism gives birth.

  std::function<void(Agent &)> evaluate_agent; ///< Evaluate agent on everything required for selection. Cache results.
  std::function<double(Agent &)> fit_fun;       ///< Fitness function given to world.

  std::function<double(Agent &)> special_roulette_fit_fun;       ///< Fitness function given to world.


  // Elite select mask.
  template<typename WORLD_TYPE>
  void EliteSelect_MASK(WORLD_TYPE & world, size_t e_count=1, size_t copy_count=1) {
    if (copy_count == 0 || e_count == 0) { return; }
    emp::EliteSelect(world, e_count, copy_count);
  }

  double CalcDist(const emp::vector<double> & a, const emp::vector<double> & b) {
    emp_assert(a.size() == b.size());
    emp_assert(a.size() == DIMENSIONS);
    double dist = 0.0;
    for (size_t dim = 0; dim < DIMENSIONS; ++dim) {
      dist += emp::Pow(a[dim] - b[dim], 2);
    }
    return std::sqrt(dist);
  }

public:
  OptimizationProblemExp(const OptimizationConfig & config)   // @constructor
    : min_score(0), score_ceil(0), score_floor(0)
  {
    RUN_MODE = config.RUN_MODE();
    RANDOM_SEED = config.RANDOM_SEED();
    POP_SIZE = config.POP_SIZE();
    GENERATIONS = config.GENERATIONS();
    PROBLEM = config.PROBLEM();

    HINT_GRID_RES = config.HINT_GRID_RES();

    SELECTION_METHOD = config.SELECTION_METHOD();
    ELITE_SELECT__ELITE_CNT = config.ELITE_SELECT__ELITE_CNT();
    LEXICASE_EPSILON = config.LEXICASE_EPSILON();
    TOURNAMENT_SIZE = config.TOURNAMENT_SIZE();
    RESOURCE_SELECT__RES_AMOUNT = config.RESOURCE_SELECT__RES_AMOUNT();
    RESOURCE_SELECT__RES_INFLOW = config.RESOURCE_SELECT__RES_INFLOW();
    RESOURCE_SELECT__OUTFLOW = config.RESOURCE_SELECT__OUTFLOW();
    RESOURCE_SELECT__FRAC = config.RESOURCE_SELECT__FRAC();
    RESOURCE_SELECT__MAX_BONUS = config.RESOURCE_SELECT__MAX_BONUS();
    RESOURCE_SELECT__COST = config.RESOURCE_SELECT__COST();

    MUTATION_STD = config.MUTATION_STD();

    SYSTEMATICS_INTERVAL = config.SYSTEMATICS_INTERVAL();
    FITNESS_INTERVAL = config.FITNESS_INTERVAL();
    POP_SNAPSHOT_INTERVAL = config.POP_SNAPSHOT_INTERVAL();
    DATA_DIRECTORY = config.DATA_DIRECTORY();

    std::cout << "Mutation thing: " << MUTATION_STD << std::endl;

    // for (size_t i = 0; i < PROBLEM_MAP.size(); ++i) {
    //   std::cout << "Problem: " << PROBLEM_MAP[i] << std::endl;
    //   eval_function = emp::NewPtr<CEC2013>(PROBLEM_MAP[i]);
    //   std::cout << "  Dimensions: " << eval_function->get_dimension() << std::endl;
    //   eval_function.Delete();
    // }

    for (size_t i = 0; i < MUTATION_TYPES.size(); ++i) {
      last_mutation.emplace(MUTATION_TYPES[i], 0);
    }

    random = emp::NewPtr<emp::Random>(RANDOM_SEED);

    eval_function = emp::NewPtr<CEC2013>(PROBLEM_MAP[PROBLEM]); // Prepare eval function.

    world = emp::NewPtr<world_t>(random, "OptimizationProblem-World");   // Build the world.
    world->Reset();
    world->SetWellMixed(true);

    agent_phen_cache.resize(POP_SIZE);                          // Resize phenotype cache.

    if (RUN_MODE == RUN_ID__EXP) {
      // Make data directory.
      mkdir(DATA_DIRECTORY.c_str(), ACCESSPERMS);
      if (DATA_DIRECTORY.back() != '/') DATA_DIRECTORY += '/';
    }

    emp::vector<emp::vector<double>> grid_anchors(DIMENSIONS);
    mid_point.resize(DIMENSIONS);
    ubounds.resize(DIMENSIONS);
    lbounds.resize(DIMENSIONS);
    mut_std.resize(DIMENSIONS);
    for (size_t dim = 0; dim < DIMENSIONS; ++dim) {
      grid_anchors[dim].resize(HINT_GRID_RES, 0.0);
      double l = eval_function->get_lbound(dim);
      double u = eval_function->get_ubound(dim);
      double step = (u - l)/(HINT_GRID_RES - 1);
      mid_point[dim] = l + ((u - l)/2);
      ubounds[dim] = u;
      lbounds[dim] = l;
      mut_std[dim] = (u - l)*MUTATION_STD;
      for (size_t i = 0; i < HINT_GRID_RES; ++i) {
        grid_anchors[dim][i] = l + (i*step);
      }
    }
    max_dist = CalcDist(ubounds, lbounds);

    // Fill out key_points. (NOTE: currently not generic for more than 2 dimensions...)
    score_ceil = eval_function->get_fitness_goptima();
    score_floor = score_ceil;
    for (size_t i = 0; i < HINT_GRID_RES; ++i) {
      for (size_t j = 0; j < HINT_GRID_RES; ++j) {
        key_points.emplace_back();
        key_points.back().resize(DIMENSIONS);
        key_points.back()[0] = grid_anchors[0][i];
        key_points.back()[1] = grid_anchors[1][j];
        const double key_point_score = eval_function->evaluate(key_points.back());
        score_floor = (key_point_score < score_floor) ? key_point_score : score_floor;
      }
    }
    adjusted_lexicase_epsilon = (score_ceil - score_floor)*LEXICASE_EPSILON;
    std::cout << "Epsilon: " << adjusted_lexicase_epsilon << std::endl;

    // Configure phenotype cache for key_point distances.
    for (size_t i = 0; i < agent_phen_cache.size(); ++i) {
      Phenotype & phen = agent_phen_cache[i];
      phen.distances.resize(key_points.size());
      phen.testcase_scores.resize(key_points.size());
    }

    // To configure:
    // do_begin_run_setup_sig
    do_begin_run_setup_sig.AddAction([this]() {
      std::cout << "Doing initial run setup." << std::endl;
      world->SetFitFun(fit_fun);
      world->SetMutFun([this](Agent & agent, emp::Random & rnd) { return this->Mutate(agent, rnd); }, ELITE_SELECT__ELITE_CNT);
      // Setup systematics/fitness tracking.
      auto & sys_file = world->SetupSystematicsFile(DATA_DIRECTORY + "systematics.csv");
      sys_file.SetTimingRepeat(SYSTEMATICS_INTERVAL);
      auto & fit_file = world->SetupFitnessFile(DATA_DIRECTORY + "fitness.csv");
      fit_file.SetTimingRepeat(FITNESS_INTERVAL);
      emp::AddPhylodiversityFile(*world, DATA_DIRECTORY + "phylodiversity.csv").SetTimingRepeat(SYSTEMATICS_INTERVAL);
      emp::AddLineageMutationFile(*world, DATA_DIRECTORY + "lineage_mutations.csv", MUTATION_TYPES).SetTimingRepeat(SYSTEMATICS_INTERVAL);
      AddDominantFile(*world, DATA_DIRECTORY + "dominant.csv", MUTATION_TYPES).SetTimingRepeat(SYSTEMATICS_INTERVAL);
      world->OnGenotypeKnown([this](emp::Ptr<genotype_t> genotype, size_t pos) {
        genotype->GetData().RecordMutation(last_mutation);
      });
      record_fit_sig.AddAction([this](size_t pos, double fitness) { world->GetGenotypeAt(pos)->GetData().RecordFitness(fitness); } );
      record_phen_sig.AddAction([this](size_t pos, const phenotype_t & phen) { world->GetGenotypeAt(pos)->GetData().RecordPhenotype(phen); } );
      // Generate the initial population.
      do_pop_init_sig.Trigger();
    });

    // do_evaluation_sig
    do_evaluation_sig.AddAction([this]() {
      double best_score = -32767;
      min_score = 0;
      best_agent_id = 0;
      for (size_t id = 0; id < world->GetSize(); ++id) {
        // std::cout << "Evaluating agent: " << id << std::endl;
        // Evaluate agent given by id.
        Agent & our_hero = world->GetOrg(id);
        our_hero.SetID(id);
        this->evaluate_agent(our_hero);
        Phenotype & phen = agent_phen_cache[id];
        if (id == 0 || phen.score > best_score) {
          best_score = phen.score;
          best_agent_id = id;
        }
        min_score = (id == 0 || phen.score < min_score) ? phen.score : min_score;
      }
      std::cout << "Update: " << update << " Max score: " << best_score << std::endl;
    });

    // do_selection_sig
    switch (SELECTION_METHOD) {
      case SELECTION_METHOD_ID__TOURNAMENT:
        ConfigTournamentSelection();
        break;
      case SELECTION_METHOD_ID__LEXICASE:
        ConfigLexicaseSelection();
        break;
      case SELECTION_METHOD_ID__ECOEA:
        ConfigEcoEASelection();
        break;
      case SELECTION_METHOD_ID__MAPELITES:
        std::cout << "Map elites not implemented! Exiting..." << std::endl;
        exit(-1);
        break;
      case SELECTION_METHOD_ID__ROULETTE:
        ConfigRouletteSelection();
        break;
      case SELECTION_METHOD_ID__DRIFT:
        ConfigDriftSelection();
        break;
      default:
        std::cout << "Unrecognized selection method! Exiting..." << std::endl;
        exit(-1);
    }

    // do_pop_init_sig
    do_pop_init_sig.AddAction([this]() {
      // Init from single (in middle), mutate with crazy high mutation rate, run for single generation.
      Agent ancestor(mid_point);
      world->SetMutFun([this](Agent & agent, emp::Random & r) {
        genome_t & genome = agent.GetGenome();

        double new_x = r.GetDouble(lbounds[0], ubounds[0]);
        double new_y = r.GetDouble(lbounds[1], ubounds[1]);


        last_mutation["normal"] = CalcDist(genome, {new_x, new_y});
        last_mutation["normal_x"] = emp::Abs(genome[0]-new_x);
        last_mutation["normal_y"] = emp::Abs(genome[1]-new_y);

        genome[0] = new_x;
        genome[1] = new_y;

        return 2.0;
      }, 0);
      world->Inject(ancestor.GetGenome(), 1);
      // Run 1 interation of selection/mutation to fill out population.
      do_evaluation_sig.Trigger();
      emp::TournamentSelect(*world, 1, POP_SIZE);
      world->Update();
      // Fix the mutation function.
      world->SetMutFun([this](Agent & agent, emp::Random & rnd) { return this->Mutate(agent, rnd); }, ELITE_SELECT__ELITE_CNT);
    });

    // do_world_update_sig
    do_world_update_sig.AddAction([this]() { world->Update(); });

    // do_analysis_sig (TODO)
    do_analysis_sig.AddAction([this]() {
      std::cout << "Analysis mode not implemented!" << std::endl;
      exit(-1);
    });

    // do_pop_snapshot_sig
    do_pop_snapshot_sig.AddAction([this](size_t update) {
      this->Snapshot(update);
    });

    // record_fit_sig
    // record_phen_sig

    // Test mutation...
    // std::cout << "(" << mid_point[0] << "," << mid_point[1] << ")" << std::endl;
    // for (size_t i = 0; i < 100; ++i) {
    //   Mutate(test, *random);
    //   std::cout << "(" << test.genome[0] << "," << test.genome[1] << "): " << eval_function->evaluate(test.genome) << std::endl;
    // }

    // Print general info about problem.
    // std::cout << "Lower bound: (" << lbounds[0] << "," << lbounds[1] << ")" << std::endl;
    // std::cout << "Upper bound: (" << ubounds[0] << "," << ubounds[1] << ")" << std::endl;
    // std::cout << "Mut stds: " << mut_std[0] << "," << mut_std[1] << std::endl;
    // std::cout << "Mid point: (" << mid_point[0] << "," << mid_point[1] << ")" << std::endl;
    // std::cout << "Max dist: " << max_dist << std::endl;
    // std::cout << "Max score: " << score_ceil << std::endl;
    // std::cout << "Floor score: " << score_floor << std::endl;
    // std::cout << "Grid anchors: \n   ";
    // for (size_t i = 0; i < grid_anchors[0].size(); ++i) {
    //   std::cout << " " << grid_anchors[0][i];
    // } std::cout << std::endl;
    // std::cout << "   ";
    // for (size_t i = 0; i < grid_anchors[1].size(); ++i) {
    //   std::cout << " " << grid_anchors[1][i];
    // } std::cout << std::endl;
    // // Print info about key points.
    // std::cout << "Key points: " << std::endl;
    // for (size_t i = 0; i < key_points.size(); ++i) {
    //   std::cout << "  " << i << ": (" << key_points[i][0] << "," << key_points[i][1] << ")" << std::endl;
    // }
    // // evaluate agent..
    // do_begin_run_setup_sig.Trigger();
    // do_evaluation_sig.Trigger();
    // std::cout << "Printing population information" << std::endl;
    // for (size_t id = 0; id < world->GetSize(); ++id) {
    //   Agent & agent = world->GetOrg(id);
    //   Phenotype & phen = agent_phen_cache[id];
    //   // Print phenotype info about test.
    //   std::cout << "---Phenotype "<<id<<"---" << std::endl;
    //   std::cout << "Genome: (" << agent.genome[0] << "," << agent.genome[1] << ")" << std::endl;
    //   std::cout << "Score: "<< phen.score << std::endl;
    //   std::cout << "Transformed score: " << phen.transformed_score << std::endl;
    //   std::cout << "Distances: " << std::endl;
    //   for (size_t i = 0; i < phen.distances.size(); ++i) {
    //     std::cout << "  (" << key_points[i][0] << "," << key_points[i][1] << ") = " << phen.distances[i];
    //     std::cout << " ==> score = " << phen.testcase_scores[i] << std::endl;
    //   }
    // }
    // exit(-1);
  }

  ~OptimizationProblemExp() {
    random.Delete();
    eval_function.Delete();
    world.Delete();
  }

  void Run() {
    switch (RUN_MODE) {
      case RUN_ID__EXP: {
        std::clock_t base_start_time = std::clock();

        do_begin_run_setup_sig.Trigger();
        for (update = world->GetUpdate(); update <= GENERATIONS; ++update) {
          RunStep();
          if (update % POP_SNAPSHOT_INTERVAL == 0) do_pop_snapshot_sig.Trigger(update);
        }

        std::clock_t base_tot_time = std::clock() - base_start_time;
        std::cout << "Time = " << 1000.0 * ((double) base_tot_time) / (double) CLOCKS_PER_SEC
                  << " ms." << std::endl;
        break;
      }
      case RUN_ID__ANALYSIS: {
        do_analysis_sig.Trigger();
        break;
      }
      default:
        std::cout << "Unrecognized run mode! Exiting..." << std::endl;
        exit(-1);
    }
  }

  /// Do a single step of evolution.
  void RunStep() {
    do_evaluation_sig.Trigger();
    do_selection_sig.Trigger();
    do_world_update_sig.Trigger();
  }

  size_t Mutate(Agent & agent, emp::Random & rnd);

  void Snapshot(size_t u);

  template <typename WORLD_TYPE>
  emp::DataFile & AddDominantFile(WORLD_TYPE & world, const std::string & fpath="dominant.csv", emp::vector<std::string> mut_types = {"substitution"});

  void ConfigTournamentSelection();
  void ConfigLexicaseSelection();
  void ConfigEcoEASelection();
  void ConfigRouletteSelection();
  void ConfigDriftSelection();

};

/// NOTE: assumes (at least) 2 dimensions
size_t OptimizationProblemExp::Mutate(Agent & agent, emp::Random & rnd) {
  genome_t & genome = agent.GetGenome();
  for (size_t i = 0; i < MUTATION_TYPES.size(); ++i) {
    last_mutation[MUTATION_TYPES[i]] = 0;
  }
  genome_t new_vals(genome.size());
  for (size_t d = 0; d < genome.size(); ++d) {
    double new_val = rnd.GetRandNormal(genome[d], mut_std[d]);
    // Clamp proposed mutation value.
    new_val = (new_val > ubounds[d]) ? ubounds[d] : new_val;
    new_val = (new_val < lbounds[d]) ? lbounds[d] : new_val;
    new_vals[d] = new_val;
  }
  // Record last mutation info.
  last_mutation["normal"] = CalcDist(genome, new_vals);
  last_mutation["normal_x"] = emp::Abs(genome[0]-new_vals[0]);
  last_mutation["normal_y"] = emp::Abs(genome[1]-new_vals[1]);
  // Update genome with new values
  for (size_t d = 0; d < genome.size(); ++d) { genome[d] = new_vals[d]; }
  return genome.size();
}

void OptimizationProblemExp::Snapshot(size_t u) {
  std::string snapshot_dir = DATA_DIRECTORY + "pop_" + emp::to_string((int)update);
  mkdir(snapshot_dir.c_str(), ACCESSPERMS);

  // Print phylogeny.
  std::ofstream phylo_stream(snapshot_dir + "/phylogeny_" + emp::to_string((int)update) + ".csv");
  world->GetSystematics().PrintPhylogeny(phylo_stream);
  phylo_stream.close();

  // For each program in the population, dump the full program description in a single file.
  std::ofstream pop_ofstream(snapshot_dir + "/pop_" + emp::to_string((int)update) + ".pop");
  for (size_t i = 0; i < world->GetSize(); ++i) {
    Agent & agent = world->GetOrg(i);
    genome_t & genome = agent.GetGenome();
    pop_ofstream << "(";
    for (size_t d = 0; d < genome.size(); ++d) {
      if (d) pop_ofstream << ",";
      pop_ofstream << genome[d];
    }
    pop_ofstream << ")\n";
  }
  pop_ofstream.close();
}

template <typename WORLD_TYPE>
emp::DataFile & OptimizationProblemExp::AddDominantFile(WORLD_TYPE & world, const std::string & fpath, emp::vector<std::string> mut_types) {
  auto & file = world.SetupFile(fpath);

  std::function<size_t(void)> get_update = [&world](){return world.GetUpdate();};
  file.AddFun(get_update, "update", "Update");

  std::function<double(void)> get_score = [&world, this]() {
    Phenotype & best_phen = this->agent_phen_cache[this->best_agent_id];
    return best_phen.score;
  };
  file.AddFun(get_score, "score", "get best phenotype score from this update");

  for (size_t i = 0; i < mut_types.size(); ++i) {
    std::string mut_type = mut_types[i];
    std::function<int(void)> mut_fun = [&world, this, mut_type]() { return emp::CountMuts(world.GetGenotypeAt(this->best_agent_id), mut_type); };
    file.AddFun(mut_fun, "dominant_"+mut_type+"_mutation_magnitude", "magnitude of "+mut_type+" mutations along dominant organism's lineage");
  }

  std::function<int(void)> dom_lin_len = [&world, this](){
    return emp::LineageLength(world.GetGenotypeAt(this->best_agent_id));
  };
  std::function<int(void)> dom_lin_len_norm = [&world, this](){
    return emp::CountMutSteps(world.GetGenotypeAt(this->best_agent_id), "normal");
  };
  std::function<int(void)> dom_lin_len_normx = [&world, this](){
    return emp::CountMutSteps(world.GetGenotypeAt(this->best_agent_id), "normal_x");
  };
  std::function<int(void)> dom_lin_len_normy = [&world, this](){
    return emp::CountMutSteps(world.GetGenotypeAt(this->best_agent_id), "normal_y");
  };
  std::function<int(void)> dom_del_step = [&world, this](){
    return emp::CountDeleteriousSteps(world.GetGenotypeAt(this->best_agent_id));
  };
  std::function<size_t(void)> dom_phen_vol = [&world, this](){
    return emp::CountPhenotypeChanges(world.GetGenotypeAt(this->best_agent_id));
  };
  std::function<size_t(void)> dom_unique_phen = [&world, this](){
    return emp::CountUniquePhenotypes(world.GetGenotypeAt(this->best_agent_id));
  };

  // file.AddFun(dom_mut_count, "dominant_mutation_count", "sum of mutations along dominant organism's lineage");
  file.AddFun(dom_lin_len, "dominant_lineage_length", "count of changes in genotype in the dominant organism's lineage.");
  file.AddFun(dom_lin_len_norm, "dominant_lineage_length_by_normal", "count of changes in genotype due to instruction substitutions in the dominant organism's lineage.");
  file.AddFun(dom_lin_len_normx, "dominant_lineage_length_by_normal_x", "count of changes in genotype in the dominant organism's lineage.");
  file.AddFun(dom_lin_len_normy, "dominant_lineage_length_by_normal_y", "count of changes in genotype due to all non-tag (bit flip) mutations.");
  file.AddFun(dom_del_step, "dominant_deleterious_steps", "count of deleterious steps along dominant organism's lineage");
  file.AddFun(dom_phen_vol, "dominant_phenotypic_volatility", "count of changes in phenotype along dominant organism's lineage");
  file.AddFun(dom_unique_phen, "dominant_unique_phenotypes", "count of unique phenotypes along dominant organism's lineage");
  file.PrintHeaderKeys();
  return file;

}


void OptimizationProblemExp::ConfigTournamentSelection() {
  // 1) Fill out evaluate function.
  evaluate_agent = [this](Agent & agent) {
    const size_t id = agent.GetID();
    Phenotype & phen = agent_phen_cache[id];
    phen.score = eval_function->evaluate(agent.genome);
    record_phen_sig.Trigger(id, {phen.score});
    record_fit_sig.Trigger(id, phen.score);
  };
  // 2) Fill out fit fun.
  fit_fun = [this](Agent & agent) {
    const size_t id = agent.GetID();
    return agent_phen_cache[id].score;
  };
  // 3) Setup do_selection_sig
  do_selection_sig.AddAction([this]() {
    this->EliteSelect_MASK(*world, ELITE_SELECT__ELITE_CNT, 1);
    emp::TournamentSelect(*world, TOURNAMENT_SIZE, POP_SIZE - ELITE_SELECT__ELITE_CNT);
  });
}

void OptimizationProblemExp::ConfigLexicaseSelection() {
  // 1) Fill out evaluate function.
  evaluate_agent = [this](Agent & agent) {
    const size_t id = agent.GetID();
    Phenotype & phen = agent_phen_cache[id];
    phen.score = eval_function->evaluate(agent.genome);
    phen.transformed_score = (phen.score-score_floor)/(score_ceil-score_floor);
    // Evaluate distances.
    for (size_t i = 0; i < key_points.size(); ++i) {
      phen.distances[i] = CalcDist(key_points[i], agent.genome); // Don't want to end up with divide-by-zero error.
      // phen.testcase_scores[i] = (phen.score < 0) ? (1+phen.distances[i]/max_dist)*phen.score : (1 - phen.distances[i]/max_dist)*phen.score;
      phen.testcase_scores[i] = (1 - (phen.distances[i]/max_dist)) * phen.transformed_score;
    }
    record_phen_sig.Trigger(id, {phen.score});
    record_fit_sig.Trigger(id, phen.score);
  };
  // 2) Fill out fit fun.
  fit_fun = [this](Agent & agent) {
    const size_t id = agent.GetID();
    return agent_phen_cache[id].score;
  };
  // 2.5) Fill out fit set.
  for (size_t i = 0; i < key_points.size(); ++i) {
    fit_set.push_back([this, i](Agent & agent) {
      const size_t id = agent.GetID();
      const Phenotype & phen = agent_phen_cache[id];
      return phen.testcase_scores[i];
    });
  }
  fit_set.push_back([this](Agent & agent) {
    const size_t id = agent.GetID();
    const Phenotype & phen = agent_phen_cache[id];
    return phen.score;
  });
  // 3) Setup do_selection_sig
  do_selection_sig.AddAction([this]() {
    this->EliteSelect_MASK(*world, ELITE_SELECT__ELITE_CNT, 1);
    emp::LexicaseSelect(*world, fit_set, POP_SIZE - ELITE_SELECT__ELITE_CNT, 0, adjusted_lexicase_epsilon);
  });
}

void OptimizationProblemExp::ConfigEcoEASelection() {
  // 1) Fill out evaluate function.
  evaluate_agent = [this](Agent & agent) {
    const size_t id = agent.GetID();
    Phenotype & phen = agent_phen_cache[id];
    phen.score = eval_function->evaluate(agent.genome);
    phen.transformed_score = (phen.score-score_floor)/(score_ceil-score_floor);
    // Evaluate distances.
    // std::cout << "Eval agent" << std::endl;
    // std::cout << "  score floor: " << score_floor << std::endl;
    // std::cout << "  score ceil: " << score_ceil << std::endl;
    // std::cout << "  transformed score: " << phen.transformed_score << std::endl;
    // std::cout << "  score: " << phen.score << std::endl;
    for (size_t i = 0; i < key_points.size(); ++i) {
      phen.distances[i] = CalcDist(key_points[i], agent.genome); // Don't want to end up with divide-by-zero error.
      // phen.testcase_scores[i] = (phen.score < 0) ? (1+phen.distances[i]/max_dist)*phen.score : (1 - phen.distances[i]/max_dist)*phen.score;
      phen.testcase_scores[i] = (1 - (phen.distances[i]/max_dist)) * phen.transformed_score;
    }
    record_phen_sig.Trigger(id, {phen.score});
    record_fit_sig.Trigger(id, phen.score);
  };
  // 2) Fill out fit fun.
  fit_fun = [this](Agent & agent) {
    const size_t id = agent.GetID();
    return agent_phen_cache[id].score;
  };
  // 2.5) Fill out fit set & resources.
  for (size_t i = 0; i < key_points.size(); ++i) {
    fit_set.push_back([this, i](Agent & agent) {
      const size_t id = agent.GetID();
      const Phenotype & phen = agent_phen_cache[id];
      return phen.testcase_scores[i];
    });
    resources.emplace_back(RESOURCE_SELECT__RES_AMOUNT, RESOURCE_SELECT__RES_INFLOW, RESOURCE_SELECT__OUTFLOW);
  }
  fit_set.push_back([this](Agent & agent) {
    const size_t id = agent.GetID();
    const Phenotype & phen = agent_phen_cache[id];
    return phen.transformed_score;
  });
  resources.emplace_back(RESOURCE_SELECT__RES_AMOUNT, RESOURCE_SELECT__RES_INFLOW, RESOURCE_SELECT__OUTFLOW);
  // 3) Setup do_selection_sig
  do_selection_sig.AddAction([this]() {
    this->EliteSelect_MASK(*world, ELITE_SELECT__ELITE_CNT, 1);
    emp::ResourceSelect(*world, fit_set, resources,
                        TOURNAMENT_SIZE, POP_SIZE - ELITE_SELECT__ELITE_CNT, RESOURCE_SELECT__FRAC,
                        RESOURCE_SELECT__MAX_BONUS, RESOURCE_SELECT__COST, false);
  });
}

void OptimizationProblemExp::ConfigRouletteSelection() {
  // Add another action to do_evaluation_sig ==> Transform scores if necessary.
  do_evaluation_sig.AddAction([this]() {
    // Need to shift everything if min_score <= 0
    if (min_score < 0) {
      // std::cout << "Min score ("<<min_score<<") less than 0!" << std::endl;
      const double shift = 1 - min_score;
      for (size_t i = 0; i < agent_phen_cache.size(); ++i) {
        Phenotype & phen = agent_phen_cache[i];
        phen.transformed_score = phen.score + shift;
      }
    }
  });
  // 1) Fill out evaluate function.
  evaluate_agent = [this](Agent & agent) {
    const size_t id = agent.GetID();
    Phenotype & phen = agent_phen_cache[id];
    phen.score = eval_function->evaluate(agent.genome);
    record_phen_sig.Trigger(id, {phen.score});
    record_fit_sig.Trigger(id, phen.score);
  };

  // 2) Fill out fit fun.
  fit_fun = [this](Agent & agent) {
    const size_t id = agent.GetID();
    return agent_phen_cache[id].score;
  };
  special_roulette_fit_fun = [this](Agent & agent) {
    const size_t id = agent.GetID();
    return agent_phen_cache[id].transformed_score;
  };
  // 3) Setup do_selection_sig -- TODO, fix rs
  do_selection_sig.AddAction([this]() {
    this->EliteSelect_MASK(*world, ELITE_SELECT__ELITE_CNT, 1);
    emp::NastyRouletteSelect(*world, POP_SIZE - ELITE_SELECT__ELITE_CNT, special_roulette_fit_fun);
  });
}

void OptimizationProblemExp::ConfigDriftSelection() {
  // 1) Fill out evaluate function.
  evaluate_agent = [this](Agent & agent) {
    const size_t id = agent.GetID();
    Phenotype & phen = agent_phen_cache[id];
    phen.score = eval_function->evaluate(agent.genome);
    record_phen_sig.Trigger(id, {phen.score});
    record_fit_sig.Trigger(id, phen.score);
  };

  // 2) Fill out fit fun.
  fit_fun = [this](Agent &agent) {
    const size_t id = agent.GetID();
    return agent_phen_cache[id].score;
  };

  // 3) Setup do_selection_sig
  do_selection_sig.AddAction([this]() { emp::TournamentSelect(*world, 1, POP_SIZE); });

}

#endif
