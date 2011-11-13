CXX = g++
CXXFLAGS = -I /usr/include/python2.7 -fPIC -Wall -ggdb
LDFLAGS = -shared
LDLIBS = -lpython2.7

samplesim.so: capy.o samplesim.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

capy.o samplesim.o: capy.hh types.hh exceptions.hh
