include $(HCLIB_ROOT)/../modules/transitive_joins/inc/hclib_transitive_joins.pre.mak
include $(HCLIB_ROOT)/include/hclib.mak
include $(HCLIB_ROOT)/../modules/transitive_joins/inc/hclib_transitive_joins.post.mak

TARGETS=TestExampleProgram

all: $(TARGETS)

%: %.cpp
	$(CXX) -g -O3 -std=c++14 $(HCLIB_CFLAGS) $(HCLIB_LDFLAGS) -o $@.out $^ $(HCLIB_LDLIBS) -lhclib_transitive_joins

clean:
	rm -f *.out