// user communication
//

//*  includes
//
#include "stdafx.h"

#include "buffer.hh"
#include "comm.hh"

namespace eap 
{
    extern "C" {
#include <errno.h>	
#include <stdarg.h>	
#include <stdio.h>
#include <string.h>
#include <stdlib.h>	

#ifdef WIN32
#	include <io.h>
#	define vsnprintf		_vsnprintf
#	define STDERR_FILENO	(int)stderr
#else
#	include <unistd.h>
#	include <syslog.h>	
#endif
    }

    //*  functions
    //
    static char *find_m(char *s)
    {
	while (1) {
	    switch (*s) {
	    case 0:
		goto none;

	    case '%':
		switch (s[1]) {
		case 0:
		    return 0;

		case 'm':
		    return s;
		}
	    }

	    ++s;
	}

    none:
	return 0;
    }

    static char *find_marker(char *s)
    {
	while (1) {
	    switch (*s) {
	    case 0:
		goto none;

	    case '\001':
		switch(s[1]) {
		case 0:
		    return 0;

		case '\002':
		    return s;
		}
	    }

	    ++s;
	}

    none:
	return 0;
    }

    //*  variables
    //
    int out::debug_level = out::d_notice;
    out *out::sink;

    //*  methods
    //**  out
    //
    void out::notice(char const *fmt, ...)
    {
	va_list va;

	if (debug_level >= d_notice) {
	    va_start(va, fmt);
	    sink->do_print(fmt, va);
	    va_end(va);
	}
    }
	
    void out::warn(char const *fmt, ...)
    {
	if (debug_level >= d_warn) { 
	    va_list va; 
	    va_start(va, fmt); 
	    sink->do_print(fmt, va); 
	    va_end(va); 
	} 
    }
    
    void out::info(char const *fmt, ...)
    {
	if (debug_level >= d_info) { 
	    va_list va; 
	    va_start(va, fmt); 
	    sink->do_print(fmt, va); 
	    va_end(va); 
	} 
    }
	
    void out::debug(char const *fmt, ...)
    {
	if (debug_level >= d_debug) { 
	    va_list va; 
	    va_start(va, fmt); 
	    sink->do_print(fmt, va); 
	    va_end(va); 
	} 
    }
    
    void out::do_print(char const *fmt, va_list va)
    {
		return;	// zl temp
	char *pm;
	int rc;
	size_t need, elen;
	buffer f(need = strlen(fmt) + 1);
	
	memcpy(f, fmt, need);

	pm = find_m(f);
	if (pm) {
	    *pm = '\001';
	    pm[1] = '\002';
	}

	while (1) {
	    rc = vsnprintf((char *)fmt_buf, fmt_buf.buf_size(), (char *)f, va);
	    if ((unsigned)rc < fmt_buf.buf_size()) break;
	    
	    fmt_buf.expand();
	}
	
	if (pm) {
	    pm = find_marker(fmt_buf);
	    
	    need = elen = strlen(strerror(errno));
	    need += rc;
	    need -= 2;
	    while (fmt_buf.buf_size() < need) fmt_buf.expand();
	    memmove(pm + elen, pm + 2, rc - 2 - (pm - fmt_buf));
	    memcpy(pm, strerror(errno), elen);

	    len = need;
	} else len = rc;
	
	flush();
    }

    //**  printer
    //
    void printer::make()
    {
	if (sink) delete sink;
	sink = new printer;
    }

    void printer::flush()
    {
	if (!remain()) fmt_buf.expand();

	// work around for MSVC, change from : 	fmt_buf[len] = '\n';
	(ui8 &)(fmt_buf.operator [](len)) = 0;

#ifdef WIN32
	printf(fmt_buf);
#else
	write(STDERR_FILENO, fmt_buf, len + 1);
#endif

	len = 0;
    }

    //**  logger
    //
    void logger::make(char const *name, int fac)
    {
	if (sink) delete sink;
	sink = new logger;

#ifndef WIN32
	closelog();
	openlog(name, LOG_PID | LOG_NDELAY, fac);
#endif
    }

    void logger::flush()
    {
	if (!remain()) fmt_buf.expand();

	// work around for MSVC, changed from :	fmt_buf[len] = 0;
	(ui8 &)(fmt_buf.operator [](len)) = 0;

#ifdef WIN32
	printf("%s", (char *)fmt_buf);
#else
	syslog(LOG_INFO, "%s", (char *)fmt_buf);
#endif
	len = 0;
	}
}
