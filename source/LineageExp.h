#ifndef LINEAGE_EXP_H
#define LINEAGE_EXP_H

// @includes
#include <iostream>
#include <string>
#include <utility>
#include <fstream>
#include <sys/stat.h>
#include <algorithm>
#include <functional>

#include "base/Ptr.h"
#include "base/vector.h"
#include "control/Signal.h"
#include "Evolve/World.h"
#include "Evolve/Resource.h"
#include "Evolve/SystematicsAnalysis.h"
#include "Evolve/World_output.h"
#include "games/Othello8.h"
#include "hardware/EventDrivenGP.h"
#include "hardware/AvidaGP.h"
#include "hardware/AvidaCPU_InstLib.h"
#include "hardware/InstLib.h"
#include "tools/BitVector.h"
#include "tools/Random.h"
#include "tools/random_utils.h"
#include "tools/math.h"
#include "tools/string_utils.h"

#include "TestcaseSet.h"
#include "OthelloHW.h"
#include "lineage-config.h"

// @constants
constexpr int TESTCASE_FILE__DARK_ID = 1;
constexpr int TESTCASE_FILE__LIGHT_ID = -1;
constexpr int TESTCASE_FILE__OPEN_ID = 0;

constexpr size_t SGP__TAG_WIDTH = 16;

constexpr size_t REPRESENTATION_ID__AVIDAGP = 0;
constexpr size_t REPRESENTATION_ID__SIGNALGP = 1;

constexpr size_t TRAIT_ID__MOVE = 0;
constexpr size_t TRAIT_ID__DONE = 1;

constexpr size_t RUN_ID__EXP = 0;
constexpr size_t RUN_ID__ANALYSIS = 1;

constexpr size_t ANALYSIS_TYPE_ID__DEBUGGING = 0;

constexpr int AGENT_VIEW__ILLEGAL_ID = -1;
constexpr int AGENT_VIEW__OPEN_ID = 0;
constexpr int AGENT_VIEW__SELF_ID = 1;
constexpr int AGENT_VIEW__OPP_ID = 2;

constexpr size_t SELECTION_METHOD_ID__TOURNAMENT = 0;
constexpr size_t SELECTION_METHOD_ID__LEXICASE = 1;
constexpr size_t SELECTION_METHOD_ID__ECOEA = 2;
constexpr size_t SELECTION_METHOD_ID__MAPELITES = 3;
constexpr size_t SELECTION_METHOD_ID__ROULETTE = 4;

constexpr size_t RESOURCE_SELECT_MODE_ID__PHASES = 0;   ///< Each resource is a collection of testcase that share a range of game rounds.
constexpr size_t RESOURCE_SELECT_MODE_ID__INDIV = 1;    ///< Each test case is a resource.

constexpr size_t POP_INITIALIZATION_METHOD_ID__ANCESTOR_FILE = 0;
constexpr size_t POP_INITIALIZATION_METHOD_ID__RANDOM_POP = 1;

constexpr size_t OTHELLO_BOARD_WIDTH = 8;
constexpr size_t OTHELLO_BOARD_NUM_CELLS = OTHELLO_BOARD_WIDTH * OTHELLO_BOARD_WIDTH;

const emp::vector<std::string> MUTATION_TYPES = {"inst_substitutions", "arg_substitutions", "tag_bit_flips", "inst_insertions", "inst_deletions", "func_duplications", "func_deletions"};

/// Setup a data_file with world that records information about the dominant genotype.
template <typename WORLD_TYPE>
emp::DataFile & AddDominantFile(WORLD_TYPE & world, const std::string & fpath="dominant.csv", emp::vector<std::string> mut_types = {"substitution"}){
    auto & file = world.SetupFile(fpath);

    std::function<size_t(void)> get_update = [&world](){return world.GetUpdate();};
    file.AddFun(get_update, "update", "Update");

    // Add file field for each mutation type.
    for (size_t i = 0; i < mut_types.size(); ++i) {
      std::string mut_type = mut_types[i];
      std::function<int(void)> mut_fun = [&world, mut_type]() { return emp::CountMuts(world.GetGenotypeAt(0), mut_type); };
      file.AddFun(mut_fun, "dominant_"+mut_type+"_mutation_count", "sum of "+mut_type+" mutations along dominant organism's lineage");
    }

    std::function<int(void)> dom_lin_len = [&world](){
      return emp::LineageLength(world.GetGenotypeAt(0));
    };
    std::function<int(void)> dom_lin_len_inst_sub = [&world](){
      return emp::CountMutSteps(world.GetGenotypeAt(0), "inst_substitutions");
    };
    std::function<int(void)> dom_lin_len_all_sub = [&world](){
      return emp::CountMutSteps(world.GetGenotypeAt(0), emp::vector<std::string>({"inst_substitutions", "arg_substitutions"}));
    };

    std::function<int(void)> dom_lin_len_all_non_bit = [&world](){
      return emp::CountMutSteps(world.GetGenotypeAt(0), emp::vector<std::string>({"inst_substitutions", "arg_substitutions", "inst_insertions", "inst_deletions", "func_duplications", "func_deletions"}));
    };
    std::function<int(void)> dom_lin_len_all_non_bit_non_arg = [&world](){
      return emp::CountMutSteps(world.GetGenotypeAt(0), emp::vector<std::string>({"inst_substitutions", "inst_insertions", "inst_deletions", "func_duplications", "func_deletions"}));
    };

    std::function<int(void)> dom_del_step = [&world](){
      return emp::CountDeleteriousSteps(world.GetGenotypeAt(0));
    };
    std::function<size_t(void)> dom_phen_vol = [&world](){
      return emp::CountPhenotypeChanges(world.GetGenotypeAt(0));
    };
    std::function<size_t(void)> dom_unique_phen = [&world](){
      return emp::CountUniquePhenotypes(world.GetGenotypeAt(0));
    };

    // file.AddFun(dom_mut_count, "dominant_mutation_count", "sum of mutations along dominant organism's lineage");
    file.AddFun(dom_lin_len, "dominant_lineage_length", "count of changes in genotype in the dominant organism's lineage.");
    file.AddFun(dom_lin_len_inst_sub, "dominant_lineage_length_by_inst_substitution", "count of changes in genotype due to instruction substitutions in the dominant organism's lineage.");
    file.AddFun(dom_lin_len_all_sub, "dominant_lineage_length_by_all_substitutions", "count of changes in genotype in the dominant organism's lineage.");
    file.AddFun(dom_lin_len_all_non_bit, "dominant_lineage_length_by_all_non_bit_muts", "count of changes in genotype due to all non-tag (bit flip) mutations.");
    file.AddFun(dom_lin_len_all_non_bit_non_arg, "dominant_lineage_length_by_all_non_bit_non_arg_muts", "count of changes in genotype due to all non-tag (bit flip) and non-argument mutations.");
    file.AddFun(dom_del_step, "dominant_deleterious_steps", "count of deleterious steps along dominant organism's lineage");
    file.AddFun(dom_phen_vol, "dominant_phenotypic_volatility", "count of changes in phenotype along dominant organism's lineage");
    file.AddFun(dom_unique_phen, "dominant_unique_phenotypes", "count of unique phenotypes along dominant organism's lineage");
    file.PrintHeaderKeys();
    return file;
}

class LineageExp {
public:
  // @aliases
  using othello_t = emp::Othello8;
  using player_t = othello_t::Player;
  using facing_t = othello_t::Facing;
  using othello_idx_t = othello_t::Index;
  // SignalGP-specific type aliases:
  using SGP__hardware_t = emp::EventDrivenGP_AW<SGP__TAG_WIDTH>;
  using SGP__program_t = SGP__hardware_t::Program;
  using SGP__state_t = SGP__hardware_t::State;
  using SGP__inst_t = SGP__hardware_t::inst_t;
  using SGP__inst_lib_t = SGP__hardware_t::inst_lib_t;
  using SGP__event_t = SGP__hardware_t::event_t;
  using SGP__event_lib_t = SGP__hardware_t::event_lib_t;
  using SGP__memory_t = SGP__hardware_t::memory_t;
  using SGP__tag_t = SGP__hardware_t::affinity_t;

  // AvidaGP-specific type aliases:
  using AGP__hardware_t = emp::AvidaGP;
  using AGP__program_t = AGP__hardware_t::genome_t;
  using AGP__inst_t = AGP__hardware_t::inst_t;
  using AGP__inst_lib_t = AGP__hardware_t::inst_lib_t;

  struct Agent {
    size_t agent_id;
    size_t GetID() const { return agent_id; }
    void SetID(size_t id) { agent_id = id; }
  };

  /// SGP Target of evolution:
  struct SignalGPAgent : Agent {
    using Agent::agent_id;
    SGP__program_t program;

    SignalGPAgent(const SGP__program_t & _p)
      : program(_p)
    { ; }

    SignalGPAgent(const SignalGPAgent && in)
      : Agent(in), program(in.program)
    { ; }

    SignalGPAgent(const SignalGPAgent & in)
      : Agent(in), program(in.program)
    { ; }

    SGP__program_t & GetGenome() { return program; }

  };

  /// AGP target of evolution:
  struct AvidaGPAgent : Agent {
    using Agent::agent_id;
    AGP__program_t program;

    AvidaGPAgent(const AGP__program_t & _p)
    : Agent(), program(_p)
    { ; }

    AvidaGPAgent(const AvidaGPAgent && in)
      : Agent(in), program(in.program)
    { ; }

    AvidaGPAgent(const AvidaGPAgent & in)
      : Agent(in), program(in.program)
    { ; }

    AGP__program_t & GetGenome() { return program; }
  };

  struct TestcaseInput {
    othello_t game;  ///< Othello game board.
    player_t playerID;    ///< From what player is the testcase move made?
    size_t round;       ///< What round is this testcase?

    TestcaseInput(player_t pID=player_t::DARK) : playerID(pID) { ; }
    TestcaseInput(const TestcaseInput &) = default;
    TestcaseInput(TestcaseInput &&) = default;
  };

  struct TestcaseOutput {
    othello_idx_t expert_move;             ///< What move did the expert make on the associated testcase?
    emp::vector<size_t> move_valid; ///< Is a given move valid (1) or not (0)?
  };

  struct Phenotype {
    emp::vector<double> testcase_scores;
    size_t illegal_move_total;
    size_t valid_move_total;
    size_t expert_move_total;
    double aggregate_score;
  };

  // More aliases
  using phenotype_t = emp::vector<double>;
  using data_t = emp::mut_landscape_info<phenotype_t>;
  using mut_count_t = std::unordered_map<std::string, int>;
  using SGP__world_t = emp::World<SignalGPAgent, data_t>;
  using AGP__world_t = emp::World<AvidaGPAgent, data_t>;
  using SGP__genotype_t = SGP__world_t::genotype_t;
  using AGP__genotype_t = AGP__world_t::genotype_t;

protected:
  // == Configurable experiment parameters ==
  // @config_declarations
  // General parameters
  size_t RUN_MODE;
  int RANDOM_SEED;
  size_t POP_SIZE;
  size_t GENERATIONS;
  size_t EVAL_TIME;
  size_t REPRESENTATION;
  std::string TEST_CASE_FILE;
  std::string ANCESTOR_FPATH;
  size_t POP_INITIALIZATION_METHOD;
  // Selection Group parameters
  size_t SELECTION_METHOD;
  size_t ELITE_SELECT__ELITE_CNT;
  size_t TOURNAMENT_SIZE;
  size_t RESOURCE_SELECT__MODE;
  double RESOURCE_SELECT__RES_AMOUNT;
  double RESOURCE_SELECT__RES_INFLOW;
  double RESOURCE_SELECT__OUTFLOW;
  double RESOURCE_SELECT__FRAC;
  double RESOURCE_SELECT__MAX_BONUS;
  double RESOURCE_SELECT__COST;
  size_t RESOURCE_SELECT__GAME_PHASE_LEN;
  // Scoring Group parameters
  double SCORE_MOVE__ILLEGAL_MOVE_VALUE;
  double SCORE_MOVE__LEGAL_MOVE_VALUE;
  double SCORE_MOVE__EXPERT_MOVE_VALUE;
  // Othello Group parameters
  size_t OTHELLO_HW_BOARDS;
  // SignalGP program group parameters
  size_t SGP_FUNCTION_LEN;
  size_t SGP_FUNCTION_CNT;
  size_t SGP_PROG_MAX_LENGTH;
  // SignalGP Hardware Group parameters
  size_t SGP_HW_MAX_CORES;
  size_t SGP_HW_MAX_CALL_DEPTH;
  double SGP_HW_MIN_BIND_THRESH;
  // SignalGP Mutation Group parameters
  int SGP_PROG_MAX_ARG_VAL;
  double SGP_PER_BIT__TAG_BFLIP_RATE;
  double SGP_PER_INST__SUB_RATE;
  bool SGP_VARIABLE_LENGTH;
  double SGP_PER_INST__INS_RATE;
  double SGP_PER_INST__DEL_RATE;
  double SGP_PER_FUNC__FUNC_DUP_RATE;
  double SGP_PER_FUNC__FUNC_DEL_RATE;
  // AvidaGP Program group parameters
  size_t AGP_GENOME_SIZE;
  // AvidaGP Mutation Group parameters
  double AGP_PER_INST__SUB_RATE;
  // Data Collection parameters
  size_t SYSTEMATICS_INTERVAL;
  size_t FITNESS_INTERVAL;
  size_t POP_SNAPSHOT_INTERVAL;
  std::string DATA_DIRECTORY;
  // Analysis parameters
  size_t ANALYSIS_TYPE;
  std::string ANALYZE_PROGRAM_FPATH;

  // Experiment variables.
  emp::Ptr<emp::Random> random;

  size_t update;                ///< Current update/generation.
  size_t eval_time;             ///< Current evaluation time point (within an agent's turn).
  size_t OTHELLO_MAX_ROUND_CNT; ///< What are the maximum number of rounds in game?
  size_t best_agent_id;

  // Testcases
  TestcaseSet<TestcaseInput,TestcaseOutput> testcases; ///< Test cases are OthelloBoard ==> Expert move
  using test_case_t = typename TestcaseSet<TestcaseInput, TestcaseOutput>::test_case_t;
  size_t cur_testcase;  ///< What's the current test case that *someone* is solving?
  // Fitness function sets.
  emp::vector<std::function<double(SignalGPAgent &)>> sgp_lexicase_fit_set; ///< Fit set for SGP lexicase selection.
  emp::vector<std::function<double(AvidaGPAgent &)>> agp_lexicase_fit_set;  ///< Fit set for AGP lexicase selection.
  emp::vector<std::function<double(SignalGPAgent &)>> sgp_resource_fit_set; ///< Fit set for SGP resource selection.
  emp::vector<std::function<double(AvidaGPAgent &)>> agp_resource_fit_set;  ///< Fit set for AGP resource selection.

  emp::vector<Phenotype> agent_phen_cache;

  mut_count_t last_mutation;

  emp::vector<emp::vector<size_t>> testcases_by_phase;  ///< Testcase IDs organized by game phase (the length of which is defined by RESOURCE_SELECT__GAME_PHASE_LEN)
  emp::vector<emp::Resource> resources;                 ///< Resources for emp::ResourceSelect. One for each game phase.

  // emp::CollectionDataFile<std::unordered_set<emp::Ptr<SGP__genotype_t>, typename emp::Ptr<SGP__genotype_t>::hash_t>*> sgp_muller_file;
  // emp::CollectionDataFile<std::unordered_set<emp::Ptr<AGP__genotype_t>, typename emp::Ptr<AGP__genotype_t>::hash_t>*> agp_muller_file;

  emp::Ptr<OthelloHardware> othello_dreamware; ///< Othello game board dreamware!

  // SignalGP-specifics.
  emp::Ptr<SGP__world_t> sgp_world;         ///< World for evolving SignalGP agents.
  emp::Ptr<SGP__inst_lib_t> sgp_inst_lib;   ///< SignalGP instruction library.
  emp::Ptr<SGP__event_lib_t> sgp_event_lib; ///< SignalGP event library.
  emp::Ptr<SGP__hardware_t> sgp_eval_hw;    ///< Hardware used to evaluate SignalGP programs during evolution/analysis.

  // AvidaGP-specifics.
  emp::Ptr<AGP__world_t> agp_world;         ///< World for evolving AvidaGP agents.
  emp::Ptr<AGP__inst_lib_t> agp_inst_lib;   ///< AvidaGP instruction library.
  emp::Ptr<AGP__hardware_t> agp_eval_hw;    ///< Hardware used to evaluate AvidaGP programs during evolution/analysis.

  // --- Signals and functors! ---
  // Many of these are hardware-specific.
  // Experiment running/setup signals.
  emp::Signal<void(void)> do_begin_run_setup_sig;   ///< Triggered at begining of run. Shared between AGP and SGP
  emp::Signal<void(void)> do_pop_init_sig;          ///< Triggered during run setup. Defines way population is initialized.
  emp::Signal<void(void)> do_evaluation_sig;        ///< Triggered during run step. Should trigger population-wide agent evaluation.
  emp::Signal<void(void)> do_selection_sig;         ///< Triggered during run step. Should trigger selection (which includes selection, reproduction, and mutation).
  emp::Signal<void(void)> do_world_update_sig;      ///< Triggered during run step. Should trigger world->Update(), and whatever else should happen right before/after population turnover.
  emp::Signal<void(void)> do_analysis_sig;          ///< Triggered if in analysis mode. Should trigger appropriate analyses.
  // Systematics-specific signals.
  emp::Signal<void(size_t)> do_pop_snapshot_sig;    ///< Triggered if we should take a snapshot of the population (as defined by POP_SNAPSHOT_INTERVAL). Should call appropriate functions to take snapshot.
  emp::Signal<void(size_t pos, double)> record_fit_sig;        ///< Trigger signal before organism gives birth.
  emp::Signal<void(size_t pos, const phenotype_t &)> record_phen_sig;  ///< Trigger signal before organism gives birth.
  // Agent evaluation signals.
  emp::Signal<void(const othello_t &)> begin_turn_sig; ///< Called at beginning of agent turn during evaluation.
  emp::Signal<void(void)> agent_advance_sig;              ///< Called during agent's turn. Should cause agent to advance by a single timestep.

  std::function<size_t(void)> get_eval_agent_move;              ///< Should return eval_hardware's current move selection. Hardware-specific!
  std::function<bool(void)> get_eval_agent_done;                ///< Should return whether or not eval_hardware is done. Hardware-specific!
  std::function<player_t(void)> get_eval_agent_playerID;          ///< Should return eval_hardware's current playerID. Hardware-specific!
  std::function<double(test_case_t &, othello_idx_t)> calc_test_score; ///< Given a test case and a move, what is the appropriate score? Shared between hardware types.

  /// Get othello board index given *any* position.
  /// If position can't be used to make an Othello::Index struct, clamp it so that it can.
  othello_idx_t GetOthelloIndex(size_t pos) {
    return (pos > OTHELLO_BOARD_NUM_CELLS) ? OTHELLO_BOARD_NUM_CELLS : pos;
  }
  /// Evaluate GP move (hardware-agnostic).
  /// Requires that the following signals/functors be setup:
  ///  - begin_turn_sig
  ///  - agent_advance_sig
  ///  - get_eval_agent_done
  ///  - get_eval_agent_move
  ///  - get_eval_agent_playerID
  othello_idx_t EvalMove__GP(othello_t & game, bool promise_validity=false) {
    // Signal begin_turn
    begin_turn_sig.Trigger(game);
    // Run agent until time is up or until agent indicates it is done evaluating.
    for (eval_time = 0; eval_time < EVAL_TIME && !get_eval_agent_done(); ++eval_time) {
      agent_advance_sig.Trigger();
    }
    // Extract agent's move.
    othello_idx_t move = GetOthelloIndex(get_eval_agent_move());
    // Did we promise a valid move?
    if (promise_validity) {
      // Double-check move validity.
      const player_t playerID = get_eval_agent_playerID();
      if (!game.IsValidMove(playerID, move)) {
        // Move is not valid. Needs to be fixed, so set it to the nearest valid move.
        emp::vector<othello_idx_t> valid_moves = game.GetMoveOptions(playerID);
        const size_t move_x = move.x();
        const size_t move_y = move.y();
        size_t new_move_x = 0;
        size_t new_move_y = 0;
        size_t sq_move_dist = game.GetNumCells() * game.GetNumCells();
        for (size_t i = 0; i < valid_moves.size(); ++i) {
          const size_t proposed_x = valid_moves[i].x();
          const size_t proposed_y = valid_moves[i].y();
          const size_t proposed_dist = std::pow(proposed_x - move_x, 2) + std::pow(proposed_y - move_y, 2);
          if (proposed_dist < sq_move_dist) {
            new_move_x = proposed_x; new_move_y = proposed_y; sq_move_dist = proposed_dist;
          }
        }
        move.Set(new_move_x, new_move_y);
      }
    }
    return move;
  }

  /// Returns a random valid move.
  othello_idx_t EvalMove__Random(othello_t & game, player_t playerID, bool promise_validity=false) {
    emp::vector<othello_idx_t> options = game.GetMoveOptions(playerID);
    return options[random->GetUInt(0, options.size())];
  }

