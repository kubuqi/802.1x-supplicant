#*  802.1x makefile
#
#	$Id: Makefile,v 1.19 2002/09/29 10:36:21 rw Exp $
#

#**  macros
#***  system dependant
#
SYSTEM := $(shell uname)

#***  pathnames
#
BASE := $(shell pwd)
BIN := $(BASE)/bin
INCLUDE := -I$(BASE)/include
SRC := $(BASE)/src
OUT := $(BASE)/tmp

ifeq ($(SYSTEM), FreeBSD)
DEFINES := -DFreeBSD -DUSE_BPF
endif

ifeq ($(SYSTEM), Linux)
DEFINES := -DUSE_PF_PACKET -DLinux
endif

#***  programs
#
GCC := g++
CC :=	$(GCC) -Wall -Wno-reorder -Wno-switch -pipe -c $(INCLUDE) $(DEFINES)

LD := $(GCC) $(LIB)
DEP := $(GCC) -MM $(INCLUDE) $(DEFINES)

#***  flags
#
ifdef FINAL
  CCFLAGS := -s -O2
  LDFLAGS := -s
  OEXT := .o2
else
  ifdef PROF
    CCFLAGS := -g -pg -O2
    LDFLAGS := -g -pg
    OEXT := .op
  else
    CCFLAGS := -g
    LDFLAGS := -g
    OEXT := .o
  endif
endif

#***  files
#
SOURCES := $(notdir $(wildcard $(SRC)/*.cc))
DEPS := $(addprefix $(OUT)/, $(SOURCES:.cc=.d))
OBJECTS := $(addprefix $(OUT)/, $(SOURCES:.cc=$(OEXT)))

#**  vpaths
#
vpath %.h $(INCLUDE)
vpath %.cc $(SRC)
vpath %$(OEXT) $(OUT)
vpath %.d $(OUT)

#**  targets
#
.PHONY: clean real-clean lib

all: $(BIN)/eapd

clean:
	-@rm -f $(OUT)/*.[o]*
	-@rm -f $(BIN)/*

real-clean: clean
	-@rm -f $(OUT)/*.d

$(BIN)/eapd: $(OBJECTS)

#**  dependencies
#
include $(DEPS)

#**  %-rules
#
$(OUT)/%.d: %.cc
	$(DEP) $< | perl -pe 's|$*\.o|$@|;' >$@


$(OUT)/%$(OEXT): %.cc %.d
	$(CC) $(CCFLAGS) -o $@ $<

$(BIN)/%.a:
	$(AR) src $@ $^

$(BIN)/%:
	$(LD) $(LDFLAGS) -o $@ $^ -lcrypto -lssl
