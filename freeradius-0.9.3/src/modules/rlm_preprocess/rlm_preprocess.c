/*
 * rlm_preprocess.c
 *		Contains the functions for the "huntgroups" and "hints"
 *		files.
 *
 * Version:     $Id: rlm_preprocess.c,v 1.45 2003/07/07 19:17:31 aland Exp $
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

static const char rcsid[] = "$Id: rlm_preprocess.c,v 1.45 2003/07/07 19:17:31 aland Exp $";

#include	"autoconf.h"
#include	"libradius.h"

#include	<sys/stat.h>

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	"radiusd.h"
#include	"modules.h"

typedef struct rlm_preprocess_t {
	char		*huntgroup_file;
	char		*hints_file;
	PAIR_LIST	*huntgroups;
	PAIR_LIST	*hints;
	int		with_ascend_hack;
	int		ascend_channels_per_line;
	int		with_ntdomain_hack;
	int		with_specialix_jetstream_hack;
	int		with_cisco_vsa_hack;
	/* zl added begin */
	int		with_bjfu_hack; 
	char	bjfu_hack_tokenchar;
	int		bjfu_hack_overwrite_ip;
	int		bjfu_hack_overwrite_mac;
	/* zl added end */
} rlm_preprocess_t;

static CONF_PARSER module_config[] = {
	{ "huntgroups",			PW_TYPE_STRING_PTR,
	  offsetof(rlm_preprocess_t,huntgroup_file), NULL,
	  "${raddbdir}/huntgroups" },
	{ "hints",			PW_TYPE_STRING_PTR,
	  offsetof(rlm_preprocess_t,hints_file), NULL,
	  "${raddbdir}/hints" },
	{ "with_ascend_hack",		PW_TYPE_BOOLEAN,
	  offsetof(rlm_preprocess_t,with_ascend_hack), NULL, "no" },
	{ "ascend_channels_per_line",   PW_TYPE_INTEGER,
	  offsetof(rlm_preprocess_t,ascend_channels_per_line), NULL, "23" },

	{ "with_ntdomain_hack",		PW_TYPE_BOOLEAN,
	  offsetof(rlm_preprocess_t,with_ntdomain_hack), NULL, "no" },
	{ "with_specialix_jetstream_hack",  PW_TYPE_BOOLEAN,
	  offsetof(rlm_preprocess_t,with_specialix_jetstream_hack), NULL,
	  "no" },
	{ "with_cisco_vsa_hack",        PW_TYPE_BOOLEAN,
	  offsetof(rlm_preprocess_t,with_cisco_vsa_hack), NULL, "no" },

	/* zl added begin */
	{ "with_bjfu_hack",		PW_TYPE_BOOLEAN,
	  offsetof(rlm_preprocess_t,with_bjfu_hack), NULL, "no" },

	{ "bjfu_hack_tokenchar",		PW_TYPE_INTEGER,
	  offsetof(rlm_preprocess_t,bjfu_hack_tokenchar), NULL, "10" },	/* default '\n' */

	{ "bjfu_hack_overwrite_ip",		PW_TYPE_BOOLEAN,
	  offsetof(rlm_preprocess_t,bjfu_hack_overwrite_ip), NULL, "yes" },	

	/* we have a Calling-Station-Id here, as spec says, */
    /* it should be a MAC of supplicant, so default to "no" */                
	{ "bjfu_hack_overwrite_mac",		PW_TYPE_BOOLEAN,
	  offsetof(rlm_preprocess_t,bjfu_hack_overwrite_mac), NULL, "no" },	

	/* zl added end */

	{ NULL, -1, 0, NULL, NULL }
};

/*
 * zl added begin
 *
 * bjfu_hack
 * This hack extract IP&MAC address from username and store them
 * into Framed-IP-Address & Calling-Station-Id.
 * The username was expected with format, in case a token char '@'
 * is defined.
 *		"username@10.1.10.44@00:00:00:00:00:00"
 *
 */
