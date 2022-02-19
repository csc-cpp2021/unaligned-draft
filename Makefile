CXX=g++
CXXFLAGS=-fsanitize=alignment -Wall -Wextra -std=c++2a

lvl0: unaligned.cpp
	$(CXX) $(CXXFLAGS) $< -o unaligned -DTEST_LEVEL=0

lvl1: unaligned.cpp
	$(CXX) $(CXXFLAGS) $< -o unaligned -DTEST_LEVEL=1

lvl2: unaligned.cpp
	$(CXX) $(CXXFLAGS) $< -o unaligned -DTEST_LEVEL=2

lvl3: unaligned.cpp
	$(CXX) $(CXXFLAGS) $< -o unaligned -DTEST_LEVEL=3