/*
 * mainconf.c	Handle the server's configuration.
 *
 * Version:	$Id: mainconfig.c,v 1.20.2.1 2003/07/22 18:17:27 aland Exp $
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
 * Copyright 2002  The FreeRADIUS server project
 * Copyright 2002  Alan DeKok <aland@ox.org>
 */

#include "autoconf.h"
#include "libradius.h"

#include <stdlib.h>
#include <string.h>

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#include "radiusd.h"
#include "rad_assert.h"
#include "conffile.h"
#include "token.h"

#include <sys/resource.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <grp.h>
#include <pwd.h>


struct main_config_t mainconfig;

/*
 *  Local variables for stuff.
 */
static uid_t server_uid;
static gid_t server_gid;

/*
 *	These are not used anywhere else..
 */
static const char *localstatedir = NULL;
static const char *prefix = NULL;

/*
 *  Map the proxy server configuration parameters to variables.
 */
static CONF_PARSER proxy_config[] = {
	{ "retry_delay",  PW_TYPE_INTEGER, 0, &mainconfig.proxy_retry_delay, Stringify(RETRY_DELAY) },
	{ "retry_count",  PW_TYPE_INTEGER, 0, &mainconfig.proxy_retry_count, Stringify(RETRY_COUNT) },
	{ "synchronous",  PW_TYPE_BOOLEAN, 0, &mainconfig.proxy_synchronous, "no" },
	{ "default_fallback", PW_TYPE_BOOLEAN, 0, &mainconfig.proxy_fallback, "no" },
	{ "dead_time",    PW_TYPE_INTEGER, 0, &mainconfig.proxy_dead_time, Stringify(DEAD_TIME) },
        { "post_proxy_authorize", PW_TYPE_BOOLEAN, 0, &mainconfig.post_proxy_authorize, "yes" },
	{ "wake_all_if_all_dead", PW_TYPE_BOOLEAN, 0, &mainconfig.wake_all_if_all_dead, "no" },
	{ NULL, -1, 0, NULL, NULL }
};

/*
 *  Security configuration for the server.
 */
static CONF_PARSER security_config[] = {
	{ "max_attributes",  PW_TYPE_INTEGER, 0, &librad_max_attributes, Stringify(0) },
	{ "reject_delay",  PW_TYPE_INTEGER, 0, &mainconfig.reject_delay, Stringify(0) },
	{ "status_server", PW_TYPE_BOOLEAN, 0, &mainconfig.status_server, "no"},
	{ NULL, -1, 0, NULL, NULL }
};

/*
 *  A mapping of configuration file names to internal variables
 */
