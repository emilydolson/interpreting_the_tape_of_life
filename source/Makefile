# Project-specific settings
EXPERIMENT := optimization_problems

EMP_DIR := ../../Empirical/source
CEC2013_DIR := CEC2013

# Flags to use regardless of compiler
CFLAGS_all := -Wall -Wno-unused-function -std=c++14 -I$(EMP_DIR)/ -I$(CEC2013_DIR)/

# Native compiler information
CXX_nat := g++
CFLAGS_nat := -O3 -DNDEBUG $(CFLAGS_all)
CFLAGS_nat_debug := -g $(CFLAGS_all)

# Emscripten compiler information
CXX_web := emcc
OFLAGS_web_all := -s TOTAL_MEMORY=67108864 --js-library $(EMP_DIR)/web/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=1 #--embed-file configs
OFLAGS_web := -Oz -DNDEBUG
OFLAGS_web_debug := -g4 -Oz -pedantic -Wno-dollar-in-identifier-extension

CFLAGS_web := $(CFLAGS_all) $(OFLAGS_web) $(OFLAGS_web_all)
CFLAGS_web_debug := $(CFLAGS_all) $(OFLAGS_web_debug) $(OFLAGS_web_all)

experiment: $(EXPERIMENT)
default: experiment


debug:	CFLAGS_nat := $(CFLAGS_nat_debug)
debug:	$(EXPERIMENT)

cec2013.o: $(CEC2013_DIR)/cec2013.h $(CEC2013_DIR)/cec2013.cpp $(CEC2013_DIR)/cfunction.h $(CEC2013_DIR)/cfunction.cpp
	$(CXX_nat) $(CFLAGS_nat) -c $(CEC2013_DIR)/cec2013.cpp
cfunction.o: $(CEC2013_DIR)/cfunction.h $(CEC2013_DIR)/cfunction.cpp
	$(CXX_nat) $(CFLAGS_nat) -c $(CEC2013_DIR)/cfunction.cpp
rand2.o: $(CEC2013_DIR)/rand2.h $(CEC2013_DIR)/rand2.c
	$(CXX_nat) $(CFLAGS_nat) -c $(CEC2013_DIR)/rand2.c

$(EXPERIMENT):	cec2013.o cfunction.o rand2.o $(EXPERIMENT).cc
	$(CXX_nat) $(CFLAGS_nat) $(EXPERIMENT).cc cec2013.o cfunction.o rand2.o -o $(EXPERIMENT)



clean:
	rm -f $(EXPERIMENT) *~ *.o

# Debugging information
print-%: ; @echo '$(subst ','\'',$*=$($*))'
