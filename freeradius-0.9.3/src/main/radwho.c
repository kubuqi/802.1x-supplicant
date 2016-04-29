/*
 * radwho.c	Show who is logged in on the terminal servers.
 *		Can also be installed as fingerd on the UNIX
 *		machine RADIUS runs on.
 *
 * Version:	$Id: radwho.c,v 1.42 2002/10/30 20:38:18 aland Exp $
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Copyright 2000  The FreeRADIUS server project
 * Copyright 2000  Alan DeKok <aland@ox.org>
 */

static const char rcsid[] =
"$Id: radwho.c,v 1.42 2002/10/30 20:38:18 aland Exp $";

#include "autoconf.h"
#include "libradius.h"

#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <ctype.h>
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#include "sysutmp.h"
#include "radutmp.h"
#include "radiusd.h"
#include "conffile.h"

/*
 *	FIXME: put in header file.
 */
#define SYS_FINGER "/usr/bin/finger"
#define FINGER_DIR "/usr/local/lib/finger"

/*
 *	Header above output and format.
 */
static const char *hdr1 = 
"Login      Name              What  TTY  When      From      Location";
static const char *ufmt1 = "%-10.10s %-17.17s %-5.5s %-4.4s %-9.9s %-9.9s %-.16s%s";
static const char *ufmt1r = "%s,%s,%s,%s,%s,%s,%s%s";
static const char *rfmt1 = "%-10.10s %-17.17s %-5.5s %s%-3d %-9.9s %-9.9s %-.19s%s";
static const char *rfmt1r = "%s,%s,%s,%s%d,%s,%s,%s%s";

static const char *hdr2 = 
"Login      Port    What      When          From       Location";
static const char *ufmt2 = "%-10.10s %-6.6d %-7.7s %-13.13s %-10.10s %-.16s%s";
static const char *ufmt2r = "%s,%d,%s,%s,%s,%s%s";
static const char *rfmt2 = "%-10.10s %s%-5d  %-6.6s %-13.13s %-10.10s %-.28s%s";
static const char *rfmt2r = "%s,%s%d,%s,%s,%s,%s%s";

static const char *eol = "\n";
static int showname = -1;
static int showptype = 0;
static int showcid = 0;
int debug_flag = 0;
const char *progname = "radwho";
const char *radlog_dir = NULL;
radlog_dest_t radlog_dest = RADLOG_FILES;
const char *radutmp_file = NULL;

const char *radius_dir = NULL;
const char *radacct_dir = NULL;
const char *radlib_dir = NULL;
int auth_port = 0;
int acct_port;
uint32_t myip = INADDR_ANY;
int log_stripped_names;

struct radutmp_config_t {
  char *radutmp_fn;
} radutmpconfig;

static CONF_PARSER module_config[] = {
  { "filename", PW_TYPE_STRING_PTR, 0, &radutmpconfig.radutmp_fn,  RADUTMP },
  { NULL, -1, 0, NULL, NULL }
};

/*
 *	Safe popen. Ugh.
 */
static FILE *safe_popen(const char *cmd, const char *mode)
{
	char		*p;
	char		buf[1024];

	/*
	 *	Change all suspect characters into a space.
	 */
	strncpy(buf, cmd, sizeof(buf));
	buf[sizeof(buf) - 1] = 0;
	for (p = buf; *p; p++) {
		if (isalnum((int) *p))
			continue;
		if (strchr("@%-_ \t+:,./", *p) == NULL)
			*p = ' ';
	}

	return popen(buf, mode);
}

/*
 *	Print a file from FINGER_DIR. If the file is executable,
 *	execute it instead. Return 0 if succesfull.
 */