static CONF_PARSER server_config[] = {
	/*
	 *	FIXME: 'prefix' is the ONLY one which should be
	 *	configured at compile time.  Hard-coding it here is
	 *	bad.  It will be cleaned up once we clean up the
	 *	hard-coded defines for the locations of the various
	 *	files.
	 */
	{ "prefix",             PW_TYPE_STRING_PTR, 0, &prefix,            "/usr/local"},
	{ "localstatedir",      PW_TYPE_STRING_PTR, 0, &localstatedir,     "${prefix}/var"}, 
	{ "logdir",             PW_TYPE_STRING_PTR, 0, &radlog_dir,        "${localstatedir}/log"},
	{ "libdir",             PW_TYPE_STRING_PTR, 0, &radlib_dir,        "${prefix}/lib"},
	{ "radacctdir",         PW_TYPE_STRING_PTR, 0, &radacct_dir,       "${logdir}/radacct" },
	{ "hostname_lookups",   PW_TYPE_BOOLEAN,    0, &librad_dodns,      "no" },
#ifdef WITH_SNMP
	{ "snmp",   		PW_TYPE_BOOLEAN,    0, &mainconfig.do_snmp,      "no" },
#endif
	{ "max_request_time", PW_TYPE_INTEGER, 0, &mainconfig.max_request_time, Stringify(MAX_REQUEST_TIME) },
	{ "cleanup_delay", PW_TYPE_INTEGER, 0, &mainconfig.cleanup_delay, Stringify(CLEANUP_DELAY) },
	{ "max_requests", PW_TYPE_INTEGER, 0, &mainconfig.max_requests, Stringify(MAX_REQUESTS) },
	{ "delete_blocked_requests", PW_TYPE_INTEGER, 0, &mainconfig.kill_unresponsive_children, Stringify(FALSE) },
	{ "port", PW_TYPE_INTEGER, 0, &auth_port, Stringify(PW_AUTH_UDP_PORT) },
	{ "allow_core_dumps", PW_TYPE_BOOLEAN, 0, &mainconfig.allow_core_dumps, "no" },
	{ "log_stripped_names", PW_TYPE_BOOLEAN, 0, &log_stripped_names,"no" },
	{ "log_file", PW_TYPE_STRING_PTR, -1, &mainconfig.log_file, "${logdir}/radius.log" },
	{ "log_auth", PW_TYPE_BOOLEAN, -1, &mainconfig.log_auth, "no" },
	{ "log_auth_badpass", PW_TYPE_BOOLEAN, 0, &mainconfig.log_auth_badpass, "no" },
	{ "log_auth_goodpass", PW_TYPE_BOOLEAN, 0, &mainconfig.log_auth_goodpass, "no" },
	{ "pidfile", PW_TYPE_STRING_PTR, 0, &mainconfig.pid_file, "${run_dir}/radiusd.pid"},
	{ "bind_address", PW_TYPE_IPADDR, 0, &mainconfig.myip, "*" },
	{ "user", PW_TYPE_STRING_PTR, 0, &mainconfig.uid_name, NULL},
	{ "group", PW_TYPE_STRING_PTR, 0, &mainconfig.gid_name, NULL},
	{ "usercollide", PW_TYPE_BOOLEAN, 0, &mainconfig.do_usercollide,  "no" },
	{ "lower_user", PW_TYPE_STRING_PTR, 0, &mainconfig.do_lower_user, "no" },
	{ "lower_pass", PW_TYPE_STRING_PTR, 0, &mainconfig.do_lower_pass, "no" },
	{ "nospace_user", PW_TYPE_STRING_PTR, 0, &mainconfig.do_nospace_user, "no" },
	{ "nospace_pass", PW_TYPE_STRING_PTR, 0, &mainconfig.do_nospace_pass, "no" },
	{ "checkrad", PW_TYPE_STRING_PTR, 0, &mainconfig.checkrad, "${sbindir}/checkrad" },
	{ "proxy_requests", PW_TYPE_BOOLEAN, 0, &mainconfig.proxy_requests, "yes" },
	{ "proxy", PW_TYPE_SUBSECTION, 0, proxy_config, NULL },
	{ "security", PW_TYPE_SUBSECTION, 0, security_config, NULL },
	{ "debug_level", PW_TYPE_INTEGER, 0, &mainconfig.debug_level, "0"},
	{ NULL, -1, 0, NULL, NULL }
};

/*
 *	Xlat for %{config:section.subsection.attribute}
 */
