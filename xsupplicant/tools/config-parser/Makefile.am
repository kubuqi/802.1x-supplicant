AUTOMAKE_OPTIONS=foreign no-dependencies

#CORELIBS = @CORELIBS@
#MORELIBS = @MORELIBS@
#MOREDEFS = @MOREDEFS@
INCLUDES = @INCS@ -I../../src/
DEFS = @DEFS@ $(MOREDEFS)

CLEANFILES= config_grammar.c config_lexicon.c config_grammar.h

bin_PROGRAMS = config-parser
config_parser_SOURCES = config-parser.c \
		../../src/xsup_debug.c \
	 	../../src/config.c ../../src/config.h \
		../../src/config_grammar.y ../../src/config_lexicon.l
config_parser_LDADD = $(MORELIBS)