static void bjfu_hack(VALUE_PAIR *vp, char token, int overwrite_ip, int overwrite_mac)
{
    VALUE_PAIR *username;
    VALUE_PAIR *ip;
    VALUE_PAIR *mac;
    char *p1;
    char *p2;

    username = pairfind(vp, PW_USER_NAME);
    if (!username)		return;

	radlog(L_AUTH, "rlm_preprocess: bjfu_hacking %s with token %c", username->strvalue, token);

    /*
     * split username attribute
     */
    p1 = strchr(username->strvalue, token);    /* find first token */
    if (!p1)	return;

    p2 = strchr(p1+1, token);                  /* find second token */
    if (p2)         *p2 = 0;                  /* break the string */    

    /*
     * trim username attribute
     */
    *p1 = 0;
    username->length = strlen(username->strvalue);
	radlog(L_AUTH, "rlm_preprocess: bjfu_hacking username to:%s", username->strvalue);

    /*
     * append or rewrite attribute Framed-IP-Address
     */
    ip = pairfind(vp, PW_FRAMED_IP_ADDRESS);
    if (ip) {
		/* we have a Framed-IP-Address here, overwrite ? */
		if (overwrite_ip) {
			radlog(L_AUTH, "rlm_preprocess: bjfu_hack found existing Framed-IP-Address, overwrite it");
			pairparsevalue(ip, p1+1);
		} else 
			radlog(L_AUTH, "rlm_preprocess: bjfu_hack found existing Framed-IP-Address, keep it");
    } else {
            /* append new attribute */
            ip = pairmake("Framed-IP-Address", p1+1, T_OP_SET);
			if (ip)	{
				pairadd(&vp, ip);
				radlog(L_AUTH, "rlm_preprocess: bjfu_hacking username to:%s", username->strvalue);
			}
    }
    /*
     * append or rewrite attribute Calling-Station-Id
     */
    if (!p2)	return; /* no MAC found */

    mac = pairfind(vp, PW_CALLING_STATION_ID);
    if (mac) {
		if (overwrite_mac) {
			radlog(L_AUTH, "rlm_preprocess: bjfu_hack found existing Calling-Station-Id, keep it");
			pairparsevalue(mac, p2+1);
		} else 
			radlog(L_AUTH, "rlm_preprocess: bjfu_hack found existing Calling-Station-Id, keep it");
	} else {
            /* append new attribute */
            mac = pairmake("Calling-Station-Id", p2+1, T_OP_SET);
			if (mac) {
				pairadd(&vp, mac);
				radlog(L_AUTH, "rlm_preprocess: bjfu_hack append Calling-Station-Id as %s", p2);
			}
    }
}
/* zl added end */


/*
 *	dgreer --
 *	This hack changes Ascend's wierd port numberings
 *	to standard 0-??? port numbers so that the "+" works
 *	for IP address assignments.
 */
static void ascend_nasport_hack(VALUE_PAIR *nas_port, int channels_per_line)
{
	int service;
	int line;
	int channel;

	if (!nas_port) {
		return;
	}

	if (nas_port->lvalue > 9999) {
		service = nas_port->lvalue/10000; /* 1=digital 2=analog */
		line = (nas_port->lvalue - (10000 * service)) / 100;
		channel = nas_port->lvalue-((10000 * service)+(100 * line));
		nas_port->lvalue =
			(channel - 1) + (line - 1) * channels_per_line;
	}
}

/*
 *	ThomasJ --
 *	This hack strips out Cisco's VSA duplicities in lines
 *	(Cisco not implemented VSA's in standard way.
 *
 *	Cisco sends it's VSA attributes with the attribute name *again*
 *	in the string, like:  H323-Attribute = "h323-attribute=value".
 *	This sort of behaviour is nonsense.
 */