static int ffile(const char *arg)
{
	FILE *fp;
	char fn[1024];
	int p = 0;
	char *s;

	snprintf(fn, sizeof(fn), "%s/%.32s", FINGER_DIR, arg);
	if (access(fn, X_OK) == 0) {
		p = 1;
		snprintf(fn, sizeof(fn), "exec %s/%.32s 2>&1", FINGER_DIR, arg);
		fp = safe_popen(fn, "r");
	} else fp = fopen(fn, "r");

	if (fp == NULL) 
		return -1;

	while(fgets(fn, 1024, fp)) {
		if ((s = strchr(fn, '\n')) != NULL)
			*s = 0;
		fprintf(stdout, "%s\r\n", fn);
	}
	if (p)
		pclose(fp);
	else
		fclose(fp);
	fflush(stdout);
	return 0;
}


/*
 *	Execute the system finger and translate LF to CRLF.
 */
static void sys_finger(const char *l)
{
	FILE *fp;
	char fn[1024];
	char *p;

	if (ffile(l) == 0)
		exit(0);

	snprintf(fn, sizeof(fn), "exec %s %s", SYS_FINGER, l);
	if ((fp = safe_popen(fn, "r")) == NULL) {
		printf("popen: %s\r\n", strerror(errno));
		exit(1);
	}

	while(fgets(fn, 1024, fp)) {
		if ((p = strchr(fn, '\n')) != NULL)
			*p = 0;
		fprintf(stdout, "%s\r\n", fn);
	}
	pclose(fp);
	exit(0);
}


/*
 *	Get fullname of a user.
 */
static char *fullname(char *username)
{
	struct passwd *pwd;
	char *s;

	if ((pwd = getpwnam(username)) != NULL) {
		if ((s = strchr(pwd->pw_gecos, ',')) != NULL) *s = 0;
		return pwd->pw_gecos;
	}
	return username;
}

/*
 *	Return protocol type.
 */
static const char *proto(int id, int porttype)
{
	static char buf[8];

	if (showptype) {
		if (!strchr("ASITX", porttype))
			porttype = ' ';
		if (id == 'S')
			snprintf(buf, sizeof(buf), "SLP %c", porttype);
		else if (id == 'P')
			snprintf(buf, sizeof(buf), "PPP %c", porttype);
		else
			snprintf(buf, sizeof(buf), "shl %c", porttype);
		return buf;
	}
	if (id == 'S') return "SLIP";
	if (id == 'P') return "PPP";
	return "shell";
}

/*
 *	Return a time in the form day hh:mm
 */
static char *dotime(time_t t)
{
	char *s = ctime(&t);

	if (showname) {
		strncpy(s + 4, s + 11, 5);
		s[9] = 0;
	} else {
		strncpy(s + 4, s + 8, 8);
		s[12] = 0;
	}

	return s;
}

#if 0 /*UNUSED*/
/*
 *	See how long a tty has been idle.
 */
char *idletime(char *line)
{
	char tty[16];
	static char tmp[8];
	time_t t;
	struct stat st;
	int hr, min, days;

	if (line[0] == '/')
		strcpy(tty, "/dev/");
	else
		tty[0] = 0;
	strncat(tty, line, 10);
	tty[15] = 0;

	tmp[0] = 0;
	if (stat(tty, &st) == 0) {
		time(&t);
		t -= st.st_mtime;
		if (t >= 60) {
			min = (t / 60);
			hr = min / 24;
			days = hr / 24;
			min %= 60;
			hr %= 24;
			if (days > 0)
				snprintf(tmp, sizeof(tmp), "%dd", days);
			else
				snprintf(tmp, sizeof(tmp), "%2d:%02d", hr, min);
		}
	}
	return tmp;
}
#endif

/*
 *	Shorten tty name.
 */
static const char *ttyshort(char *tty)
{
	static char tmp[16];

	if (tty[0] == '/') tty += 5;

	if (strncmp(tty, "tty", 3) == 0) {
		if (tty[3] >= '0' && tty[3] <= '9')
			snprintf(tmp, sizeof(tmp), "v%.14s", tty + 3);
		else
			snprintf(tmp, sizeof(tmp), "%.15s", tty + 3);
		return tmp;
	}
	if (strncmp(tty, "vc", 2) == 0) {
		snprintf(tmp, sizeof(tmp), "v.14%s", tty + 2);
		return tmp;
	}
	if (strncmp(tty, "cu", 2) == 0) {
		return tmp + 2;
	}
	return "??";
}


