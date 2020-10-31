# Post Makefile includes are the main part of a module's build system, allowing
# it to add flags to the overall project compile and link flags.
HCLIB_CFLAGS += -I$(HCLIB_ROOT)/../modules/transitive_joins/inc
HCLIB_LDFLAGS += -L$(HCLIB_ROOT)/../modules/baletransitive_joins_actor/lib