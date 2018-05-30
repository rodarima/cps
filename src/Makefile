CXX = g++
CXXFLAGS += -g

# Gecode options

# Modify the prefix to suit your instalation
GECODE_PREFIX = /usr

CXXFLAGS += -std=c++11 -I${GECODE_PREFIX}/include
LDFLAGS += -L${GECODE_PREFIX}/lib
LDLIBS += -lgecodedriver -lgecodesearch -lgecodeminimodel \
       -lgecodeint -lgecodekernel -lgecodesupport

# CPLEX options

CXXFLAGS += -DIL_STD
LDFLAGS += -L./lib
LDLIBS += -lm -lpthread -lilocplex -lconcert -lcplex

# Binaries

BINS=plot checker lp cp

all: ${BINS}

clean:
	rm ${BINS}