  /// From vector of strings pulled from a single line of a testcases file, generate a single
  /// test case.
  /// Expected line contents: game board positions, player ID, expert move, round
  test_case_t GenerateTestcase(emp::vector<std::string> & test_case_strings) {
    // Build test case input.
    TestcaseInput input;
    othello_t & game = input.game;
    // test_case_strings expectation: game_board_positions, round, playerID, expert_move
    emp_assert(test_case_strings.size() == (game.GetNumCells() + 3));

    // Get the round.
    size_t rnd = std::atoi(test_case_strings.back().c_str());
    test_case_strings.resize(test_case_strings.size() - 1);

    // Get the expert move.
    size_t expert_move = std::atoi(test_case_strings.back().c_str());
    test_case_strings.resize(test_case_strings.size() - 1);

    // Get the playerID.
    int id = std::atoi(test_case_strings.back().c_str());
    player_t playerID = (id == TESTCASE_FILE__DARK_ID) ? player_t::DARK : player_t::LIGHT;
    test_case_strings.resize(test_case_strings.size() - 1);

    for (size_t i = 0; i < test_case_strings.size(); ++i) {
      int board_space = std::atoi(test_case_strings[i].c_str());
      switch (board_space) {
        case TESTCASE_FILE__DARK_ID:
          game.SetPos(i, player_t::DARK);
          break;
        case TESTCASE_FILE__LIGHT_ID:
          game.SetPos(i, player_t::LIGHT);
          break;
        case TESTCASE_FILE__OPEN_ID:
          game.SetPos(i, player_t::NONE);
          break;
        default:
          std::cout << "Unrecognized board tile! Exiting..." << std::endl;
          exit(-1);
      }
    }
    game.SetupCache();
    input.round = rnd;
    input.playerID = playerID;

    // Fill out testcase output.
    TestcaseOutput output;
    output.expert_move.pos = expert_move;
    emp::vector<othello_idx_t> valid_moves = game.GetMoveOptions(playerID);
    output.move_valid.resize(game.GetNumCells());   // Resize bit vector to match board size.
    for (size_t i = 0; i < output.move_valid.size(); ++i) output.move_valid[i] = 0;
    for (size_t i = 0; i < valid_moves.size(); ++i) {
      output.move_valid[valid_moves[i]] = 1;
    }
    return test_case_t(input, output);
  }

  /// Run all tests on current eval hardware.
  void Evaluate(Agent & agent) {
    const size_t id = agent.GetID();
    Phenotype & phen = agent_phen_cache[id];
    // Reset score and various phenotype information.
    double score = 0.0;
    phen.illegal_move_total = 0;
    phen.valid_move_total = 0;
    phen.expert_move_total = 0;
    // Evaluate agent on all test cases.
    for (cur_testcase = 0; cur_testcase < testcases.GetSize(); ++cur_testcase) {
      const double test_score = this->RunTest(id, cur_testcase);
      phen.testcase_scores[cur_testcase] = test_score;
      if (test_score == SCORE_MOVE__EXPERT_MOVE_VALUE) {
        phen.expert_move_total += 1;
        phen.valid_move_total += 1;
      } else if (test_score == SCORE_MOVE__LEGAL_MOVE_VALUE) {
        phen.valid_move_total += 1;
      } else if (test_score == SCORE_MOVE__ILLEGAL_MOVE_VALUE){
        phen.illegal_move_total += 1;
      } else {
        std::cout << "Trying to record phenotype information about move types. Something went horribly wrong." << std::endl;
        exit(-1);
      }
      score += test_score;
    }
    phen.aggregate_score = score;
    // Trigger systematics-recording functions:
    record_fit_sig.Trigger(id, score);
    record_phen_sig.Trigger(id, phen.testcase_scores); // TODO: pass reference instead of full vector.
  }

  /// Run test return test score.
  double RunTest(size_t agentID, size_t testID) {
    // If we haven't cached this test case yet, go ahead and calculate it.
    test_case_t & test = testcases[testID];
    othello_idx_t move = EvalMove__GP(test.GetInput().game, false);
    return calc_test_score(test, move);
  }

  facing_t IntToFacing(int dir) {
    dir = emp::Mod(dir, othello_t::NUM_DIRECTIONS);
    switch(dir) {
      case 0: return facing_t::N;
      case 1: return facing_t::NE;
      case 2: return facing_t::E;
      case 3: return facing_t::SE;
      case 4: return facing_t::S;
      case 5: return facing_t::SW;
      case 6: return facing_t::W;
      case 7: return facing_t::NW;
    }
    emp_assert(false); return facing_t::N; //< Should never get here.
  }

  // Elite select mask.
  template<typename WORLD_TYPE>
  void EliteSelect_MASK(WORLD_TYPE & world, size_t e_count=1, size_t copy_count=1) {
    if (copy_count == 0 || e_count == 0) { return; }
    emp::EliteSelect(world, e_count, copy_count);
  }

public:
  LineageExp(const LineageConfig & config)   // @constructor
    : update(0), eval_time(0), OTHELLO_MAX_ROUND_CNT(0), best_agent_id(0), testcases(), cur_testcase(0), last_mutation()//,
      // sgp_muller_file(DATA_DIRECTORY + "muller_data.dat"),
      // agp_muller_file(DATA_DIRECTORY + "muller_data.dat")
  {
    // Localize configs.
    RUN_MODE = config.RUN_MODE();
    RANDOM_SEED = config.RANDOM_SEED();
    POP_SIZE = config.POP_SIZE();
    GENERATIONS = config.GENERATIONS();
    AGP_GENOME_SIZE = config.AGP_GENOME_SIZE();
    EVAL_TIME = config.EVAL_TIME();
    REPRESENTATION = config.REPRESENTATION();
    TEST_CASE_FILE = config.TEST_CASE_FILE();
    ANCESTOR_FPATH = config.ANCESTOR_FPATH();
    POP_INITIALIZATION_METHOD = config.POP_INITIALIZATION_METHOD();
    SELECTION_METHOD = config.SELECTION_METHOD();
    ELITE_SELECT__ELITE_CNT = config.ELITE_SELECT__ELITE_CNT();
    TOURNAMENT_SIZE = config.TOURNAMENT_SIZE();
    RESOURCE_SELECT__MODE = config.RESOURCE_SELECT__MODE();
    RESOURCE_SELECT__RES_AMOUNT = config.RESOURCE_SELECT__RES_AMOUNT();
    RESOURCE_SELECT__RES_INFLOW = config.RESOURCE_SELECT__RES_INFLOW();
    RESOURCE_SELECT__OUTFLOW = config.RESOURCE_SELECT__OUTFLOW();
    RESOURCE_SELECT__FRAC = config.RESOURCE_SELECT__FRAC();
    RESOURCE_SELECT__MAX_BONUS = config.RESOURCE_SELECT__MAX_BONUS();
    RESOURCE_SELECT__COST = config.RESOURCE_SELECT__COST();
    RESOURCE_SELECT__GAME_PHASE_LEN = config.RESOURCE_SELECT__GAME_PHASE_LEN();
    SCORE_MOVE__ILLEGAL_MOVE_VALUE = config.SCORE_MOVE__ILLEGAL_MOVE_VALUE();
    SCORE_MOVE__LEGAL_MOVE_VALUE = config.SCORE_MOVE__LEGAL_MOVE_VALUE();
    SCORE_MOVE__EXPERT_MOVE_VALUE = config.SCORE_MOVE__EXPERT_MOVE_VALUE();
    OTHELLO_HW_BOARDS = config.OTHELLO_HW_BOARDS();
    SGP_FUNCTION_LEN = config.SGP_FUNCTION_LEN();
    SGP_FUNCTION_CNT = config.SGP_FUNCTION_CNT();
    SGP_PROG_MAX_LENGTH = config.SGP_PROG_MAX_LENGTH();
    SGP_HW_MAX_CORES = config.SGP_HW_MAX_CORES();
    SGP_HW_MAX_CALL_DEPTH = config.SGP_HW_MAX_CALL_DEPTH();
    SGP_HW_MIN_BIND_THRESH = config.SGP_HW_MIN_BIND_THRESH();
    SGP_PROG_MAX_ARG_VAL = config.SGP_PROG_MAX_ARG_VAL();
    SGP_PER_BIT__TAG_BFLIP_RATE = config.SGP_PER_BIT__TAG_BFLIP_RATE();
    SGP_PER_INST__SUB_RATE = config.SGP_PER_INST__SUB_RATE();
    AGP_PER_INST__SUB_RATE = config.AGP_PER_INST__SUB_RATE();
    SGP_VARIABLE_LENGTH = config.SGP_VARIABLE_LENGTH();
    SGP_PER_INST__INS_RATE = config.SGP_PER_INST__INS_RATE();
    SGP_PER_INST__DEL_RATE = config.SGP_PER_INST__DEL_RATE();
    SGP_PER_FUNC__FUNC_DUP_RATE = config.SGP_PER_FUNC__FUNC_DUP_RATE();
    SGP_PER_FUNC__FUNC_DEL_RATE = config.SGP_PER_FUNC__FUNC_DEL_RATE();
    SYSTEMATICS_INTERVAL = config.SYSTEMATICS_INTERVAL();
    FITNESS_INTERVAL = config.FITNESS_INTERVAL();
    POP_SNAPSHOT_INTERVAL = config.POP_SNAPSHOT_INTERVAL();
    DATA_DIRECTORY = config.DATA_DIRECTORY();
    ANALYSIS_TYPE = config.ANALYSIS_TYPE();
    ANALYZE_PROGRAM_FPATH = config.ANALYZE_PROGRAM_FPATH();

    // Make a random number generator.
    random = emp::NewPtr<emp::Random>(RANDOM_SEED);

    // What is the maximum number of rounds for an othello game?
    OTHELLO_MAX_ROUND_CNT = (OTHELLO_BOARD_WIDTH * OTHELLO_BOARD_WIDTH) - 4;

    for (size_t i = 0; i < MUTATION_TYPES.size(); ++i) {
      last_mutation.emplace(MUTATION_TYPES[i], 0);
    }

    // Load test cases.
    testcases.RegisterTestcaseReader([this](emp::vector<std::string> & strs) { return this->GenerateTestcase(strs); });
    testcases.LoadTestcases(TEST_CASE_FILE);

    // Setup testcase cache.
    // testcase_score_cache.resize(POP_SIZE * testcases.GetSize(), 0.0);
    // agent_score_cache.resize(POP_SIZE, 0.0);
    agent_phen_cache.resize(POP_SIZE);
    for (size_t i = 0; i < agent_phen_cache.size(); ++i) {
      agent_phen_cache[i].testcase_scores.resize(testcases.GetSize(), 0);
      agent_phen_cache[i].illegal_move_total = 0; //TODO: reset these between evaluations
      agent_phen_cache[i].valid_move_total = 0;
      agent_phen_cache[i].expert_move_total = 0;
      agent_phen_cache[i].aggregate_score = 0;
    }

    // Organize testcase IDs into phases.
    // - How many phases are we working with?
    int bucket_cnt = std::ceil(((double)OTHELLO_MAX_ROUND_CNT)/((double)RESOURCE_SELECT__GAME_PHASE_LEN));
    testcases_by_phase.resize(bucket_cnt);
    // - What phase does each test case belong to?
    for (size_t i = 0; i < testcases.GetSize(); ++i) {
      size_t rd = testcases[i].GetInput().round;
      size_t phase = emp::Min(rd/RESOURCE_SELECT__GAME_PHASE_LEN, testcases_by_phase.size() - 1);
      testcases_by_phase[phase].emplace_back(i);
    }

    // Fill out resources based on resource select mode
    switch (RESOURCE_SELECT__MODE) {
      case RESOURCE_SELECT_MODE_ID__PHASES: {
        // Fill out resources.
        for (size_t i = 0; i < bucket_cnt; ++i) {
          resources.emplace_back(RESOURCE_SELECT__RES_AMOUNT, RESOURCE_SELECT__RES_INFLOW, RESOURCE_SELECT__OUTFLOW);
        }
        // Print out test case distribution.
        std::cout << "Test case phase distribution (phase:size):";
        for (size_t i = 0; i < testcases_by_phase.size(); ++i) {
          std::cout << " " << i << ":" << testcases_by_phase[i].size();
        } std::cout << std::endl;
        break;
      }
      case RESOURCE_SELECT_MODE_ID__INDIV: {
        for (size_t i = 0; i < testcases.GetSize(); ++i) {
          resources.emplace_back(RESOURCE_SELECT__RES_AMOUNT, RESOURCE_SELECT__RES_INFLOW, RESOURCE_SELECT__OUTFLOW);
        }
        break;
      }
      default:
        std::cout << "Unrecognized resource select mode. Exiting..." << std::endl;
        exit(-1);
    }

    // Because roulette select can't take negative scores, make sure we'll never
    // return negative values from calc_test_score (i.e. check user-input parameters)
    if (SCORE_MOVE__ILLEGAL_MOVE_VALUE <= 0 || SCORE_MOVE__LEGAL_MOVE_VALUE <= 0 || SCORE_MOVE__EXPERT_MOVE_VALUE <= 0) {
      std::cout << "Move scores cannot be less than 0 (gee thanks, RouletteSelect)! Exiting..." << std::endl;
      exit(-1);
    }
    // Given a test case and a move, how are we scoring an agent?
    calc_test_score = [this](test_case_t & test, othello_idx_t move) {
      // Score move given test.
      if (move == test.GetOutput().expert_move) {
        return SCORE_MOVE__EXPERT_MOVE_VALUE;
      } else if (!move.IsValid()) {
        return SCORE_MOVE__ILLEGAL_MOVE_VALUE;
      } else {
        return (test.GetOutput().move_valid[move]) ? SCORE_MOVE__LEGAL_MOVE_VALUE : SCORE_MOVE__ILLEGAL_MOVE_VALUE;
      }
    };

    for (size_t i = 0; i < testcases.GetSize(); ++i) {
      std::cout << "============= Test case: " << i << " =============" << std::endl;
      std::cout << "ID: " << testcases[i].id << std::endl;
      std::cout << "Input" << std::endl;
      // Board
      testcases[i].GetInput().game.Print();
      auto options = testcases[i].GetInput().game.GetMoveOptions(testcases[i].GetInput().playerID);
      std::cout << "Board options: ";
      for (size_t j = 0; j < options.size(); ++j) {
        std::cout << " " << options[j];
      } std::cout << std::endl;
      std::cout << "Valid options: ";
      for (size_t j = 0; j < testcases[i].GetOutput().move_valid.size(); ++j) {
        std::cout << " " << testcases[i].GetOutput().move_valid[j];
      } std::cout << std::endl;
      std::cout << "Board width: " << testcases[i].GetInput().game.GetBoardWidth() << std::endl;
      std::cout << "Round: " << testcases[i].GetInput().round << std::endl;
      std::cout << "PlayerID: " << testcases[i].GetInput().playerID << std::endl;
      std::cout << "Output" << std::endl;
      std::cout << "Expert move: " << testcases[i].GetOutput().expert_move << std::endl;
    }

    // Configure the dreamware!
    othello_dreamware = emp::NewPtr<OthelloHardware>(1);

    // Make the world(s)!
    // - SGP World -
    sgp_world = emp::NewPtr<SGP__world_t>(random, "SGP-LineageAnalysis-World");
    agp_world = emp::NewPtr<AGP__world_t>(random, "AGP-LineageAnalysis-World");

    // Configure instruction/event libraries.
    sgp_inst_lib = emp::NewPtr<SGP__inst_lib_t>();
    sgp_event_lib = emp::NewPtr<SGP__event_lib_t>();
    agp_inst_lib = emp::NewPtr<AGP__inst_lib_t>();

    if (RUN_MODE == RUN_ID__EXP) {
      // Make data directory.
      mkdir(DATA_DIRECTORY.c_str(), ACCESSPERMS);
      if (DATA_DIRECTORY.back() != '/') DATA_DIRECTORY += '/';
    }

    // Config experiment based on representation type.
    switch (REPRESENTATION) {
      case REPRESENTATION_ID__AVIDAGP:
        ConfigAGP();
        break;
      case REPRESENTATION_ID__SIGNALGP:
        ConfigSGP();
        break;
      default:
        std::cout << "Unrecognized representation configuration setting (" << REPRESENTATION << "). Exiting..." << std::endl;
        exit(-1);
    }
  }

