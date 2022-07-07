# Build targets
	
USER_FLAGS=-Wno-unused-result -Wno-unused-but-set-variable -Wno-sign-compare -Wall 
USER_LIBS=

# Compiler flags
CXX = g++ -std=c++11 
#CXXFLAGS=$(USER_FLAGS) -O3 -fPIC -g  -I ./include  
CXXFLAGS=$(USER_FLAGS) -O3 -I ./include 
LDFLAGS=-lpthread


# The name of the excution that will be built
target_swing=bin/swing
target_swing_1st_order=bin/swing_1st_order
target_swing_symmetric=bin/swing_symmetric

all: $(target_swing) $(target_swing_1st_order) $(target_swing_symmetric) 


$(target_swing): src/swing.cc utils/utils.cc  include/*
	$(CXX) -w $(LDFLAGS) -o $(target_swing) src/swing.cc utils/utils.cc $(CXXFLAGS)

$(target_swing_1st_order):  src/swing_1st_order.cc utils/utils.cc  include/*
	$(CXX) -w $(LDFLAGS) -o $(target_swing_1st_order) src/swing_1st_order.cc utils/utils.cc $(CXXFLAGS)

$(target_swing_symmetric): src/swing_symmetric.cc utils/utils.cc include/*
	$(CXX) -w $(LDFLAGS) -o $(target_swing_symmetric) src/swing_symmetric.cc utils/utils.cc $(CXXFLAGS)


clean:
	rm -f $(target_swing) $(target_swing_1st_order) $(target_swing_symmetric) 
	find . -name '*.o' -print | xargs rm -f

