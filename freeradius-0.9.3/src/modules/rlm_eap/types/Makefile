TARGET_LIBS = static dynamic

all:
	$(MAKE) $(MFLAGS) WHAT_TO_MAKE=$@ common

static:
	$(MAKE) $(MFLAGS) WHAT_TO_MAKE=$@ common

dynamic:
	$(MAKE) $(MFLAGS) WHAT_TO_MAKE=$@ common

clean:
	$(MAKE) $(MFLAGS) WHAT_TO_MAKE=$@ common

install:
	$(MAKE) $(MFLAGS) WHAT_TO_MAKE=$@ common

common: 
	@for mod in rlm_eap*; do \
		what=$(WHAT_TO_MAKE); \
		[ "$$what" = "all" ] && what="$(TARGET_LIBS)"; \
		if [ -d $$mod ] && [ -f $$mod/Makefile ]; then \
			echo "Making $$what in $$mod..."; \
			(cd $$mod && $(MAKE) $(MFLAGS) $$what) || exit 1; \
		fi; \
	done