static void cisco_vsa_hack(VALUE_PAIR *vp)
{
	int		vendorcode;
	char		*ptr;
	char		newattr[MAX_STRING_LEN];

	for ( ; vp != NULL; vp = vp->next) {
		vendorcode = (vp->attribute >> 16); /* HACK! */
		if (vendorcode == 0) continue;	/* ignore non-VSA attributes */

		if (vendorcode == 0) continue; /* ignore unknown VSA's */

		if (vendorcode != 9) continue; /* not a Cisco VSA, continue */

		if (vp->type != PW_TYPE_STRING) continue;

		/*
		 *  No weird packing.  Ignore it.
		 */
		ptr = strchr(vp->strvalue, '='); /* find an '=' */
		if (!ptr) continue;

		/*
		 *	Cisco-AVPair's get packed as:
		 *
		 *	Cisco-AVPair = "h323-foo-bar = baz"
		 *
		 *	which makes sense only if you're a lunatic.
		 *	This code looks for the attribute named inside
		 *	of the string, and if it exists, adds it as a new
		 *	attribute.
		 */
		if ((vp->attribute & 0xffff) == 1) {
			char *p;
			DICT_ATTR	*dattr;

			p = vp->strvalue;
			getword(&p, newattr, sizeof(newattr));

			if (((dattr = dict_attrbyname(newattr)) != NULL) &&
			    (dattr->type == PW_TYPE_STRING)) {
				VALUE_PAIR *newvp;
				
				/*
				 *  Make a new attribute.
				 */
				newvp = pairmake(newattr, ptr + 1, T_OP_EQ);
				if (newvp) {
					pairadd(&vp, newvp);
				}
			}
		} else {	/* h322-foo-bar = "h323-foo-bar = baz" */
			/*
			 *	We strip out the duplicity from the
			 *	value field, we use only the value on
			 *	the right side of the '=' character.
			 */
			strNcpy(newattr, ptr + 1, sizeof(newattr));
			strNcpy((char *)vp->strvalue, newattr,
				sizeof(vp->strvalue));
			vp->length = strlen((char *)vp->strvalue);
		}
	}
}

/*
 *	Mangle username if needed, IN PLACE.
 */
static void rad_mangle(rlm_preprocess_t *data, REQUEST *request)
{
	VALUE_PAIR	*namepair;
	VALUE_PAIR	*request_pairs;
	VALUE_PAIR	*tmp;

	/*
	 *	Get the username from the request
	 *	If it isn't there, then we can't mangle the request.
	 */
	request_pairs = request->packet->vps;
	namepair = pairfind(request_pairs, PW_USER_NAME);
	if ((namepair == NULL) || 
	    (namepair->length <= 0)) {
	  return;
	}

	/* zl added begin 
	 * 
	 * Fix user name for BJFU format
	 */
	if (data->with_bjfu_hack) 
		bjfu_hack(request_pairs, data->bjfu_hack_tokenchar, data->bjfu_hack_overwrite_ip, data->bjfu_hack_overwrite_mac);
	/* zl added end  */

	if (data->with_ntdomain_hack) {
		char		*ptr;
		char		newname[MAX_STRING_LEN];

		/*
		 *	Windows NT machines often authenticate themselves as
		 *	NT_DOMAIN\username. Try to be smart about this.
		 *
		 *	FIXME: should we handle this as a REALM ?
		 */
		if ((ptr = strchr(namepair->strvalue, '\\')) != NULL) {
			strNcpy(newname, ptr + 1, sizeof(newname));
			/* Same size */
			strcpy(namepair->strvalue, newname);
			namepair->length = strlen(newname);
		}
	}

	if (data->with_specialix_jetstream_hack) {
		char		*ptr;

		/*
		 *	Specialix Jetstream 8500 24 port access server.
		 *	If the user name is 10 characters or longer, a "/"
		 *	and the excess characters after the 10th are
		 *	appended to the user name.
		 *
		 *	Reported by Lucas Heise <root@laonet.net>
		 */
		if ((strlen((char *)namepair->strvalue) > 10) &&
		    (namepair->strvalue[10] == '/')) {
			for (ptr = (char *)namepair->strvalue + 11; *ptr; ptr++)
				*(ptr - 1) = *ptr;
			*(ptr - 1) = 0;
			namepair->length = strlen((char *)namepair->strvalue);
		}
	}

	/*
	 *	Small check: if Framed-Protocol present but Service-Type
	 *	is missing, add Service-Type = Framed-User.
	 */
	if (pairfind(request_pairs, PW_FRAMED_PROTOCOL) != NULL &&
	    pairfind(request_pairs, PW_SERVICE_TYPE) == NULL) {
		tmp = paircreate(PW_SERVICE_TYPE, PW_TYPE_INTEGER);
		if (tmp) {
			tmp->lvalue = PW_FRAMED_USER;
			pairmove(&request_pairs, &tmp);
		}
	}
}

/*
 *	Compare the request with the "reply" part in the
 *	huntgroup, which normally only contains username or group.
 *	At least one of the "reply" items has to match.
 */
static int hunt_paircmp(VALUE_PAIR *request, VALUE_PAIR *check)
{
	VALUE_PAIR	*check_item = check;
	VALUE_PAIR	*tmp;
	int		result = -1;

	if (check == NULL) return 0;

	while (result != 0 && check_item != NULL) {

		tmp = check_item->next;
		check_item->next = NULL;

		result = paircmp(NULL, request, check_item, NULL);

		check_item->next = tmp;
		check_item = check_item->next;
	}

	return result;
}


/*
 *	Compare prefix/suffix
 */
static int presufcmp(VALUE_PAIR *check, char *name, char *rest)
{
	int		len, namelen;
	int		ret = -1;

#if 0 /* DEBUG */
	printf("Comparing %s and %s, check->attr is %d\n",
		name, check->strvalue, check->attribute);
#endif

	len = strlen((char *)check->strvalue);
	switch (check->attribute) {
		case PW_PREFIX:
			ret = strncmp(name, (char *)check->strvalue, len);
			if (ret == 0 && rest)
				strcpy(rest, name + len);
			break;
		case PW_SUFFIX:
			namelen = strlen(name);
			if (namelen < len)
				break;
			ret = strcmp(name + namelen - len,
				     (char *)check->strvalue);
			if (ret == 0 && rest) {
				strncpy(rest, name, namelen - len);
				rest[namelen - len] = 0;
			}
			break;
	}

	return ret;
}

/*
 *	Match a username with a wildcard expression.
 *	Is very limited for now.
 */
static int matches(char *name, PAIR_LIST *pl, char *matchpart)
{
	int len, wlen;
	int ret = 0;
	char *wild = pl->name;
	VALUE_PAIR *tmp;

	/*
	 *	We now support both:
	 *
	 *		DEFAULT	Prefix = "P"
	 *
	 *	and
	 *		P*
	 */
	if ((tmp = pairfind(pl->check, PW_PREFIX)) != NULL ||
	    (tmp = pairfind(pl->check, PW_SUFFIX)) != NULL) {

		if (strncmp(pl->name, "DEFAULT", 7) == 0 ||
		    strcmp(pl->name, name) == 0)
			return !presufcmp(tmp, name, matchpart);
	}

	/*
	 *	Shortcut if there's no '*' in pl->name.
	 */
	if (strchr(pl->name, '*') == NULL &&
	    (strncmp(pl->name, "DEFAULT", 7) == 0 ||
	     strcmp(pl->name, name) == 0)) {
		strcpy(matchpart, name);
		return 1;
	}

	/*
	 *	Normally, we should return 0 here, but we
	 *	support the old * stuff.
	 */
	len = strlen(name);
	wlen = strlen(wild);

	if (len == 0 || wlen == 0) return 0;

	if (wild[0] == '*') {
		wild++;
		wlen--;
		if (wlen <= len && strcmp(name + (len - wlen), wild) == 0) {
			strcpy(matchpart, name);
			matchpart[len - wlen] = 0;
			ret = 1;
		}
	} else if (wild[wlen - 1] == '*') {
		if (wlen <= len && strncmp(name, wild, wlen - 1) == 0) {
			strcpy(matchpart, name + wlen - 1);
			ret = 1;
		}
	}

	return ret;
}


/*
 *	Add hints to the info sent by the terminal server
 *	based on the pattern of the username.
 */
