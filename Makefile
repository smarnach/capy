CXX = g++
CXXFLAGS = -I /usr/include/python2.7 -fPIC -Wall -ggdb
LDFLAGS = -shared
LDLIBS = -lpython2.7

samplesim.so: samplesim.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

samplesim.o: capy.hh types.hh exceptions.hh api.hh array.hh
