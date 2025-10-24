CXX=g++
CXXFLAGS=-std=c++17 -O2 -Wall -Wextra -pedantic

ALL=01_fcfs 02_sjf 03_srtf 04_priority_np 05_rr 06_mlq 07_mlfq 08_lottery 09_cfs 10_edf

all: $(ALL)

%: %.cpp common.hpp
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -f $(ALL)
