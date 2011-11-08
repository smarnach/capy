CXX = g++
CXXFLAGS = -I /usr/include/python2.7 -fPIC -Wall
LDFLAGS = -shared
LDLIBS = -lpython2.7

samplesim.so: simwrap.o samplesim.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

simwrap.o samplesim.o: simwrap.hh
