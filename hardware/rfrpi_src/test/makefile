CXXFLAGS += -O2 -Wall -g -pthread

all: test

test:  test.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ 

clean:
	$(RM) *.o test