static int xlat_config(void *instance, REQUEST *request,
		       char *fmt, char *out, int outlen,
		       RADIUS_ESCAPE_STRING func)
{
	CONF_SECTION *cs;
	CONF_PAIR *cp;
	char buffer[1024];
	char *p, *value;
	const char *start = fmt;

	request = request;	/* -Wunused */
	instance = instance;	/* -Wunused */

	cp = NULL;
	cs = NULL;

	while (cp == NULL) {
		/*
		 *	Find the next section.
		 */
		for (p = buffer; (*fmt != 0) && (*fmt != '.'); p++, fmt++) {
			*p = *fmt;
		}
		*p = '\0';

		/*
		 *  The character is a '.', find a section (as the user
		 *  has given us a subsection to find)
		 */
		if (*fmt == '.') {
			CONF_SECTION *next;

			fmt++;	/* skip the period */

			if (cs == NULL) {
			  next = cf_section_find(buffer);
			} else {
			  next = cf_subsection_find_next(cs, NULL, buffer);
			}
			if (next == NULL) {
				radlog(L_ERR, "config: No such section %s in format string %s", buffer, start);
				return 0;
			}
			cs = next;

		} else {	/* no period, must be a conf-part */
			cp = cf_pair_find(cs, buffer);

			if (cp == NULL) {
				radlog(L_ERR, "config: No such section %s in format string %s", buffer, start);
				return 0;
			}
		}
	} /* until cp is non-NULL */

	/*
	 *  Ensure that we only copy what's necessary.
	 *
	 *  If 'outlen' is too small, then the output is chopped to fit.
	 */
	value = cf_pair_value(cp);
	if (value) {
		if (outlen > strlen(value)) {
			outlen = strlen(value) + 1;
		}
	}
	
	return func(out, outlen, value);
}


/*
 *	Recursively make directories.
 */
static int r_mkdir(const char *part) {
	char *ptr, parentdir[500];
	struct stat st;

	if (stat(part, &st) == 0)
		return(0);

	ptr = strrchr(part, '/');

	if (ptr == part)
		return(0);

	snprintf(parentdir, (ptr - part)+1, "%s", part);

	if (r_mkdir(parentdir) != 0)
		return(1);

	if (mkdir(part, 0770) != 0) {
		fprintf(stderr, "mkdir(%s) error: %s\n", part, strerror(errno));
		return(1);
	}

	fprintf(stderr, "Created directory %s\n", part);

	return(0);
}
		
/*
 *	Checks if the log directory is writeable by a particular user.
 */
static int radlogdir_iswritable(const char *effectiveuser) {
	struct passwd *pwent;

	if (radlog_dir[0] != '/')
		return(0);

	if (r_mkdir(radlog_dir) != 0)
		return(1);

	/* FIXME: do we have this function? */
	if (strstr(radlog_dir, "radius") == NULL)
		return(0);

	/* we have a logdir that mentions 'radius', so it's probably 
	 * safe to chown the immediate directory to be owned by the normal 
	 * process owner. we gotta do it before we give up root.  -chad
	 */
	
	if (!effectiveuser) {
		return 1;
	}

	pwent = getpwnam(effectiveuser);

	if (pwent == NULL) /* uh oh! */
		return(1);

	if (chown(radlog_dir, pwent->pw_uid, -1) != 0)
		return(1);

	return(0);
}


static int switch_users(void) {
	
	/*
	 *  Switch UID and GID to what is specified in the config file
	 */

	/*  Set GID.  */
	if (mainconfig.gid_name != NULL) {
		struct group *gr;

		gr = getgrnam(mainconfig.gid_name);
		if (gr == NULL) {
			if (errno == ENOMEM) {
				radlog(L_ERR|L_CONS, "Cannot switch to Group %s: out of memory", mainconfig.gid_name);
			} else {
				radlog(L_ERR|L_CONS, "Cannot switch group; %s doesn't exist", mainconfig.gid_name);
			}
			exit(1);
		}
		server_gid = gr->gr_gid;
		if (setgid(server_gid) < 0) {
			radlog(L_ERR|L_CONS, "Failed setting Group to %s: %s", mainconfig.gid_name, strerror(errno));
			exit(1);
		}
	}

	/*  Set UID.  */
	if (mainconfig.uid_name != NULL) {
		struct passwd *pw;

		pw = getpwnam(mainconfig.uid_name);
		if (pw == NULL) {
			if (errno == ENOMEM) {
				radlog(L_ERR|L_CONS, "Cannot switch to User %s: out of memory", mainconfig.uid_name);
			} else {
				radlog(L_ERR|L_CONS, "Cannot switch user; %s doesn't exist", mainconfig.uid_name);
			}
			exit(1);
		}
		server_uid = pw->pw_uid;
		if (setuid(server_uid) < 0) {
			radlog(L_ERR|L_CONS, "Failed setting User to %s: %s", mainconfig.uid_name, strerror(errno));
			exit(1);
		}
	}
	return(0);
}


/* 
 * Create the linked list of realms from the new configuration type
 * This way we don't have to change to much in the other source-files
 */
static int generate_realms(const char *filename)
{
	CONF_SECTION *cs;
	REALM *my_realms = NULL;
	REALM *c, **tail;
	char *s, *t, *authhost, *accthost;
	char *name2;

	tail = &my_realms;
	for (cs = cf_subsection_find_next(mainconfig.config, NULL, "realm"); cs != NULL;
			cs = cf_subsection_find_next(mainconfig.config, cs, "realm")) {
		name2 = cf_section_name2(cs);
		if (!name2) {
			radlog(L_CONS|L_ERR, "%s[%d]: Missing realm name", filename, cf_section_lineno(cs));
			return -1;
		}
		/*
		 * We've found a realm, allocate space for it
		 */
		c = rad_malloc(sizeof(REALM));
		memset(c, 0, sizeof(REALM));

		c->secret[0] = '\0';

		/*
		 *	No authhost means LOCAL.
		 */
		if ((authhost = cf_section_value_find(cs, "authhost")) == NULL) {
			c->ipaddr = htonl(INADDR_NONE);
			c->auth_port = auth_port;
		} else {
			if ((s = strchr(authhost, ':')) != NULL) {
				*s++ = 0;
				c->auth_port = atoi(s);
			} else {
				c->auth_port = auth_port;
			}
			if (strcmp(authhost, "LOCAL") == 0) {
				/*
				 *	Local realms don't have an IP address,
				 *	secret, or port.
				 */
				c->ipaddr = htonl(INADDR_NONE);
				c->auth_port = auth_port;
			} else {
				c->ipaddr = ip_getaddr(authhost);
			}

			/* 
			 * Double check length, just to be sure!
			 */
			if (strlen(authhost) >= sizeof(c->server)) {
				radlog(L_ERR, "%s[%d]: Server name of length %d is greater than allowed: %d",
				       filename, cf_section_lineno(cs),
				       strlen(authhost), sizeof(c->server) - 1);
				return -1;
			}
		}

		/*
		 *	No accthost means LOCAL
		 */
		if ((accthost = cf_section_value_find(cs, "accthost")) == NULL) {
			c->acct_ipaddr = htonl(INADDR_NONE);
			c->acct_port = acct_port;
		} else {
			if ((s = strchr(accthost, ':')) != NULL) {
				*s++ = 0;
				c->acct_port = atoi(s);	
			} else {
				c->acct_port = acct_port;
			}
			if (strcmp(accthost, "LOCAL") == 0) {
				/*
				 *	Local realms don't have an IP address,
				 *	secret, or port.
				 */
				c->acct_ipaddr = htonl(INADDR_NONE);
				c->acct_port = acct_port;
			} else {
				c->acct_ipaddr = ip_getaddr(accthost);
			}

			if (strlen(accthost) >= sizeof(c->acct_server)) {
				radlog(L_ERR, "%s[%d]: Server name of length %d is greater than allowed: %d",
				       filename, cf_section_lineno(cs),
				       strlen(accthost), sizeof(c->acct_server) - 1);
				return -1;
			}
		}

		if (strlen(name2) >= sizeof(c->realm)) {
			radlog(L_ERR, "%s[%d]: Realm name of length %d is greater than allowed %d",
					filename, cf_section_lineno(cs),
					strlen(name2), sizeof(c->server) - 1);
			return -1;
		}
		
		strcpy(c->realm, name2);
                if (authhost) strcpy(c->server, authhost);
		if (accthost) strcpy(c->acct_server, accthost);	

		/*
		 *	If one or the other of authentication/accounting
		 *	servers is set to LOCALHOST, then don't require
		 *	a shared secret.
		 */
		if ((c->ipaddr != htonl(INADDR_NONE)) ||
		    (c->acct_ipaddr != htonl(INADDR_NONE))) {
			if ((s = cf_section_value_find(cs, "secret")) == NULL ) {
				radlog(L_ERR, "%s[%d]: No shared secret supplied for realm: %s",
				       filename, cf_section_lineno(cs), name2);
				return -1;
			}
			
			if (strlen(s) >= sizeof(c->secret)) {
				radlog(L_ERR, "%s[%d]: Secret of length %d is greater than the allowed maximum of %d.",
				       filename, cf_section_lineno(cs),
				       strlen(s), sizeof(c->secret) - 1);
				return -1;
			}
			strNcpy((char *)c->secret, s, sizeof(c->secret));
		}

		c->striprealm = 1;
		
		if ((cf_section_value_find(cs, "nostrip")) != NULL)
			c->striprealm = 0;
		if ((cf_section_value_find(cs, "noacct")) != NULL)
			c->acct_port = 0;
		if ((cf_section_value_find(cs, "trusted")) != NULL)
			c->trusted = 1;
		if ((cf_section_value_find(cs, "notrealm")) != NULL)
			c->notrealm = 1;
		if ((cf_section_value_find(cs, "notsuffix")) != NULL)
			c->notrealm = 1;
		if ((t = cf_section_value_find(cs,"ldflag")) != NULL) {
			static const LRAD_NAME_NUMBER ldflags[] = {
				{ "fail_over",   0 },
				{ "round_robin", 1 },
				{ NULL, 0 }
			};

			c->ldflag = lrad_str2int(ldflags, t, -1);
			if (c->ldflag == -1) {
				radlog(L_ERR, "%s[%d]: Unknown value \"%s\" for ldflag",
				       filename, cf_section_lineno(cs),
				       t);
				return -1;
			}

		} else {
			c->ldflag = 0; /* non, make it fail-over */
		}
		c->active = TRUE;
		c->acct_active = TRUE;

		c->next = NULL;
		*tail = c;
		tail = &c->next;
	}

	/*
	 *	And make these realms preferred over the ones
	 *	in the 'realms' file.
	 */
	*tail = mainconfig.realms;
	mainconfig.realms = my_realms;

	/*
	 *  Ensure that all of the flags agree for the realms.
	 *
	 *	Yeah, it's O(N^2), but it's only once, and the
	 *	maximum number of realms is small.
	 */
	for(c = mainconfig.realms; c != NULL; c = c->next) {
		REALM *this;

		/*
		 *	Check that we cannot load balance to LOCAL
		 *	realms, as that doesn't make any sense.
		 */
		if ((c->ldflag == 1) &&
		    ((c->ipaddr == htonl(INADDR_NONE)) ||
		     (c->acct_ipaddr == htonl(INADDR_NONE)))) {
			radlog(L_ERR | L_CONS, "ERROR: Realm %s cannot be load balanced to LOCAL",
			       c->realm);
			exit(1);
		}

		/*
		 *	Compare this realm to all others, to ensure
		 *	that the configuration is consistent.
		 */
		for (this = c->next; this != NULL; this = this->next) {
			if (strcasecmp(c->realm, this->realm) != 0) {
				continue;
			}

			/*
			 *	Same realm: Different load balancing
			 *	flag: die.
			 */
			if (c->ldflag != this->ldflag) {
				radlog(L_ERR | L_CONS, "ERROR: Inconsistent value in realm %s for load balancing 'ldflag' attribute",
				       c->realm);
				exit(1);
			}
		}
	}

	return 0;
}