static int hints_setup(PAIR_LIST *hints, REQUEST *request)
{
	char		newname[MAX_STRING_LEN];
	char		*name;
	VALUE_PAIR	*add;
	VALUE_PAIR	*last;
	VALUE_PAIR	*tmp;
	PAIR_LIST	*i;
	int		do_strip;
	VALUE_PAIR *request_pairs;

	request_pairs = request->packet->vps;

	if (hints == NULL || request_pairs == NULL)
		return RLM_MODULE_NOOP;

	/* 
	 *	Check for valid input, zero length names not permitted 
	 */
	if ((tmp = pairfind(request_pairs, PW_USER_NAME)) == NULL)
		name = NULL;
	else
		name = (char *)tmp->strvalue;

	if (name == NULL || name[0] == 0)
		/*
		 *	No name, nothing to do.
		 */
		return RLM_MODULE_NOOP;

	for (i = hints; i; i = i->next) {
		if (matches(name, i, newname)) {
			DEBUG2("  hints: Matched %s at %d",
			       i->name, i->lineno);
			break;
		}
	}

	if (i == NULL) return RLM_MODULE_NOOP;

	add = paircopy(i->reply);

#if 0 /* DEBUG */
	printf("In hints_setup, newname is %s\n", newname);
#endif

	/*
	 *	See if we need to adjust the name.
	 */
	do_strip = 1;
	if ((tmp = pairfind(i->reply, PW_STRIP_USER_NAME)) != NULL
	     && tmp->lvalue == 0)
		do_strip = 0;
	if ((tmp = pairfind(i->check, PW_STRIP_USER_NAME)) != NULL
	     && tmp->lvalue == 0)
		do_strip = 0;

	if (do_strip) {
		tmp = pairfind(request_pairs, PW_STRIPPED_USER_NAME);
		if (tmp) {
			strcpy((char *)tmp->strvalue, newname);
			tmp->length = strlen((char *)tmp->strvalue);
		} else {
			/*
			 *	No Stripped-User-Name exists: add one.
			 */
			tmp = paircreate(PW_STRIPPED_USER_NAME, PW_TYPE_STRING);
			if (!tmp) {
				radlog(L_ERR|L_CONS, "no memory");
				exit(1);
			}
			strcpy((char *)tmp->strvalue, newname);
			tmp->length = strlen((char *)tmp->strvalue);
			pairadd(&request_pairs, tmp);
		}
		request->username = tmp;
	}

	/*
	 *	Now add all attributes to the request list,
	 *	except the PW_STRIP_USER_NAME one.
	 */
	pairdelete(&add, PW_STRIP_USER_NAME);
	for(last = request_pairs; last && last->next; last = last->next)
		;
	if (last) last->next = add;

	return RLM_MODULE_UPDATED;
}

/*
 *	See if the huntgroup matches. This function is
 *	tied to the "Huntgroup" keyword.
 */
static int huntgroup_cmp(void *instance, REQUEST *req, VALUE_PAIR *request, VALUE_PAIR *check,
			 VALUE_PAIR *check_pairs, VALUE_PAIR **reply_pairs)
{
	PAIR_LIST	*i;
	char		*huntgroup;
	rlm_preprocess_t *data = (rlm_preprocess_t *) instance;

	check_pairs = check_pairs; /* shut the compiler up */
	reply_pairs = reply_pairs;

	huntgroup = (char *)check->strvalue;

	for (i = data->huntgroups; i; i = i->next) {
		if (strcmp(i->name, huntgroup) != 0)
			continue;
		if (paircmp(req, request, i->check, NULL) == 0) {
			DEBUG2("  huntgroups: Matched %s at %d",
			       i->name, i->lineno);
			break;
		}
	}

	/*
	 *	paircmp() expects to see zero on match, so let's
	 *	keep it happy.
	 */
	if (i == NULL) {
		return -1;
	}
	return 0;
}


/*
 *	See if we have access to the huntgroup.
 */