  ~LineageExp() {
    random.Delete();
    othello_dreamware.Delete();
    sgp_world.Delete();
    agp_world.Delete();
    sgp_inst_lib.Delete();
    agp_inst_lib.Delete();
    sgp_event_lib.Delete();
    sgp_eval_hw.Delete();
    agp_eval_hw.Delete();
  }

  void Run() {
    switch (RUN_MODE) {
      case RUN_ID__EXP: {
        do_begin_run_setup_sig.Trigger();
        for (update = 0; update <= GENERATIONS; ++update) {
          RunStep();
          if (update % POP_SNAPSHOT_INTERVAL == 0) do_pop_snapshot_sig.Trigger(update);
        }
        break;
      }
      case RUN_ID__ANALYSIS:
        do_analysis_sig.Trigger();
        break;
      default:
        std::cout << "Unrecognized run mode! Exiting..." << std::endl;
        exit(-1);
    }
  }

  /// Do a single step of evolution.
  void RunStep() {
    do_evaluation_sig.Trigger();    // Update agent scores.
    do_selection_sig.Trigger();     // Do selection (selection, reproduction, mutation).
    do_world_update_sig.Trigger();  // Do world update (population turnover, clear score caches).
  }

  // Fitness function(s)
  double CalcFitness(Agent & agent) {
    return agent_phen_cache[agent.GetID()].aggregate_score;
  }

  // Config functions. These do all of the hardware-specific experiment setup/configuration.
  void ConfigSGP();
  void ConfigSGP_InstLib();
  void ConfigAGP();
  void ConfigAGP_InstLib();

  // Mutation functions
  size_t SGP__Mutate_FixedLength(SignalGPAgent & agent, emp::Random & rnd);
  size_t SGP__Mutate_VariableLength(SignalGPAgent & agent, emp::Random & rnd);
  size_t AGP__Mutate(AvidaGPAgent & agent, emp::Random & rnd);

  // Population snapshot functions
  void SGP_Snapshot_SingleFile(size_t update);
  void AGP_Snapshot_SingleFile(size_t update);

  template <typename WORLD_TYPE>
  emp::DataFile & AddBestPhenotypeFile(WORLD_TYPE & world, const std::string & fpath="best_phenotype.csv") {
      auto & file = world.SetupFile(fpath);

      std::function<size_t(void)> get_update = [&world](){ return world.GetUpdate(); };
      file.AddFun(get_update, "update", "Update");

      // NOTE: when this gets called, world->GetOrg(best_agent_id) is no longer accurate.
      //       But... agent_phen_cache is still good to go.
      std::function<double(void)> get_score = [&world, this]() {
        Phenotype & best_phen = this->agent_phen_cache[this->best_agent_id];
        return best_phen.aggregate_score;
      };
      file.AddFun(get_score, "score", "get best phenotype score from this update");

      std::function<size_t(void)> get_illegal_total = [&world, this]() {
        Phenotype & best_phen = this->agent_phen_cache[this->best_agent_id];
        return best_phen.illegal_move_total;
      };
      file.AddFun(get_illegal_total, "illegal_move_total", "get total illegal moves made by best phenotype from this update");

      std::function<size_t(void)> get_valid_total = [&world, this]() {
        Phenotype & best_phen = this->agent_phen_cache[this->best_agent_id];
        return best_phen.valid_move_total;
      };
      file.AddFun(get_valid_total, "valid_move_total", "get total valid moves made by best phenotype from this update");

      std::function<size_t(void)> get_expert_total = [&world, this]() {
        Phenotype & best_phen = this->agent_phen_cache[this->best_agent_id];
        return best_phen.expert_move_total;
      };
      file.AddFun(get_expert_total, "expert_move_total", "get total expert moves made by best phenotype from this update");
      file.PrintHeaderKeys();
      return file;
  }

  // SignalGP utility functions.
  void SGP__InitPopulation_Random();
  void SGP__InitPopulation_FromAncestorFile();
  void SGP__ResetHW(const SGP__memory_t & main_in_mem=SGP__memory_t());

  //AvidaGP utility functions.
  void AGP__InitPopulation_Random();
  void AGP__InitPopulation_FromAncestorFile();
  void AGP__ResetHW();

  // SignalGP Analysis functions.
  void SGP__Debugging_Analysis();

