#
# Makefile
#
#		NOTE: This top-level Makefile must not
#		use GNU-make extensions. The lower ones can.
#
# Version:	$Id: Makefile,v 1.41 2003/06/27 15:04:15 aland Exp $
#

include Make.inc

.PHONY: all clean install

SUBDIRS		= ${LIBLTDLPATH} src raddb scripts doc
WHAT_TO_MAKE	= all

all:
	@$(MAKE) $(MFLAGS) WHAT_TO_MAKE=$@ common

clean:
	@$(MAKE) $(MFLAGS) WHAT_TO_MAKE=$@ common
	@rm -f *~

install:
	$(INSTALL) -d -m 755	$(R)$(sbindir)
	$(INSTALL) -d -m 755	$(R)$(bindir)
	$(INSTALL) -d -m 755	$(R)$(raddbdir)
	$(INSTALL) -d -m 755	$(R)$(mandir)
	$(INSTALL) -d -m 755	$(R)$(RUNDIR)
	$(INSTALL) -d -m 700	$(R)$(logdir)
	$(INSTALL) -d -m 700	$(R)$(radacctdir)
	$(INSTALL) -d -m 700	$(R)$(datadir)
	$(INSTALL) -d -m 755	$(R)$(dictdir)
	@$(MAKE) $(MFLAGS) WHAT_TO_MAKE=$@ common
	for i in 1 5 8; do \
		$(INSTALL) -d -m 755	$(R)$(mandir)/man$$i; \
		for p in man/man$$i/*.$$i; do \
			$(INSTALL) -m 644 $$p $(R)$(mandir)/man$$i; \
		done \
	done
	@echo "Installing dictionary files in $(R)$(dictdir)"; \
	cd share; \
	for i in dictionary*; do \
		$(INSTALL) -m 644 $$i $(R)$(dictdir); \
	done; \

common:
	@for dir in $(SUBDIRS); do \
		echo "Making $(WHAT_TO_MAKE) in $$dir..."; \
		(cd $$dir && $(MAKE) $(MFLAGS) $(WHAT_TO_MAKE)) || exit 1;\
	done

distclean: clean
	rm -f config.cache config.log config.status libtool \
		src/include/radpaths.h src/include/stamp-h \
		libltdl/config.log libltdl/config.status \
		libltdl/libtool
	-find . ! -name configure.in -name \*.in -print | \
		sed 's/\.in$$//' | \
		while read file; do rm -f $$file; done
	-find src/modules -name config.mak | \
		while read file; do rm -f $$file; done
	-find src/modules -name config.h | \
		while read file; do rm -f $$file; done

######################################################################
#
#  Automatic remaking rules suggested by info:autoconf#Automatic_Remaking
#
######################################################################
reconfig: configure src/include/autoconf.h.in

configure: configure.in aclocal.m4
	$(AUTOCONF)

# autoheader might not change autoconf.h.in, so touch a stamp file
src/include/autoconf.h.in: src/include/stamp-h.in acconfig.h
src/include/stamp-h.in: configure.in acconfig.h
	$(AUTOHEADER)
	echo timestamp > src/include/stamp-h.in

src/include/autoconf.h: src/include/stamp-h
src/include/stamp-h: src/include/autoconf.h.in config.status
	./config.status

config.status: configure
	./config.status --recheck

configure.in:
