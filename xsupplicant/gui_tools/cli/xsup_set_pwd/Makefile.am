AUTOMAKE_OPTIONS=foreign no-dependencies

CORELIBS = @CORELIBS@
MORELIBS = @MORELIBS@
MOREDEFS = @MOREDEFS@
INCLUDES = @INCS@
DEFS = @DEFS@ $(MOREDEFS)

bin_PROGRAMS = xsup_set_pwd
xsup_set_pwd_SOURCES = xsup_set_pwd.c ../../common/gui_interface.c

xsup_set_pwd_LDADD = $(CORELIBS) $(MORELIBS)