static int huntgroup_access(PAIR_LIST *huntgroups, VALUE_PAIR *request_pairs)
{
	PAIR_LIST	*i;
	int		r = RLM_MODULE_OK;

	/*
	 *	We're not controlling access by huntgroups:
	 *	Allow them in.
	 */
	if (huntgroups == NULL)
		return RLM_MODULE_OK;

	for(i = huntgroups; i; i = i->next) {
		/*
		 *	See if this entry matches.
		 */
		if (paircmp(NULL, request_pairs, i->check, NULL) != 0)
			continue;

		/*
		 *	Now check for access.
		 */
		r = RLM_MODULE_REJECT;
		if (hunt_paircmp(request_pairs, i->reply) == 0) {
			VALUE_PAIR *vp;

			/*
			 *  We've matched the huntgroup, so add it in
			 *  to the list of request pairs.
			 */
			vp = pairfind(request_pairs, PW_HUNTGROUP_NAME);
			if (!vp) {
				vp = paircreate(PW_HUNTGROUP_NAME,
						PW_TYPE_STRING);
				if (!vp) {
					radlog(L_ERR, "No memory");
					exit(1);
				}
				
				strNcpy(vp->strvalue, i->name,
					sizeof(vp->strvalue));
				vp->length = strlen(vp->strvalue);

				pairadd(&request_pairs, vp);
			}
			r = RLM_MODULE_OK;
		}
		break;
	}

	return r;
}

/*
 *	If the NAS wasn't smart enought to add a NAS-IP-Address
 *	to the request, then add it ourselves.
 */
static void add_nas_attr(REQUEST *request)
{
	VALUE_PAIR *nas;

	nas = pairfind(request->packet->vps, PW_NAS_IP_ADDRESS);
	if (!nas) {
		nas = paircreate(PW_NAS_IP_ADDRESS, PW_TYPE_IPADDR);
		if (!nas) {
			radlog(L_ERR, "No memory");
			exit(1);
		}
		nas->lvalue = request->packet->src_ipaddr;
		ip_hostname(nas->strvalue, sizeof(nas->strvalue), nas->lvalue);
		pairadd(&request->packet->vps, nas);
	}

	/*
	 *	Add in a Client-IP-Address, to tell the user
	 *	the source IP of the request.  That is, the client,
	 *
	 *	Note that this MAY BE different from the NAS-IP-Address,
	 *	especially if the request is being proxied.
	 *
	 *	Note also that this is a server configuration item,
	 *	and will NOT make it to any packets being sent from
	 *	the server.
	 */
	nas = paircreate(PW_CLIENT_IP_ADDRESS, PW_TYPE_IPADDR);
	if (!nas) {
	  radlog(L_ERR, "No memory");
	  exit(1);
	}
	nas->lvalue = request->packet->src_ipaddr;
	ip_hostname(nas->strvalue, sizeof(nas->strvalue), nas->lvalue);
	pairadd(&request->packet->vps, nas);
}


/*
 *	Initialize.
 */
static int preprocess_instantiate(CONF_SECTION *conf, void **instance)
{
	int	rcode;
	rlm_preprocess_t *data;

	/*
	 *	Allocate room to put the module's instantiation data.
	 */
	data = (rlm_preprocess_t *) rad_malloc(sizeof(*data));
	memset(data, 0, sizeof(*data));

	/*
	 *	Read this modules configuration data.
	 */
        if (cf_section_parse(conf, data, module_config) < 0) {
		free(data);
                return -1;
        }

	data->huntgroups = NULL;
	data->hints = NULL;

	/*
	 *	Read the huntgroups file.
	 */
	rcode = pairlist_read(data->huntgroup_file, &(data->huntgroups), 0);
	if (rcode < 0) {
		radlog(L_ERR|L_CONS, "rlm_preprocess: Error reading %s",
		       data->huntgroup_file);
		return -1;
	}

	/*
	 *	Read the hints file.
	 */
	rcode = pairlist_read(data->hints_file, &(data->hints), 0);
	if (rcode < 0) {
		radlog(L_ERR|L_CONS, "rlm_preprocess: Error reading %s",
		       data->hints_file);
		return -1;
	}

	/*
	 *	Register the huntgroup comparison operation.
	 */
	paircompare_register(PW_HUNTGROUP_NAME, 0, huntgroup_cmp, data);

	/*
	 *	Save the instantiation data for later.
	 */
	*instance = data;

	return 0;
}

/*
 *	Preprocess a request.
 */