  // -- AvidaGP Instructions --
  // BoardWidth
  void AGP__Inst_GetBoardWidth(AGP__hardware_t &hw, const AGP__inst_t &inst);
  // EndTurn
  void AGP__Inst_EndTurn(AGP__hardware_t &hw, const AGP__inst_t &inst);
  // SetMove
  void AGP__Inst_SetMoveXY(AGP__hardware_t &hw, const AGP__inst_t &inst);
  void AGP__Inst_SetMoveID(AGP__hardware_t &hw, const AGP__inst_t &inst);
  // GetMove
  void AGP__Inst_GetMoveXY(AGP__hardware_t &hw, const AGP__inst_t &inst);
  void AGP__Inst_GetMoveID(AGP__hardware_t &hw, const AGP__inst_t &inst);
  // Adjacent
  void AGP__Inst_AdjacentXY(AGP__hardware_t &hw, const AGP__inst_t &inst);
  void AGP__Inst_AdjacentID(AGP__hardware_t &hw, const AGP__inst_t &inst);
  // ValidMovesCnt
  void AGP__Inst_ValidMoveCnt_HW(AGP__hardware_t &hw, const AGP__inst_t &inst);
  // ValidOppMovesCnt
  void AGP__Inst_ValidOppMoveCnt_HW(AGP__hardware_t &hw, const AGP__inst_t &inst);
  // GetBoardValue
  void AGP__Inst_GetBoardValueXY_HW(AGP__hardware_t &hw, const AGP__inst_t &inst);
  void AGP__Inst_GetBoardValueID_HW(AGP__hardware_t &hw, const AGP__inst_t &inst);
  // IsValidXY
  void AGP__Inst_IsValidXY_HW(AGP__hardware_t &hw, const AGP__inst_t &inst);
  void AGP__Inst_IsValidID_HW(AGP__hardware_t &hw, const AGP__inst_t &inst);
  // IsValidOpp
  void AGP__Inst_IsValidOppXY_HW(AGP__hardware_t &hw, const AGP__inst_t &inst);
  void AGP__Inst_IsValidOppID_HW(AGP__hardware_t &hw, const AGP__inst_t &inst);
  // PlaceXY
  void AGP__Inst_PlaceDiskXY_HW(AGP__hardware_t &hw, const AGP__inst_t &inst);
  void AGP__Inst_PlaceDiskID_HW(AGP__hardware_t &hw, const AGP__inst_t &inst);
  // PlaceOppXY
  void AGP__Inst_PlaceOppDiskID_HW(AGP__hardware_t &hw, const AGP__inst_t &inst);
  void AGP__Inst_PlaceOppDiskXY_HW(AGP__hardware_t &hw, const AGP__inst_t &inst);
  // FlipCn_tXY
  void AGP__Inst_FlipCntXY_HW(AGP__hardware_t &hw, const AGP__inst_t &inst);
  void AGP__Inst_FlipCntID_HW(AGP__hardware_t &hw, const AGP__inst_t &inst);
  // OppFli_pCntXY
  void AGP__Inst_OppFlipCntXY_HW(AGP__hardware_t &hw, const AGP__inst_t &inst);
  void AGP__Inst_OppFlipCntID_HW(AGP__hardware_t &hw, const AGP__inst_t &inst);
  // FrontierCnt
  void AGP__Inst_FrontierCnt_HW(AGP__hardware_t &hw, const AGP__inst_t &inst);
  // ResetBoard
  void AGP__Inst_ResetBoard_HW(AGP__hardware_t &hw, const AGP__inst_t &inst);
  // IsOver
  void AGP__Inst_IsOver_HW(AGP__hardware_t &hw, const AGP__inst_t &inst);