/*
 *	Create the linked list of realms from the new configuration
 *	type.  This way we don't have to change too much in the other
 *	source-files.
 */
static int generate_clients(const char *filename)
{
	CONF_SECTION	*cs;
	RADCLIENT	*c;
	char		*hostnm, *secret, *shortnm, *netmask;
	char            *nastype, *login, *password;
	char		*name2;

	for (cs = cf_subsection_find_next(mainconfig.config, NULL, "client");
	     cs != NULL; 
	     cs = cf_subsection_find_next(mainconfig.config, cs, "client")) {

		name2 = cf_section_name2(cs);
		if (!name2) {
			radlog(L_CONS|L_ERR, "%s[%d]: Missing client name",
			       filename, cf_section_lineno(cs));
			return -1;
		}
		/*
		 * Check the lengths, we don't want any core dumps
		 */
		hostnm = name2;

		if((secret = cf_section_value_find(cs, "secret")) == NULL) {
			radlog(L_ERR, "%s[%d]: Missing secret for client: %s",
				filename, cf_section_lineno(cs), name2);
			return -1;
		}

		if((shortnm = cf_section_value_find(cs, "shortname")) == NULL) {
			radlog(L_ERR, "%s[%d]: Missing shortname for client: %s",
				filename, cf_section_lineno(cs), name2);
			return -1;
		}

		netmask = strchr(hostnm, '/');

		if (strlen(secret) >= sizeof(c->secret)) {
			radlog(L_ERR, "%s[%d]: Secret of length %d is greater than the allowed maximum of %d.",
				filename, cf_section_lineno(cs),
				strlen(secret), sizeof(c->secret) - 1);
			return -1;
		}
		if (strlen(shortnm) > sizeof(c->shortname)) {
			radlog(L_ERR, "%s[%d]: Client short name of length %d is greater than the allowed maximum of %d.",
					filename, cf_section_lineno(cs),
					strlen(shortnm), sizeof(c->shortname) - 1);
			return -1;
		}

		if((nastype = cf_section_value_find(cs, "nastype")) != NULL) {
		        if(strlen(nastype) >= sizeof(c->nastype)) { 
			       radlog(L_ERR, "%s[%d]: nastype of length %d longer than the allowed maximum of %d",
				      filename, cf_section_lineno(cs), 
				      strlen(nastype), sizeof(c->nastype) - 1);
			       return -1;
			}
		}

		if((login = cf_section_value_find(cs, "login")) != NULL) {
		        if(strlen(login) >= sizeof(c->login)) { 
			       radlog(L_ERR, "%s[%d]: login of length %d longer than the allowed maximum of %d",
				      filename, cf_section_lineno(cs), 
				      strlen(login), sizeof(c->login) - 1);
			       return -1;
			}
		}

		if((password = cf_section_value_find(cs, "password")) != NULL) {
		        if(strlen(password) >= sizeof(c->password)) { 
			       radlog(L_ERR, "%s[%d]: password of length %d longer than the allowed maximum of %d",
				      filename, cf_section_lineno(cs), 
				      strlen(password), sizeof(c->password) - 1);
			       return -1;
			}
		}

		/*
		 * The size is fine.. Let's create the buffer
		 */
		c = rad_malloc(sizeof(RADCLIENT));
		memset(c, 0, sizeof(RADCLIENT));

		/*
		 *	Look for netmasks.
		 */
		c->netmask = ~0;
		if (netmask) {
			int i, mask_length;

			mask_length = atoi(netmask + 1);
			if ((mask_length < 0) || (mask_length > 32)) {
				radlog(L_ERR, "%s[%d]: Invalid value '%s' for IP network mask.",
						filename, cf_section_lineno(cs), netmask + 1);
				return -1;
			}
			
			c->netmask = (1 << 31);
			for (i = 1; i < mask_length; i++) {
				c->netmask |= (c->netmask >> 1);
			}

			*netmask = '\0';
			c->netmask = htonl(c->netmask);
		}

		c->ipaddr = ip_getaddr(hostnm);
		if (c->ipaddr == INADDR_NONE) {
			radlog(L_CONS|L_ERR, "%s[%d]: Failed to look up hostname %s",
					filename, cf_section_lineno(cs), hostnm);
			return -1;
		}

		/*
		 *	Update the client name again...
		 */
		if (netmask) {
			*netmask = '/';
			c->ipaddr &= c->netmask;
			strcpy(c->longname, hostnm);
		} else {
			ip_hostname(c->longname, sizeof(c->longname),
					c->ipaddr);
		}

		strcpy((char *)c->secret, secret);
		strcpy(c->shortname, shortnm);
		if(nastype != NULL)
		        strcpy(c->nastype, nastype);
		if(login != NULL)
		        strcpy(c->login, login);
		if(password != NULL)
		        strcpy(c->password, password);

		c->next =mainconfig. clients;
		mainconfig.clients = c;
	}

	return 0;
}

