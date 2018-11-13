#ifndef TEST_CASE_SET_H
#define TEST_CASE_SET_H

// This code was pulled and modified from: https://github.com/emilydolson/ecology_of_evolutionary_computation/blob/master/source/TestcaseSet.h
// - Original version by Emily Dolson

#include <iostream>
#include <fstream>
#include <set>
#include <algorithm>
#include <functional>

#include "base/array.h"
#include "base/vector.h"
#include "tools/string_utils.h"
#include "tools/Random.h"
#include "tools/random_utils.h"


template <typename INPUT_TYPE, typename OUTPUT_TYPE>
class TestcaseSet {
public:
  using input_t = INPUT_TYPE;
  using output_t = OUTPUT_TYPE;

  struct Testcase {
    input_t input;
    output_t output;
    size_t id;

    Testcase(const input_t & _in, const output_t & _out) : input(_in), output(_out), id(0) { ; }
    Testcase(const Testcase & other) : input(other.input), output(other.output), id(other.id) { ; }

    input_t & GetInput() { return input; }
    output_t & GetOutput() { return output; }
    
  };

  using test_case_t = Testcase;
  using reader_fun_t = std::function<test_case_t(emp::vector<std::string> &)>; ///< Pass vector as reference: registered functions are free to manipulate given vector.

protected:
    emp::vector<test_case_t> test_cases;
    reader_fun_t input_test_case;

public:
    TestcaseSet(const reader_fun_t & reader, std::string filename) {
      RegisterTestcaseReader(reader);
      LoadTestcases(filename);
    }

    TestcaseSet()
      : test_cases(), input_test_case()
    { ; }

    emp::vector<test_case_t> & GetTestcases() {
        return test_cases;
    }

    test_case_t & operator[](size_t id) {
      emp_assert(id < test_cases.size());
      return test_cases[id];
    }

    emp::vector<size_t> GetSubset(int trials, emp::Random * random) {
        return emp::Choose(*random, test_cases.size(), trials);
    }

    size_t GetSize() const { return test_cases.size(); }

    // Probably want a better name for this...
    /// Reader function should: given a vector of strings to represent testcase input and a
    /// vector of strings to represent output, return a Testcase to be added to the test case set.
    void RegisterTestcaseReader(const reader_fun_t & reader) { input_test_case = reader; }

    void AddTestcase(const test_case_t & test) {
      const size_t id = test_cases.size();
      test_cases.push_back(test);
      test_cases.back().id = id;
    }

    void AddTestcase(const input_t & input, const output_t & output) {
      const size_t id = test_cases.size();
      test_cases.emplace_back(input, output);
      test_cases[id] = id;
    }

    /// Load test cases from csv file. Each line is expected to contain a full test case.
    void LoadTestcases(std::string filename) {
      std::ifstream infile(filename);
      std::string line;
      if (!infile.is_open()){
        std::cout << "ERROR: " << filename << " did not open correctly" << std::endl;
        return;
      }
      // Ignore header
      getline(infile, line);
      while (getline(infile,line)) {
        emp::vector<std::string> split_line = emp::slice(line, ',');
        AddTestcase(input_test_case(split_line));
        // std::cout << emp::to_string(test_case) << " " << answer << std::endl;
      }
      infile.close();
  }

};

#endif