/*
 *	Print address of NAS.
 */
static const char *hostname(char *buf, size_t buflen, uint32_t ipaddr)
{
	if (ipaddr == 0 || ipaddr == (uint32_t)-1 || ipaddr == (uint32_t)-2)
		return "";
	return ip_hostname(buf, buflen, ipaddr);
}


/*
 *	Print usage message and exit.
 */
static void usage(void)
{
	fprintf(stderr, "Usage: radwho [-d raddb] [-lhfnsipcr]\n");
	fprintf(stderr, "       -d: set the raddb directory (default is %s)\n",
		RADIUS_DIR);
	fprintf(stderr, "       -l: show local (shell) users too\n");
	fprintf(stderr, "       -h: hide shell users from radius\n");
	fprintf(stderr, "       -f: give fingerd output\n");
	fprintf(stderr, "       -n: no full name\n");
	fprintf(stderr, "       -s: show full name\n");
	fprintf(stderr, "       -i: show session ID\n");
	fprintf(stderr, "       -p: show port type\n");
	fprintf(stderr, "       -c: show caller ID, if available\n");
	fprintf(stderr, "       -r: output as raw data\n");
	exit(1);
}


/*
 *	Main program, either pmwho or fingerd.
 */
int main(int argc, char **argv)
{
	CONF_SECTION *maincs, *cs;
	FILE *fp;
	struct radutmp rt;
	struct utmp ut;
	int hdrdone = 0;
	char inbuf[128];
	char myname[128];
	char othername[256];
	char nasname[1024];
	char session_id[sizeof(rt.session_id)+1];
	int fingerd = 0;
	int showlocal = 0;
	int hideshell = 0;
	int showsid = 0;
	int rawoutput = 0;
	char *p, *q;
	const char *portind;
	int c, portno;

	radius_dir = strdup(RADIUS_DIR);

	while((c = getopt(argc, argv, "d:flhnsipcr")) != EOF) switch(c) {
		case 'd':
			if (radius_dir) free(radius_dir);
			radius_dir = strdup(optarg);
			break;
		case 'f':
			fingerd++;
			showname = 0;
			break;
		case 'l':
			showlocal = 1;
			break;
		case 'h':
			hideshell = 1;
			break;
		case 'n':
			showname = 0;
			break;
		case 's':
			showname = 1;
			break;
		case 'i':
			showsid = 1;
			break;
		case 'p':
			showptype = 1;
			break;
		case 'c':
			showcid = 1;
			showname = 1;
			break;
		case 'r':
			rawoutput = 1;
			break;
		default:
			usage();
			break;
	}

	/*
	 *	Ensure that the configuration is initialized.
	 */
	memset(&mainconfig, 0, sizeof(mainconfig));

	/* Read radiusd.conf */
	if ((maincs = read_radius_conf_file()) == NULL) {
		fprintf(stderr, "%s: Errors reading radiusd.conf\n", argv[0]);
		exit(1);
	}

	/* Read the radutmp section of radiusd.conf */
	cs = cf_section_sub_find(cf_section_sub_find(maincs, "modules"), "radutmp");
	if(!cs) {
		fprintf(stderr, "%s: No configuration information in radutmp section of radiusd.conf!\n",
			argv[0]);
		exit(1);
	}

	cf_section_parse(cs, NULL, module_config);

	/* Assign the correct path for the radutmp file */
	radutmp_file = radutmpconfig.radutmp_fn;

	/*
	 *	See if we are "fingerd".
	 */
	if (strstr(argv[0], "fingerd")) {
		fingerd++;
		eol = "\r\n";
		if (showname < 0) showname = 0;
	}
	if (showname < 0) showname = 1;

	if (fingerd) {
		/*
		 *	Read first line of the input.
		 */
		fgets(inbuf, 128, stdin);
		p = inbuf;
		while(*p == ' ' || *p == '\t') p++;
		if (*p == '/' && *(p + 1)) p += 2;
		while(*p == ' ' || *p == '\t') p++;
		for(q = p; *q && *q != '\r' && *q != '\n'; q++)
			;
		*q = 0;

		/*
		 *	See if we fingered a specific user.
		 */
		ffile("header");
		if (*p) sys_finger(p);
	}

	if (showlocal && (fp = fopen(radutmp_file, "r"))) {
		if (rawoutput == 0)
		{	
			fputs(showname ? hdr1 : hdr2, stdout);
			fputs(eol, stdout);
		}
		hdrdone = 1;

		/*
		 *	Show the logged in UNIX users.
		 */
		gethostname(myname, 128);
		while(fread(&ut, sizeof(ut), 1, fp) == 1) {
#ifdef USER_PROCESS
			if (ut.ut_user[0] && ut.ut_line[0] &&
				ut.ut_type == USER_PROCESS) {
#else
			if (ut.ut_user[0] && ut.ut_line[0]) {
#endif
#ifdef UT_HOSTSIZE

#ifdef HAVE_UTMPX_H
#	define UT_TIME ut_xtime
#else
#	define UT_TIME ut_time
#endif
			if (showname)
				printf((rawoutput == 0? ufmt1: ufmt1r),
						ut.ut_name,
						fullname(ut.ut_name),
						"shell",
						ttyshort(ut.ut_line),
						dotime(ut.UT_TIME),
						ut.ut_host,
						myname, eol);
			else
				printf((rawoutput==0? ufmt2:ufmt2r),
						ut.ut_name,
						ttyshort(ut.ut_line),
						"shell",
						dotime(ut.UT_TIME),
						ut.ut_host,
						myname, eol);
#endif
			}
		}
		fclose(fp);
	}

	/*
	 *	Show the users logged in on the terminal server(s).
	 */
	if ((fp = fopen(radutmp_file, "r")) == NULL)
		return 0;

	if (!hdrdone) {
		fputs(showname ? hdr1 : hdr2, stdout);
		fputs(eol, stdout);
	}

	while(fread(&rt, sizeof(rt), 1, fp) == 1) {
		if (rt.type == P_LOGIN) {
			/*
			 *	We don't show shell users if we are
			 *	fingerd, as we have done that above.
			 */
			if (hideshell && !strchr("PCS", rt.proto))
				continue;

			memcpy(session_id, rt.session_id, sizeof(rt.session_id));
			session_id[sizeof(rt.session_id)] = 0;

			if (!rawoutput && rt.nas_port > (showname ? 999 : 99999)) {
				portind = ">";
				portno = (showname ? 999 : 99999);
			} else {
				portind = "S";
				portno = rt.nas_port;
			}
			if (showname)
				printf((rawoutput == 0? rfmt1: rfmt1r),
						rt.login,
						showcid ? rt.caller_id :
						(showsid? session_id : fullname(rt.login)),
						proto(rt.proto, rt.porttype),
						portind, portno,
						dotime(rt.time),
						nas_name3(nasname, sizeof(nasname), rt.nas_address),
						hostname(othername, sizeof(othername), rt.framed_address), eol);
			else
				printf((rawoutput == 0? rfmt2: rfmt2r),
						rt.login,
						portind, portno,
						proto(rt.proto, rt.porttype),
						dotime(rt.time),
						nas_name3(nasname, sizeof(nasname), rt.nas_address),
						hostname(othername, sizeof(othername), rt.framed_address), 	
						eol);
		}
	}
	fflush(stdout);
	fflush(stderr);
	fclose(fp);

	return 0;
}