static int preprocess_authorize(void *instance, REQUEST *request)
{
	char buf[1024];
	rlm_preprocess_t *data = (rlm_preprocess_t *) instance;

	/*
	 *	Mangle the username, to get rid of stupid implementation
	 *	bugs.
	 */
	rad_mangle(data, request);

	if (data->with_ascend_hack) {
		/*
		 *	If we're using Ascend systems, hack the NAS-Port-Id
		 *	in place, to go from Ascend's weird values to something
		 *	approaching rationality.
		 */
		ascend_nasport_hack(pairfind(request->packet->vps,
					     PW_NAS_PORT),
				    data->ascend_channels_per_line);
	}

	if (data->with_cisco_vsa_hack) {
	 	/*
		 *	We need to run this hack because the h323-conf-id
		 *	attribute should be used.
		 */
		cisco_vsa_hack(request->packet->vps);
	}

	/*
	 *	Note that we add the Request-Src-IP-Address to the request
	 *	structure BEFORE checking huntgroup access.  This allows
	 *	the Request-Src-IP-Address to be used for huntgroup
	 *	comparisons.
	 */
	add_nas_attr(request);

	hints_setup(data->hints, request);

	/*
	 *      If there is a PW_CHAP_PASSWORD attribute but there
	 *      is PW_CHAP_CHALLENGE we need to add it so that other
	 *	modules can use it as a normal attribute.
	 */
	if (pairfind(request->packet->vps, PW_CHAP_PASSWORD) &&
	    pairfind(request->packet->vps, PW_CHAP_CHALLENGE) == NULL) {
		VALUE_PAIR *vp;
		vp = paircreate(PW_CHAP_CHALLENGE, PW_TYPE_OCTETS);
		if (!vp) {
			radlog(L_ERR|L_CONS, "no memory");
			exit(1);
		}
		vp->length = AUTH_VECTOR_LEN;
		memcpy(vp->strvalue, request->packet->vector, AUTH_VECTOR_LEN);
		pairadd(&request->packet->vps, vp);
	}

	if (huntgroup_access(data->huntgroups, request->packet->vps) != RLM_MODULE_OK) {
		radlog(L_AUTH, "No huntgroup access: [%s] (%s)",
		    request->username->strvalue,
		    auth_name(buf, sizeof(buf), request, 1));
		return RLM_MODULE_REJECT;
	}

	return RLM_MODULE_OK; /* Meaning: try next authorization module */
}

/*
 *	Preprocess a request before accounting
 */
static int preprocess_preaccounting(void *instance, REQUEST *request)
{
	int r;
	rlm_preprocess_t *data = (rlm_preprocess_t *) instance;

	/*
	 *  Ensure that we have the SAME user name for both
	 *  authentication && accounting.
	 */
	rad_mangle(data, request);

	if (data->with_cisco_vsa_hack) {
	 	/*
		 *	We need to run this hack because the h323-conf-id
		 *	attribute should be used.
		 */
		cisco_vsa_hack(request->packet->vps);
	}

	/*
	 *  Ensure that we log the NAS IP Address in the packet.
	 */
	add_nas_attr(request);

	r = hints_setup(data->hints, request);

	return r;
}

/*
 *      Clean up the module's instance.
 */
static int preprocess_detach(void *instance)
{
	rlm_preprocess_t *data = (rlm_preprocess_t *) instance;

	paircompare_unregister(PW_HUNTGROUP_NAME, huntgroup_cmp);
	pairlist_free(&(data->huntgroups));
	pairlist_free(&(data->hints));

	free(data->huntgroup_file);
	free(data->hints_file);
	free(data);

	return 0;
}

/* globally exported name */
module_t rlm_preprocess = {
	"preprocess",
	0,			/* type: reserved */
	NULL,			/* initialization */
	preprocess_instantiate,	/* instantiation */
	{
		NULL,			/* authentication */
		preprocess_authorize,	/* authorization */
		preprocess_preaccounting, /* pre-accounting */
		NULL,			/* accounting */
		NULL,			/* checksimul */
		NULL,			/* pre-proxy */
		NULL,			/* post-proxy */
		NULL			/* post-auth */
	},
	preprocess_detach,	/* detach */
	NULL,			/* destroy */
};

