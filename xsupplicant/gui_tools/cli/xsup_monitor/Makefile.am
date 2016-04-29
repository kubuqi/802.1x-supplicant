AUTOMAKE_OPTIONS=foreign no-dependencies

CORELIBS = @CORELIBS@
MORELIBS = @MORELIBS@
MOREDEFS = @MOREDEFS@
INCLUDES = @INCS@
DEFS = @DEFS@ $(MOREDEFS)

bin_PROGRAMS = xsup_monitor
xsup_monitor_SOURCES = xsup_monitor.c ../../common/gui_interface.c

xsup_monitor_LDADD = $(CORELIBS) $(MORELIBS)
