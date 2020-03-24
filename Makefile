
CXX_COMPILER = g++ -fopenmp

INCLUDE_DIRS =   -I./include  

#  add include files from metis library
# jcamata
#INCLUDE_DIRS += -I$(HOME)/local/metis/include

# guilherme
INCLUDE_DIRS += -I/usr/local/include


#LDFLAGS      = -L$(HOME)/local/metis/lib 
LDFLAGS +=    -lmetis


LDFLAGS      = -L/usr/local/lib -lmetis

CXX_FLAGS    = -DDEBUG -g $(INCLUDE_DIRS)


# source files
srcfiles        := $(wildcard *.cpp) $(wildcard src/*.cpp)
objects         := $(patsubst %.cpp, %.o, $(srcfiles))


meshtools: $(objects)
	$(CXX_COMPILER) -o meshtools $(CXX_FLAGS) $(objects) $(LDFLAGS)


move:
	mv msh/*.vtu /mnt/c/Users/macha/Desktop

clean:
	rm $(objects) msh/*.vtu
#
# How to compile C++
#
%.o : %.cpp
	@echo "Compiling C++ "$<"..."
	$(CXX_COMPILER) $(CXX_FLAGS) -c $< -o $@




