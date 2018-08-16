include $(TOP)/Makefile.common
CMP = $(TOP)/assembly/cmp
CFLAGS += -I$(CMP)
CFLAGS := $(filter-out -std=c99,$(CFLAGS))
