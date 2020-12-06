include $(HCLIB_ROOT)/include/hclib.mak

SOURCE_FOLDER=./src
OBJ_FOLDER=./obj

FLAGS=-std=c++11 -Wall -fPIC -Iinc

SRC_OBJ:=$(patsubst $(SOURCE_FOLDER)/%.cpp, %.o, $(wildcard $(SOURCE_FOLDER)/*.cpp))
TARGETS=$(SRC_OBJ) lib/libhclib_transitive_joins.so

all: FLAGS += -O3
all: $(TARGETS)

test: FLAGS += -g -O0 -DLOG -DENABLE_TASK_TREE -DENABLE_FUTURE_LCA -DENABLE_PROMISE_LCA -DENABLE_PROMISE_FULFILLMENT_CHECK
test: $(TARGETS)

withTaskNodes: FLAGS += -O3 -DENABLE_TASK_TREE
withTaskNodes: $(TARGETS)

withLCA: FLAGS += -O3 -DENABLE_TASK_TREE -DENABLE_FUTURE_LCA -DENABLE_PROMISE_LCA
withLCA: $(TARGETS)

withAll: FLAGS += -O3 -DENABLE_TASK_TREE -DENABLE_FUTURE_LCA -DENABLE_PROMISE_LCA -DENABLE_PROMISE_FULFILLMENT_CHECK
withAll: $(TARGETS)

lib/libhclib_transitive_joins.so: $(SRC_OBJ)
	$(CXX) -shared -fPIC $(HCLIB_LDLIBS) $(HCLIB_LDFLAGS) -o $@ $(OBJ_FOLDER)/*.o

clean:
	rm -f obj/*.o lib/*

install:
	cp lib/*.so $(HCLIB_ROOT)/lib

.PHONY: all clean

$(SRC_OBJ): %.o: $(SOURCE_FOLDER)/%.cpp
	$(CXX) $(FLAGS) $(HCLIB_CFLAGS) $(HCLIB_LDFLAGS) -c $< -o $(OBJ_FOLDER)/$@ $(HCLIB_LDLIBS)
