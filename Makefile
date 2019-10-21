
CXX_COMPILER = g++ -DDEBUG_

INCLUDE_DIRS = -I./include 

CXX_FLAGS    = -g $(INCLUDE_DIRS)
LDFLAGS      =

# source files
srcfiles        := $(wildcard *.cpp) $(wildcard src/*.cpp)
objects         := $(patsubst %.cpp, %.o, $(srcfiles))

meshtools: $(objects)
	$(CXX_COMPILER) -o meshtools $(CXX_FLAGS) $(objects) $(LDFLAGS)



clean:
	rm $(objects)
#
# How to compile C++
#
%.o : %.cpp
	@echo "Compiling C++ "$<"..."
	$(CXX_COMPILER) $(CXX_FLAGS) -c $< -o $@