  // -- SignalGP Instructions --
  // Fork
  void SGP__Inst_Fork(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // BoardWidth
  void SGP_Inst_GetBoardWidth(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // EndTurn
  void SGP_Inst_EndTurn(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // SetMove
  void SGP__Inst_SetMoveXY(SGP__hardware_t & hw, const SGP__inst_t & inst);
  void SGP__Inst_SetMoveID(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // GetMove
  void SGP__Inst_GetMoveXY(SGP__hardware_t & hw, const SGP__inst_t & inst);
  void SGP__Inst_GetMoveID(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // Adjacent
  void SGP__Inst_AdjacentXY(SGP__hardware_t & hw, const SGP__inst_t & inst);
  void SGP__Inst_AdjacentID(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // IsValid
  void SGP__Inst_IsValidXY_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  void SGP__Inst_IsValidID_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // IsValidOpp
  void SGP__Inst_IsValidOppXY_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  void SGP__Inst_IsValidOppID_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // ValidMovesCnt
  void SGP__Inst_ValidMoveCnt_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // ValidOppMovesCnt
  void SGP__Inst_ValidOppMoveCnt_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // GetBoardValue
  void SGP__Inst_GetBoardValueXY_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  void SGP__Inst_GetBoardValueID_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // Place
  void SGP__Inst_PlaceDiskXY_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  void SGP__Inst_PlaceDiskID_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // PlaceOpp
  void SGP__Inst_PlaceOppDiskXY_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  void SGP__Inst_PlaceOppDiskID_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // FlipCnt
  void SGP__Inst_FlipCntXY_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  void SGP__Inst_FlipCntID_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // OppFlipCnt
  void SGP__Inst_OppFlipCntXY_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  void SGP__Inst_OppFlipCntID_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // FrontierCnt
  void SGP__Inst_FrontierCnt_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // ResetBoard
  void SGP__Inst_ResetBoard_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
  // IsOver
  void SGP__Inst_IsOver_HW(SGP__hardware_t & hw, const SGP__inst_t & inst);
};

// AvidaGP Functions
void LineageExp::AGP__InitPopulation_Random()
{
  std::cout << "Initializing population randomly!" << std::endl;
  for (size_t p = 0; p < POP_SIZE; ++p)
  {
    AGP__hardware_t cpu(agp_inst_lib);
    cpu.PushRandom(*random, AGP_GENOME_SIZE);
    agp_world->Inject(cpu.GetGenome(), 1);
  }
}

void LineageExp::AGP__InitPopulation_FromAncestorFile()
{
  std::cout << "Initializing population from ancestor file!" << std::endl;
  // Configure the ancestor program.
  AGP__hardware_t ancestor_prog(agp_inst_lib);
  std::ifstream ancestor_fstream(ANCESTOR_FPATH);
  if (!ancestor_fstream.is_open())
  {
    std::cout << "Failed to open ancestor program file(" << ANCESTOR_FPATH << "). Exiting..." << std::endl;
    exit(-1);
  }
  ancestor_prog.Load(ancestor_fstream);
  std::cout << " --- Ancestor program: ---" << std::endl;
  ancestor_prog.PrintGenome();
  std::cout << " -------------------------" << std::endl;
  agp_world->Inject(ancestor_prog.GetGenome(), 1); // Inject a bunch of ancestors into the population.
}

size_t LineageExp::AGP__Mutate(AvidaGPAgent &agent, emp::Random &rnd)
{
  AGP__program_t &program = agent.GetGenome();
  size_t mut_cnt = 0;
  for (size_t i = 0; i < MUTATION_TYPES.size(); ++i) {
    last_mutation[MUTATION_TYPES[i]] = 0;
  }
  // Substitutions?
  for (size_t i = 0; i < program.sequence.size(); ++i)
  {
    AGP__inst_t &inst = program.sequence[i];
    // Mutate instruction.
    if (rnd.P(AGP_PER_INST__SUB_RATE))
    {
      ++mut_cnt;
      ++last_mutation["inst_substitutions"];
      inst.id = rnd.GetUInt(program.inst_lib->GetSize());
    }
    // Mutate arguments (even if they aren't relevent to instruction).
    for (size_t k = 0; k < AGP__hardware_t::INST_ARGS; ++k)
    {
      if (rnd.P(AGP_PER_INST__SUB_RATE))
      {
        ++mut_cnt;
        ++last_mutation["arg_substitutions"];
        inst.args[k] = rnd.GetInt(AGP__hardware_t::CPU_SIZE);
      }
    }
  }

  return mut_cnt;
}

void AGP__PrintGenome(std::ostream &os, LineageExp::AGP__program_t &genome)
{
  for (const LineageExp::AGP__inst_t & inst : genome.sequence) {
    os << genome.inst_lib->GetName(inst.id);
    const size_t num_args = genome.inst_lib->GetNumArgs(inst.id);
    for (size_t i = 0; i < num_args; i++)
    {
      os << ' ' << inst.args[i];
    }
    os << '\n';
  }
}

void LineageExp::AGP_Snapshot_SingleFile(size_t update)
{
  std::string snapshot_dir = DATA_DIRECTORY + "pop_" + emp::to_string((int)update);
  mkdir(snapshot_dir.c_str(), ACCESSPERMS);
  // For each program in the population, dump the full program description in a single file.
  std::ofstream prog_ofstream(snapshot_dir + "/pop_" + emp::to_string((int)update) + ".pop");
  for (size_t i = 0; i < agp_world->GetSize(); ++i)
  {
    if (i)
      prog_ofstream << "===\n";
    AvidaGPAgent &agent = agp_world->GetOrg(i);
    AGP__PrintGenome(prog_ofstream, agent.GetGenome());
  }
  prog_ofstream.close();
}

void LineageExp::AGP__ResetHW()
{
  agp_eval_hw->ResetHardware();
  agp_eval_hw->SetTrait(TRAIT_ID__MOVE, -1);
  agp_eval_hw->SetTrait(TRAIT_ID__DONE, 0);
}

// SignalGP Functions
/// Reset the SignalGP evaluation hardware, setting input memory of
/// main thread to be equal to main_in_mem.
void LineageExp::SGP__ResetHW(const SGP__memory_t & main_in_mem) {
  sgp_eval_hw->ResetHardware();
  sgp_eval_hw->SetTrait(TRAIT_ID__MOVE, -1);
  sgp_eval_hw->SetTrait(TRAIT_ID__DONE, 0);
  sgp_eval_hw->SpawnCore(0, main_in_mem, true);
}

void LineageExp::SGP__InitPopulation_Random() {
  std::cout << "Initializing population randomly!" << std::endl;
  for (size_t p = 0; p < POP_SIZE; ++p) {
    SGP__program_t prog(sgp_inst_lib);
    for (size_t f = 0; f < SGP_FUNCTION_CNT; ++f) {
      prog.PushFunction();
      prog[f].affinity.Randomize(*random);
      for (size_t i = 0; i < SGP_FUNCTION_LEN; ++i) {
        const size_t instID = random->GetUInt(sgp_inst_lib->GetSize());
        const size_t a0 = random->GetUInt(0, SGP_PROG_MAX_ARG_VAL);
        const size_t a1 = random->GetUInt(0, SGP_PROG_MAX_ARG_VAL);
        const size_t a2 = random->GetUInt(0, SGP_PROG_MAX_ARG_VAL);
        SGP__inst_t inst(instID, a0, a1, a2);
        inst.affinity.Randomize(*random);
        prog[f].PushInst(inst);
      }
    }
    sgp_world->Inject(prog,1);
  }
}

void LineageExp::SGP__InitPopulation_FromAncestorFile() {
  std::cout << "Initializing population from ancestor file!" << std::endl;
  // Configure the ancestor program.
  SGP__program_t ancestor_prog(sgp_inst_lib);
  std::ifstream ancestor_fstream(ANCESTOR_FPATH);
  if (!ancestor_fstream.is_open()) {
    std::cout << "Failed to open ancestor program file(" << ANCESTOR_FPATH << "). Exiting..." << std::endl;
    exit(-1);
  }
  ancestor_prog.Load(ancestor_fstream);
  std::cout << " --- Ancestor program: ---" << std::endl;
  ancestor_prog.PrintProgramFull();
  std::cout << " -------------------------" << std::endl;
  sgp_world->Inject(ancestor_prog, 1);    // Inject a bunch of ancestors into the population.
}

/// Analysis mode used for tracing program.
/// Primary purpose of this analysis type is to help debug this experiment. Woo!
void LineageExp::SGP__Debugging_Analysis() {
  std::cout << "\nRunning DEBUGGING analysis...\n" << std::endl;
  // Configure analysis program.
  SGP__program_t analyze_prog(sgp_inst_lib);
  std::ifstream analyze_fstream(ANALYZE_PROGRAM_FPATH);
  if (!analyze_fstream.is_open()) {
    std::cout << "Failed to open analysis program file(" << ANALYZE_PROGRAM_FPATH << "). Exiting..." << std::endl;
    exit(-1);
  }
  analyze_prog.Load(analyze_fstream);
  std::cout << " --- Analysis program: ---" << std::endl;
  analyze_prog.PrintProgramFull();
  std::cout << " -------------------------" << std::endl;

  // Load program onto agent.
  SignalGPAgent our_hero(analyze_prog);
  our_hero.SetID(0);
  sgp_eval_hw->SetProgram(our_hero.GetGenome());
  // this->Evaluate(our_hero);
  double score = 0.0;
  for (cur_testcase = 0; cur_testcase < testcases.GetSize(); ++cur_testcase) {
    double test_score = RunTest(our_hero.GetID(), cur_testcase);
    std::cout << "TEST CASE " << cur_testcase << " SCORE: " << test_score << std::endl;
    score += test_score;
    // How did it do?
  }
  Phenotype & phen =  agent_phen_cache[our_hero.GetID()];
  // agent_score_cache[our_hero.GetID()] = score;
  phen.aggregate_score = score;

  std::cout << "\n\nFINAL SCORE (total): " << score << std::endl;
  std::cout << "Test case scores: {";
  for (size_t i = 0; i < testcases.GetSize(); ++i) {
    std::cout << "Test " << i << ": " << phen.testcase_scores[i] << ", ";
  } std::cout << "}" << std::endl;
}

/// Mutate an SGP agent's program. Only does tag mutations, instruction substitutions, and
/// argument substitutions. (maintains constand-length genomes)
size_t LineageExp::SGP__Mutate_FixedLength(SignalGPAgent & agent, emp::Random & rnd) {
  SGP__program_t & program = agent.GetGenome();
  size_t mut_cnt = 0;
  for (size_t i = 0; i < MUTATION_TYPES.size(); ++i) {
    last_mutation[MUTATION_TYPES[i]] = 0;
  }
  // For each function:
  for (size_t fID = 0; fID < program.GetSize(); ++fID) {
    // Mutate affinity.
    for (size_t i = 0; i < program[fID].GetAffinity().GetSize(); ++i) {
      SGP__tag_t & aff = program[fID].GetAffinity();
      if (rnd.P(SGP_PER_BIT__TAG_BFLIP_RATE)) {
        ++last_mutation["tag_bit_flips"];
        ++mut_cnt;
        aff.Set(i, !aff.Get(i));
      }
    }
    // Substitutions?
    for (size_t i = 0; i < program[fID].GetSize(); ++i) {
      SGP__inst_t & inst = program[fID][i];
      // Mutate affinity (even if it doesn't have one).
      for (size_t k = 0; k < inst.affinity.GetSize(); ++k) {
        if (rnd.P(SGP_PER_BIT__TAG_BFLIP_RATE)) {
          ++mut_cnt;
          ++last_mutation["tag_bit_flips"];
          inst.affinity.Set(k, !inst.affinity.Get(k));
        }
      }
      // Mutate instruction.
      if (rnd.P(SGP_PER_INST__SUB_RATE)) {
        ++mut_cnt;
        ++last_mutation["inst_substitutions"];
        inst.id = rnd.GetUInt(program.GetInstLib()->GetSize());
      }
      // Mutate arguments (even if they aren't relevent to instruction).
      for (size_t k = 0; k < SGP__hardware_t::MAX_INST_ARGS; ++k) {
        if (rnd.P(SGP_PER_INST__SUB_RATE)) {
          ++mut_cnt;
          ++last_mutation["arg_substitutions"];
          inst.args[k] = rnd.GetInt(SGP_PROG_MAX_ARG_VAL);
        }
      }
    }
  }
  return mut_cnt;
}


size_t LineageExp::SGP__Mutate_VariableLength(SignalGPAgent & agent, emp::Random & rnd) {
  SGP__program_t & program = agent.GetGenome();
  size_t mut_cnt = 0;
  // Reset last mutation.
  for (size_t i = 0; i < MUTATION_TYPES.size(); ++i) {
    last_mutation[MUTATION_TYPES[i]] = 0;
  }
  // Duplicate a function?
  size_t expected_prog_len = program.GetInstCnt();
  size_t old_content_wall = program.GetSize(); ///< First position (or invalid position) after old content.
  size_t fID = 0;
  while (fID < old_content_wall) {
    bool dup = rnd.P(SGP_PER_FUNC__FUNC_DUP_RATE);
    bool del = rnd.P(SGP_PER_FUNC__FUNC_DEL_RATE);
    if (dup && del) { dup = false; del = false; } // If we're about to dup and del, don't do anything.
    // Do we duplicate?
    if (dup && ((expected_prog_len + program[fID].GetSize()) <= SGP_PROG_MAX_LENGTH)) {
      // Adjust expected program length (total instructions).
      expected_prog_len += program[fID].GetSize();
      // Duplication.
      program.PushFunction(program[fID]);
      ++last_mutation["func_duplications"];
      ++mut_cnt;
    // Do we delete?
    } else if (del && program.GetSize() > 1) {
      // Adjust expected program length (total instructions).
      expected_prog_len -= program[fID].GetSize();
      const size_t mfID = program.GetSize()-1;
      // Deletion.
      program[fID] = program[mfID];
      program.program.resize(mfID);
      // Should we adjust the wall?
      if (mfID < old_content_wall) {
        // We're moving from within the wall, adjust wall.
        --old_content_wall;
        --fID;
      }
      ++last_mutation["func_deletions"];
      ++mut_cnt;
    }
    ++fID;
  }
  // For each function...
  for (size_t fID = 0; fID < program.GetSize(); ++fID) {
    // Mutate affinity
    for (size_t i = 0; i < program[fID].GetAffinity().GetSize(); ++i) {
      SGP__tag_t & aff = program[fID].GetAffinity();
      if (rnd.P(SGP_PER_BIT__TAG_BFLIP_RATE)) {
        ++mut_cnt;
        ++last_mutation["tag_bit_flips"];
        aff.Set(i, !aff.Get(i));
      }
    }

    // Substitution mutations?
    for (size_t i = 0; i < program[fID].GetSize(); ++i) {
      SGP__inst_t & inst = program[fID][i];
      // Mutate affinity (even if it doesn't have one).
      for (size_t k = 0; k < inst.affinity.GetSize(); ++k) {
        if (rnd.P(SGP_PER_BIT__TAG_BFLIP_RATE)) {
          ++mut_cnt;
          ++last_mutation["tag_bit_flips"];
          inst.affinity.Set(k, !inst.affinity.Get(k));
        }
      }
      // Mutate instruction.
      if (rnd.P(SGP_PER_INST__SUB_RATE)) {
        ++mut_cnt;
        ++last_mutation["inst_substitutions"];
        inst.id = rnd.GetUInt(program.GetInstLib()->GetSize());
      }
      // Mutate arguments (even if they aren't relevent to instruction).
      for (size_t k = 0; k < SGP__hardware_t::MAX_INST_ARGS; ++k) {
        if (rnd.P(SGP_PER_INST__SUB_RATE)) {
          ++mut_cnt;
          ++last_mutation["arg_substitutions"];
          inst.args[k] = rnd.GetInt(SGP_PROG_MAX_ARG_VAL);
        }
      }
    }
    // Insertion/deletion mutations?
    // - Compute insertions.
    int num_ins = rnd.GetRandBinomial(program[fID].GetSize(), SGP_PER_INST__INS_RATE);
    // Ensure that insertions don't exceed maximum program length.
    if ((num_ins + expected_prog_len) > SGP_PROG_MAX_LENGTH) {
      num_ins = SGP_PROG_MAX_LENGTH - expected_prog_len;
    }
    expected_prog_len += num_ins;

    // Do we need to do any insertions or deletions?
    if (num_ins > 0 || SGP_PER_INST__DEL_RATE > 0.0) {
      emp::vector<size_t> ins_locs = emp::RandomUIntVector(rnd, num_ins, 0, program[fID].GetSize());
      if (ins_locs.size()) std::sort(ins_locs.begin(), ins_locs.end(), std::greater<size_t>());
      SGP__hardware_t::Function new_fun(program[fID].GetAffinity());
      size_t rhead = 0;
      size_t num_dels = 0;
      while (rhead < program[fID].GetSize()) {
        if (ins_locs.size()) {
          if (rhead >= ins_locs.back()) {
            // Insert a random instruction.
            new_fun.PushInst(rnd.GetUInt(program.GetInstLib()->GetSize()),
                             rnd.GetInt(SGP_PROG_MAX_ARG_VAL),
                             rnd.GetInt(SGP_PROG_MAX_ARG_VAL),
                             rnd.GetInt(SGP_PROG_MAX_ARG_VAL),
                             SGP__tag_t());
            new_fun.inst_seq.back().affinity.Randomize(rnd);
            ++mut_cnt;
            ++last_mutation["inst_insertions"];
            ins_locs.pop_back();
            continue;
          }
        }
        // Do we delete this instruction?
        if (rnd.P(SGP_PER_INST__DEL_RATE) && num_dels < (program[fID].GetSize() - 1)) {
          ++mut_cnt;
          ++last_mutation["inst_deletions"];
          ++num_dels;
          --expected_prog_len;
        } else {
          new_fun.PushInst(program[fID][rhead]);
        }
        ++rhead;
      }
      program[fID] = new_fun;
    }
  }
  return mut_cnt;
}

void LineageExp::SGP_Snapshot_SingleFile(size_t update) {
  std::string snapshot_dir = DATA_DIRECTORY + "pop_" + emp::to_string((int)update);
  mkdir(snapshot_dir.c_str(), ACCESSPERMS);
  // For each program in the population, dump the full program description in a single file.
  std::ofstream prog_ofstream(snapshot_dir + "/pop_" + emp::to_string((int)update) + ".pop");
  for (size_t i = 0; i < sgp_world->GetSize(); ++i) {
    if (i) prog_ofstream << "===\n";
    SignalGPAgent & agent = sgp_world->GetOrg(i);
    agent.program.PrintProgramFull(prog_ofstream);
  }
  prog_ofstream.close();
}

#include "LineageExp__InstructionImpl.h"

void LineageExp::ConfigSGP() {
  // Configure the world.
  sgp_world->Reset();
  sgp_world->SetWellMixed(true);

  // Setup mutation function.
  if (SGP_VARIABLE_LENGTH) {
    sgp_world->SetMutFun([this](SignalGPAgent & agent, emp::Random & rnd) { return this->SGP__Mutate_VariableLength(agent, rnd); }, ELITE_SELECT__ELITE_CNT);
  } else {
    // NOTE: second argument specifies that we're not mutating the first thing int the pop (we're doing elite selection in all of our stuff).
    sgp_world->SetMutFun([this](SignalGPAgent & agent, emp::Random & rnd) { return this->SGP__Mutate_FixedLength(agent, rnd); }, ELITE_SELECT__ELITE_CNT);
  }

  sgp_world->SetFitFun([this](SignalGPAgent & agent) { return this->CalcFitness(agent); });
  sgp_world->OnGenotypeKnown([this](emp::Ptr<SGP__genotype_t> genotype, size_t pos) {
    // std::cout << "OnGenotypeKnown Mutations!" << std::endl;
    // for (size_t i = 0; i < MUTATION_TYPES.size(); ++i) {
    //   std::cout << "  " << MUTATION_TYPES[i] << ":" << last_mutation[MUTATION_TYPES[i]] << std::endl;
    // }
    genotype->GetData().RecordMutation(last_mutation);
  });

  ConfigSGP_InstLib();

  sgp_eval_hw = emp::NewPtr<SGP__hardware_t>(sgp_inst_lib, sgp_event_lib, random);
  sgp_eval_hw->SetMinBindThresh(SGP_HW_MIN_BIND_THRESH);
  sgp_eval_hw->SetMaxCores(SGP_HW_MAX_CORES);
  sgp_eval_hw->SetMaxCallDepth(SGP_HW_MAX_CALL_DEPTH);

  // - Setup move evaluation signals/functors -
  // Setup begin_turn_signal action:
  //  - Reset the evaluation hardware. Give hardware accurate playerID, and update the dreamboard.
  get_eval_agent_done = [this]() {
    return (bool)sgp_eval_hw->GetTrait(TRAIT_ID__DONE);
  };

  get_eval_agent_playerID = [this]() {
    return othello_dreamware->GetPlayerID();
  };

  // Setup triggers!
  // Configure initial run setup
  switch (POP_INITIALIZATION_METHOD) {
    case POP_INITIALIZATION_METHOD_ID__RANDOM_POP:
      do_pop_init_sig.AddAction([this]() {
        this->SGP__InitPopulation_Random();
      });
      break;
    case POP_INITIALIZATION_METHOD_ID__ANCESTOR_FILE:
      do_pop_init_sig.AddAction([this]() {
        this->SGP__InitPopulation_FromAncestorFile();
      });
      break;
    default:
      std::cout << "Unrecognized population initialization method! Exiting..." << std::endl;
      exit(-1);
  }

  do_begin_run_setup_sig.AddAction([this]() {
    std::cout << "Doing initial run setup." << std::endl;
    // Setup systematics/fitness tracking.
    auto & sys_file = sgp_world->SetupSystematicsFile(DATA_DIRECTORY + "systematics.csv");
    sys_file.SetTimingRepeat(SYSTEMATICS_INTERVAL);
    auto & fit_file = sgp_world->SetupFitnessFile(DATA_DIRECTORY + "fitness.csv");
    fit_file.SetTimingRepeat(FITNESS_INTERVAL);
    emp::AddPhylodiversityFile(*sgp_world, DATA_DIRECTORY + "phylodiversity.csv").SetTimingRepeat(SYSTEMATICS_INTERVAL);
    emp::AddLineageMutationFile(*sgp_world, DATA_DIRECTORY + "lineage_mutations.csv", MUTATION_TYPES).SetTimingRepeat(SYSTEMATICS_INTERVAL);
    AddDominantFile(*sgp_world, DATA_DIRECTORY + "dominant.csv", MUTATION_TYPES).SetTimingRepeat(SYSTEMATICS_INTERVAL);
    AddBestPhenotypeFile(*sgp_world, DATA_DIRECTORY+"best_phenotype.csv").SetTimingRepeat(SYSTEMATICS_INTERVAL);
    // sgp_muller_file = emp::AddMullerPlotFile(*sgp_world, DATA_DIRECTORY + "muller_data.dat");
    // sgp_world->OnUpdate([this](size_t ud){ if (ud % SYSTEMATICS_INTERVAL == 0) sgp_muller_file.Update(); });
    record_fit_sig.AddAction([this](size_t pos, double fitness) { sgp_world->GetGenotypeAt(pos)->GetData().RecordFitness(fitness); } );
    record_phen_sig.AddAction([this](size_t pos, const phenotype_t & phen) { sgp_world->GetGenotypeAt(pos)->GetData().RecordPhenotype(phen); } );
    // Generate the initial population.
    do_pop_init_sig.Trigger();
  });

  // - Configure evaluation
  // TODO: add dominant id tracking
  do_evaluation_sig.AddAction([this]() {
    double best_score = -32767;
    best_agent_id = 0;
    for (size_t id = 0; id < sgp_world->GetSize(); ++id) {
      // std::cout << "Evaluating agent: " << id << std::endl;
      // Evaluate agent given by id.
      SignalGPAgent & our_hero = sgp_world->GetOrg(id);
      our_hero.SetID(id);
      sgp_eval_hw->SetProgram(our_hero.GetGenome());
      this->Evaluate(our_hero);
      Phenotype & phen = agent_phen_cache[id];
      if (phen.aggregate_score > best_score) {
        best_score = phen.aggregate_score;
        best_agent_id = id;
      }
    }
    std::cout << "Update: " << update << " Max score: " << best_score << std::endl;
  });

  // - Configure world upate.
  do_world_update_sig.AddAction([this]() { sgp_world->Update(); });

  // do_pop_snapshot
  do_pop_snapshot_sig.AddAction([this](size_t update) { this->SGP_Snapshot_SingleFile(update); });

  // - Configure selection
  switch (SELECTION_METHOD) {
    case SELECTION_METHOD_ID__TOURNAMENT:
      do_selection_sig.AddAction([this]() {
        this->EliteSelect_MASK(*sgp_world, ELITE_SELECT__ELITE_CNT, 1);
        emp::TournamentSelect(*sgp_world, TOURNAMENT_SIZE, POP_SIZE - ELITE_SELECT__ELITE_CNT);
      });
      break;
    case SELECTION_METHOD_ID__LEXICASE: {
      sgp_lexicase_fit_set.resize(0);
      for (size_t i = 0; i < testcases.GetSize(); ++i) {
        sgp_lexicase_fit_set.push_back([i, this](SignalGPAgent & agent) {
          Phenotype & phen = agent_phen_cache[agent.GetID()];
          return phen.testcase_scores[i];
        });
      }
      do_selection_sig.AddAction([this]() {
        this->EliteSelect_MASK(*sgp_world, ELITE_SELECT__ELITE_CNT, 1);
        emp::LexicaseSelect(*sgp_world, sgp_lexicase_fit_set, POP_SIZE - ELITE_SELECT__ELITE_CNT);
      });
      break;
    }
    case SELECTION_METHOD_ID__ECOEA: {
      sgp_resource_fit_set.resize(0);
      // Setup the fitness function set based on resource select mode.
      switch (RESOURCE_SELECT__MODE) {
        case RESOURCE_SELECT_MODE_ID__PHASES: {
          // Add a resource fit function for each game phase.
          for (size_t i = 0; i < testcases_by_phase.size(); ++i) {
            sgp_resource_fit_set.push_back([i, this](SignalGPAgent & agent) {
              double score = 0;
              emp::vector<size_t> & phasecases = testcases_by_phase[i];
              // Aggregate all relevant test scores.
              for (size_t j = 0; j < phasecases.size(); ++j) {
                score += agent_phen_cache[agent.GetID()].testcase_scores[phasecases[j]];
              }
              return score;
            });
          }
          break;
        }
        case RESOURCE_SELECT_MODE_ID__INDIV: {
          // Add a resource fit function for each testcase.
          for (size_t i = 0; i < testcases.GetSize(); ++i) {
            sgp_resource_fit_set.push_back([i, this](SignalGPAgent & agent) {
              return agent_phen_cache[agent.GetID()].testcase_scores[i];
            });
          }
          break;
        }
        default:
          std::cout << "Unrecognized resource select mode. Exiting..." << std::endl;
          exit(-1);
      }
      // Setup the do selection signal action.
      do_selection_sig.AddAction([this]() {
        this->EliteSelect_MASK(*sgp_world, ELITE_SELECT__ELITE_CNT, 1);
        emp::ResourceSelect(*sgp_world, sgp_resource_fit_set, resources,
                            TOURNAMENT_SIZE, POP_SIZE - ELITE_SELECT__ELITE_CNT, RESOURCE_SELECT__FRAC,
                            RESOURCE_SELECT__MAX_BONUS, RESOURCE_SELECT__COST);
      });
      break;
    }
    case SELECTION_METHOD_ID__MAPELITES:
      std::cout << "Map-Elites selection not setup yet..." << std::endl;
      exit(-1);
      break;
    case SELECTION_METHOD_ID__ROULETTE: {
      do_selection_sig.AddAction([this]() {
        this->EliteSelect_MASK(*sgp_world, ELITE_SELECT__ELITE_CNT, 1);
        emp::RouletteSelect(*sgp_world, POP_SIZE - ELITE_SELECT__ELITE_CNT);
      });
      break;
    }
    default:
      std::cout << "Unrecognized selection method! Exiting..." << std::endl;
      exit(-1);
  }

  // ANALYSIS_TYPE ANALYSIS_TYPE_ID__DEBUGGING
  switch (RUN_MODE) {
    case RUN_ID__EXP: {
      // Setup run-mode agent advance signal response.
      agent_advance_sig.AddAction([this]() {
        sgp_eval_hw->SingleProcess();
      });
      // Setup run-mode begin turn signal response.
      begin_turn_sig.AddAction([this](const othello_t & game) {
        const player_t playerID = testcases[cur_testcase].GetInput().playerID;
        SGP__ResetHW();
        othello_dreamware->Reset(game);
        othello_dreamware->SetActiveDream(0);
        othello_dreamware->SetPlayerID(playerID);
      });
      // Setup non-verbose get move.
      get_eval_agent_move = [this]() {
        return (size_t)sgp_eval_hw->GetTrait(TRAIT_ID__MOVE);
      };

      break;
    }

    case RUN_ID__ANALYSIS: {
      switch (ANALYSIS_TYPE) {

        case ANALYSIS_TYPE_ID__DEBUGGING: {
          // Debugging analysis signal response.
          do_analysis_sig.AddAction([this]() { this->SGP__Debugging_Analysis(); });
          // Setup a verbose agent_advance_sig
          agent_advance_sig.AddAction([this]() {
            std::cout << "----- EVAL STEP: " << eval_time << " -----" << std::endl;
            sgp_eval_hw->SingleProcess();
            sgp_eval_hw->PrintState();
            std::cout << "--- DREAMBOARD STATE ---" << std::endl;
            othello_dreamware->GetActiveDreamOthello().Print();
          });
          // Setup a verbose begin_turn_sig response.
          begin_turn_sig.AddAction([this](const othello_t & game) {
            const player_t playerID = testcases[cur_testcase].GetInput().playerID;
            std::cout << "===============================================" << std::endl;
            std::cout << "TEST CASE: " << cur_testcase << std::endl;
            std::cout << "ID: " << testcases[cur_testcase].id << std::endl;
            std::cout << " ----- Input ----- " << std::endl;
            // Board
            testcases[cur_testcase].GetInput().game.Print();
            auto options = testcases[cur_testcase].GetInput().game.GetMoveOptions(testcases[cur_testcase].GetInput().playerID);
            std::cout << "Board width: " << testcases[cur_testcase].GetInput().game.GetBoardWidth() << std::endl;
            std::cout << "Round: " << testcases[cur_testcase].GetInput().round << std::endl;
            std::cout << "PlayerID: " << testcases[cur_testcase].GetInput().playerID << std::endl;
            std::cout << " ----- Output ----- " << std::endl;
            std::cout << "Expert move: " << testcases[cur_testcase].GetOutput().expert_move << std::endl;
            std::cout << "Board options: ";
            for (size_t j = 0; j < options.size(); ++j) {
              std::cout << " " << options[j];
            } std::cout << std::endl;
            std::cout << "Valid options: ";
            for (size_t j = 0; j < testcases[cur_testcase].GetOutput().move_valid.size(); ++j) {
              std::cout << " " << testcases[cur_testcase].GetOutput().move_valid[j];
            } std::cout << std::endl;

            SGP__ResetHW();
            othello_dreamware->Reset(game);
            othello_dreamware->SetActiveDream(0);
            othello_dreamware->SetPlayerID(playerID);
          });

          get_eval_agent_move = [this]() {
            size_t move = (size_t)sgp_eval_hw->GetTrait(TRAIT_ID__MOVE);
            std::cout << "SELECTED MOVE: " << move << std::endl;
            return move;
          };

          break;
        }
        default:
          std::cout << "Unrecognized analysis type. Exiting..." << std::endl;
          exit(-1);
      }
      break;
    }

    default:
      std::cout << "Unrecognized run mode! Exiting..." << std::endl;
      exit(-1);
  }

}

void LineageExp::ConfigAGP() {
  agp_world->Reset();
  agp_world->SetWellMixed(true);
  // NOTE: second argument specifies that we're not mutating the first thing in the pop (we're doing elite selection in all of our stuff).
  agp_world->SetMutFun([this](AvidaGPAgent &agent, emp::Random &rnd) { return this->AGP__Mutate(agent, rnd); }, ELITE_SELECT__ELITE_CNT);
  agp_world->SetFitFun([this](Agent &agent) { return this->CalcFitness(agent); });
  agp_world->OnGenotypeKnown([this](emp::Ptr<AGP__genotype_t> genotype, size_t pos) {
    genotype->GetData().RecordMutation(last_mutation);
  });

  ConfigAGP_InstLib();

  agp_eval_hw = emp::NewPtr<AGP__hardware_t>(agp_inst_lib);

  // Setup triggers!
  // Configure initial run setup
  switch (POP_INITIALIZATION_METHOD)
  {
  case POP_INITIALIZATION_METHOD_ID__RANDOM_POP:
    do_pop_init_sig.AddAction([this]() {
      this->AGP__InitPopulation_Random();
    });
    break;
  case POP_INITIALIZATION_METHOD_ID__ANCESTOR_FILE:
    do_pop_init_sig.AddAction([this]() {
      this->AGP__InitPopulation_FromAncestorFile();
    });
    break;
  default:
    std::cout << "Unrecognized population initialization method! Exiting..." << std::endl;
    exit(-1);
  }

  do_begin_run_setup_sig.AddAction([this]() {
    std::cout << "Doing initial run setup." << std::endl;
    // Setup systematics/fitness tracking.
    auto & sys_file = agp_world->SetupSystematicsFile(DATA_DIRECTORY + "systematics.csv");
    sys_file.SetTimingRepeat(SYSTEMATICS_INTERVAL);
    auto & fit_file = agp_world->SetupFitnessFile(DATA_DIRECTORY + "fitness.csv");
    fit_file.SetTimingRepeat(FITNESS_INTERVAL);
    emp::AddPhylodiversityFile(*agp_world, DATA_DIRECTORY + "phylodiversity.csv").SetTimingRepeat(SYSTEMATICS_INTERVAL);
    emp::AddLineageMutationFile(*agp_world, DATA_DIRECTORY + "lineage_mutations.csv", MUTATION_TYPES).SetTimingRepeat(SYSTEMATICS_INTERVAL);
    AddDominantFile(*agp_world, DATA_DIRECTORY + "dominant.csv", MUTATION_TYPES).SetTimingRepeat(SYSTEMATICS_INTERVAL);
    AddBestPhenotypeFile(*agp_world, DATA_DIRECTORY+"best_phenotype.csv").SetTimingRepeat(SYSTEMATICS_INTERVAL);
    // agp_muller_file = emp::AddMullerPlotFile(*agp_world, DATA_DIRECTORY + "muller_data.dat");
    // agp_world->OnUpdate([this](size_t ud){ if (ud % SYSTEMATICS_INTERVAL == 0) agp_muller_file.Update(); });
    record_fit_sig.AddAction([this](size_t pos, double fitness) { agp_world->GetGenotypeAt(pos)->GetData().RecordFitness(fitness); } );
    record_phen_sig.AddAction([this](size_t pos, const phenotype_t & phen) { agp_world->GetGenotypeAt(pos)->GetData().RecordPhenotype(phen); } );
    // Generate the initial population.
    do_pop_init_sig.Trigger();
  });

  // - Configure evaluation
  do_evaluation_sig.AddAction([this]() {
    double best_score = -32767;
    best_agent_id = 0;
    for (size_t id = 0; id < agp_world->GetSize(); ++id)
    {
      //std::cout << "Evaluating agent: " << id << std::endl;
      // Evaluate agent given by id.
      AvidaGPAgent &our_hero = agp_world->GetOrg(id);
      our_hero.SetID(id);
      agp_eval_hw->SetGenome(our_hero.GetGenome());
      this->Evaluate(our_hero);
      Phenotype & phen = agent_phen_cache[id];
      if (phen.aggregate_score > best_score) {
        best_score = phen.aggregate_score;
        best_agent_id = id;
      }
    }
    std::cout << "Update: " << update << " Max score: " << best_score << std::endl;
  });

  switch (SELECTION_METHOD)
  {
  case SELECTION_METHOD_ID__TOURNAMENT:
    do_selection_sig.AddAction([this]() {
      this->EliteSelect_MASK(*agp_world, ELITE_SELECT__ELITE_CNT, 1);
      emp::TournamentSelect(*agp_world, TOURNAMENT_SIZE, POP_SIZE - ELITE_SELECT__ELITE_CNT);
    });
    break;
  case SELECTION_METHOD_ID__LEXICASE: {
    agp_lexicase_fit_set.resize(0);
    for (size_t i = 0; i < testcases.GetSize(); ++i) {
      agp_lexicase_fit_set.push_back([i, this](AvidaGPAgent & agent) {
        return agent_phen_cache[agent.GetID()].testcase_scores[i];
      });
    }
    do_selection_sig.AddAction([this]() {
      this->EliteSelect_MASK(*agp_world, ELITE_SELECT__ELITE_CNT, 1);
      emp::LexicaseSelect(*agp_world, agp_lexicase_fit_set, POP_SIZE - ELITE_SELECT__ELITE_CNT);
    });
    break;
  }
  case SELECTION_METHOD_ID__ECOEA: {
    agp_resource_fit_set.resize(0);
    // Setup the fitness function set based on resource select mode.
    switch (RESOURCE_SELECT__MODE) {
      case RESOURCE_SELECT_MODE_ID__PHASES: {
        // Add a resource fit function for each game phase.
        for (size_t i = 0; i < testcases_by_phase.size(); ++i) {
          agp_resource_fit_set.push_back([i, this](AvidaGPAgent & agent) {
            double score = 0;
            emp::vector<size_t> & phasecases = testcases_by_phase[i];
            // Aggregate all relevant test scores.
            for (size_t j = 0; j < phasecases.size(); ++j) {
              score += agent_phen_cache[agent.GetID()].testcase_scores[phasecases[j]];
            }
            return score;
          });
        }
        break;
      }
      case RESOURCE_SELECT_MODE_ID__INDIV: {
        // Add a resource fit function for each testcase.
        for (size_t i = 0; i < testcases.GetSize(); ++i) {
          agp_resource_fit_set.push_back([i, this](AvidaGPAgent & agent) {
            return agent_phen_cache[agent.GetID()].testcase_scores[i];
          });
        }
        break;
      }
      default:
        std::cout << "Unrecognized resource select mode. Exiting..." << std::endl;
        exit(-1);
    }
    // Setup the do selection signal action.
    do_selection_sig.AddAction([this]() {
      this->EliteSelect_MASK(*agp_world, ELITE_SELECT__ELITE_CNT, 1);
      emp::ResourceSelect(*agp_world, agp_resource_fit_set, resources,
                          TOURNAMENT_SIZE, POP_SIZE - ELITE_SELECT__ELITE_CNT, RESOURCE_SELECT__FRAC,
                          RESOURCE_SELECT__MAX_BONUS, RESOURCE_SELECT__COST);
    });
    break;
  }
  case SELECTION_METHOD_ID__MAPELITES:
    std::cout << "Map-Elites selection not setup yet..." << std::endl;
    exit(-1);
    break;
  case SELECTION_METHOD_ID__ROULETTE: {
    do_selection_sig.AddAction([this]() {
      this->EliteSelect_MASK(*agp_world, ELITE_SELECT__ELITE_CNT, 1);
      emp::RouletteSelect(*agp_world, POP_SIZE - ELITE_SELECT__ELITE_CNT);
    });
    break;
  }
  default:
    std::cout << "Unrecognized selection method! Exiting..." << std::endl;
    exit(-1);
  }

  // - Configure world upate.
  do_world_update_sig.AddAction([this]() { agp_world->Update(); });

  // do_pop_snapshot
  do_pop_snapshot_sig.AddAction([this](size_t update) { this->AGP_Snapshot_SingleFile(update); });

   // ANALYSIS_TYPE ANALYSIS_TYPE_ID__DEBUGGING
  switch (RUN_MODE) {

    case RUN_ID__EXP: {
      // Setup run-mode agent advance signal response.
      agent_advance_sig.AddAction([this]() {
        agp_eval_hw->SingleProcess();
      });
      // Setup run-mode begin turn signal response.
      begin_turn_sig.AddAction([this](const othello_t & game) {
        const player_t playerID = testcases[cur_testcase].GetInput().playerID;
        AGP__ResetHW();
        othello_dreamware->Reset(game);
        othello_dreamware->SetActiveDream(0);
        othello_dreamware->SetPlayerID(playerID);
      });
      // Setup non-verbose get move.
      get_eval_agent_move = [this]() {
        return (size_t)agp_eval_hw->GetTrait(TRAIT_ID__MOVE);
      };

      break;
    }

    case RUN_ID__ANALYSIS: {
      std::cout << "Unimplemented analysis type. Exiting..." << std::endl;
      exit(-1);
    }

    default:
      std::cout << "Unrecognized run mode! Exiting..." << std::endl;
      exit(-1);
  }

  get_eval_agent_done = [this]() {
    return (bool)agp_eval_hw->GetTrait(TRAIT_ID__DONE);
  };

  get_eval_agent_playerID = [this]() {
    return othello_dreamware->GetPlayerID();
  };
}

#endif