#ifndef RADIUS_CONFIG
#define RADIUS_CONFIG "radiusd.conf"
#endif

CONF_SECTION *read_radius_conf_file(void)
{
	char buffer[256];
	CONF_SECTION *cs;

	/* Lets go look for the new configuration files */
	snprintf(buffer, sizeof(buffer), "%.200s/%.50s", radius_dir, RADIUS_CONFIG);
	if ((cs = conf_read(NULL, 0, buffer, NULL)) == NULL) {
		return NULL;
	}

	/*
	 *	This allows us to figure out where, relative to
	 *	radiusd.conf, the other configuration files exist.
	 */
	cf_section_parse(cs, NULL, server_config);

	/* Initialize the dictionary */
	DEBUG2("read_config_files:  reading dictionary");
	if (dict_init(radius_dir, RADIUS_DICTIONARY) != 0) {
		radlog(L_ERR|L_CONS, "Errors reading dictionary: %s",
				librad_errstr);
		cf_section_free(&cs);
		return NULL;
	}

	return cs;
}


/*
 *	Read config files.
 *
 *	This function can ONLY be called from the main server process.
 */
int read_mainconfig(int reload)
{
	struct rlimit core_limits;
	static int old_debug_level = -1;
	char buffer[1024];
	CONF_SECTION *cs, *oldcs;

	if (!reload) {
		radlog(L_INFO, "Starting - reading configuration files ...");
	} else {
		radlog(L_INFO, "Reloading configuration files.");
	}

	/* First read radiusd.conf */
	DEBUG2("reread_config:  reading radiusd.conf");
	if ((cs = read_radius_conf_file()) == NULL) {
		if (debug_flag ||
		    (radlog_dir == NULL)) {
			radlog(L_ERR|L_CONS, "Errors reading radiusd.conf");
		} else {
			radlog(L_ERR|L_CONS, "Errors reading %s/radiusd.conf: For more information, please read the tail end of %s", radlog_dir, mainconfig.log_file);
		}
		return -1;
	}

	/*
	 *	Free the old configuration items, and replace them
	 *	with the new ones.
	 *
	 *	Note that where possible, we do atomic switch-overs,
	 *	to ensure that the pointers are always valid.
	 */
	oldcs = mainconfig.config;
	mainconfig.config = cs;
	cf_section_free(&oldcs);

	/* old-style naslist file */
	snprintf(buffer, sizeof(buffer), "%.200s/%.50s", radius_dir, RADIUS_NASLIST);
	DEBUG2("read_config_files:  reading naslist");
	if (read_naslist_file(buffer) < 0) {
		radlog(L_ERR|L_CONS, "Errors reading naslist");
		return -1;
	}
	/* old-style clients file */
	snprintf(buffer, sizeof(buffer), "%.200s/%.50s", radius_dir, RADIUS_CLIENTS);
	DEBUG2("read_config_files:  reading clients");
	if (read_clients_file(buffer) < 0) {
		radlog(L_ERR|L_CONS, "Errors reading clients");
		return -1;
	}

	/*
	 *	Add to that, the *new* list of clients.
	 */
	snprintf(buffer, sizeof(buffer), "%.200s/%.50s", radius_dir, RADIUS_CONFIG);
	if (generate_clients(buffer) < 0) {
		return -1;
	}

	/* old-style realms file */
	snprintf(buffer, sizeof(buffer), "%.200s/%.50s", radius_dir, RADIUS_REALMS);
	DEBUG2("read_config_files:  reading realms");
	if (read_realms_file(buffer) < 0) {
		radlog(L_ERR|L_CONS, "Errors reading realms");
		return -1;
	}

	/*
	 *	If there isn't any realms it isn't fatal..
	 */
	snprintf(buffer, sizeof(buffer), "%.200s/%.50s", radius_dir, RADIUS_CONFIG);
	if (generate_realms(buffer) < 0) {
		return -1;
	}

	/*
	 *  Register the %{config:section.subsection} xlat function.
	 */
	xlat_register("config", xlat_config, NULL);

	/*
	 *	Set the libraries debugging flag to whatever the main
	 *	flag is.  Note that on a SIGHUP, to turn the debugging
	 *	off, we do other magic.
	 *
	 *	Increase the debug level, if the configuration file
	 *	says to, OR, if we're decreasing the debug from what it
	 *	was before, allow that, too.
	 */
	if ((mainconfig.debug_level > debug_flag) ||
	    (mainconfig.debug_level <= old_debug_level)) {
	  debug_flag = mainconfig.debug_level;
	}
	librad_debug = debug_flag;
	old_debug_level = mainconfig.debug_level;
	
	/*
	 *  Go update our behaviour, based on the configuration
	 *  changes.
	 */

	/*  Get the current maximum for core files.  */
	if (getrlimit(RLIMIT_CORE, &core_limits) < 0) {
		radlog(L_ERR|L_CONS, "Failed to get current core limit:  %s", strerror(errno));
		exit(1);
	}

	if (mainconfig.allow_core_dumps) {
		if (setrlimit(RLIMIT_CORE, &core_limits) < 0) {
			radlog(L_ERR|L_CONS, "Cannot update core dump limit: %s",
					strerror(errno));
			exit(1);

			/*
			 *  If we're running as a daemon, and core
			 *  dumps are enabled, log that information.
			 */
		} else if ((core_limits.rlim_cur != 0) && !debug_flag)
			radlog(L_INFO|L_CONS, "Core dumps are enabled.");

	} else if (!debug_flag) {
		/*
		 *  Not debugging.  Set the core size to zero, to
		 *  prevent security breaches.  i.e. People
		 *  reading passwords from the 'core' file.
		 */
		struct rlimit limits;

		limits.rlim_cur = 0;
		limits.rlim_max = core_limits.rlim_max;
		
		if (setrlimit(RLIMIT_CORE, &limits) < 0) {
			radlog(L_ERR|L_CONS, "Cannot disable core dumps: %s",
					strerror(errno));
			exit(1);
		}
	}

	/*
	 * 	The first time around, ensure that we can write to the
	 *	log directory.
	 */
	if (!reload) {
		/*
		 *	We need root to do mkdir() and chown(), so we
		 *	do this before giving up root.
		 */
		radlogdir_iswritable(mainconfig.uid_name); 
	}
	switch_users();

	/*
	 *	Sanity check the configuration for internal
	 *	consistency.
	 */
	if (mainconfig.reject_delay > mainconfig.cleanup_delay) {
		mainconfig.reject_delay = mainconfig.cleanup_delay;
	}

	return 0;
}

/*
 *	Free the configuration.
 */
int free_mainconfig(void)
{
	/*
	 *	Clean up the configuration data
	 *	structures.
	 */
	cf_section_free(&mainconfig.config);
	realm_free(mainconfig.realms);
	clients_free(mainconfig.clients);

	return 0;
}
