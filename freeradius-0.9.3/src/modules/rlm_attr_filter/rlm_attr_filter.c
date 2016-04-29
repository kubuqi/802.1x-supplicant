/*
 * rlm_attr_filter.c  - Filter A/V Pairs received back from proxy reqs
 *                      before sending reply to the NAS/Server that sent
 *                      it to us.
 *
 * Version:      $Id: rlm_attr_filter.c,v 1.13.2.2 2003/09/30 19:32:11 phampson Exp $
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
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Copyright (C) 2001 The FreeRADIUS server project
 * Copyright (C) 2001 Chris Parker <cparker@starnetusa.net>
 */

#include	"autoconf.h"
#include	"libradius.h"

#include	<sys/stat.h>

#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>
#include	<ctype.h>
#include	<fcntl.h>
#include        <limits.h>

#ifdef HAVE_REGEX_H
#  include      <regex.h>
#endif

#include	"radiusd.h"
#include	"modules.h"

static const char rcsid[] = "$Id: rlm_attr_filter.c,v 1.13.2.2 2003/09/30 19:32:11 phampson Exp $";

struct attr_filter_instance {

        /* autz */
        char *attrsfile;
        PAIR_LIST *attrs;

};

/*
 *	Copy the specified attribute to the specified list
 */
static void mypairappend(VALUE_PAIR *item, VALUE_PAIR **to)
{
  VALUE_PAIR *tmp;

  tmp = paircreate(item->attribute, item->type);
  if( tmp == NULL ) {
    radlog(L_ERR|L_CONS, "no memory");
    exit(1);
  }
  switch (tmp->type) {
      case PW_TYPE_INTEGER:
      case PW_TYPE_IPADDR:
      case PW_TYPE_DATE:
	tmp->lvalue = item->lvalue;
	break;
      default:
	memcpy((char *)tmp->strvalue, (char *)item->strvalue, item->length);
	tmp->length = item->length;
	break;
  }
  pairadd(to, tmp);
}

/*
 *     See if a VALUE_PAIR list contains Fall-Through = Yes
 */
static int fallthrough(VALUE_PAIR *vp)
{
	VALUE_PAIR *tmp;

	tmp = pairfind(vp, PW_FALL_THROUGH);

	return tmp ? tmp->lvalue : 0;
}

static CONF_PARSER module_config[] = {
        { "attrsfile",     PW_TYPE_STRING_PTR,
	  offsetof(struct attr_filter_instance,attrsfile), NULL, "${raddbdir}/attrs" },
	{ NULL, -1, 0, NULL, NULL }
};

static int getattrsfile(const char *filename, PAIR_LIST **pair_list)
{
        int rcode;
        PAIR_LIST *attrs = NULL;
	PAIR_LIST *entry;
	VALUE_PAIR *vp;

	rcode = pairlist_read(filename, &attrs, 1);
	if (rcode < 0) {
	    return -1;
	}
	
        /*
         *	Walk through the 'attrs' file list.
         */
	
	entry = attrs;
	while (entry) {
		
	    entry->check = entry->reply;
	    entry->reply = NULL;
	
	    for (vp = entry->check; vp != NULL; vp = vp->next) {

	        /*
		 *	If it's NOT a vendor attribute,
		 *	and it's NOT a wire protocol
		 *	and we ignore Fall-Through,
		 *	then bitch about it, giving a
		 *	good warning message.
		 */
	        if (!(vp->attribute & ~0xffff) &&
		    (vp->attribute > 0xff) &&
		    (vp->attribute > 1000)) {
		    log_debug("[%s]:%d WARNING! Check item \"%s\"\n"
			      "\tfound in filter list for realm \"%s\".\n",
			      filename, entry->lineno, vp->name,
			      entry->name);
		}
	    }
	    
	    entry = entry->next;
	}

	*pair_list = attrs;
        return 0;
}

/*
 *	(Re-)read the "attrs" file into memory.
 */
static int attr_filter_instantiate(CONF_SECTION *conf, void **instance)
{
        struct attr_filter_instance *inst;
	int rcode;

        inst = rad_malloc(sizeof *inst);
	if (!inst) {
		return -1;
	}
	memset(inst, 0, sizeof(*inst));

        if (cf_section_parse(conf, inst, module_config) < 0) {
                free(inst);
                return -1;
        }

	rcode = getattrsfile(inst->attrsfile, &inst->attrs);
        if (rcode != 0) {
                radlog(L_ERR|L_CONS, "Errors reading %s", inst->attrsfile);
                free(inst->attrsfile);
                free(inst);
                return -1;
        }

        *instance = inst;
        return 0;
}

/*
 *	Find the named realm in the database.  Create the
 *	set of attribute-value pairs to check and reply with
 *	for this realm from the database.
 */
static int attr_filter_authorize(void *instance, REQUEST *request)
{
	struct attr_filter_instance *inst = instance;
	VALUE_PAIR	*request_pairs;
	VALUE_PAIR      **reply_items;
	VALUE_PAIR      *reply_item;
	VALUE_PAIR	*reply_tmp = NULL;
	VALUE_PAIR      *check_item;
	PAIR_LIST	*pl;
	int             found = 0;
	int             compare;
	int             pass, fail;
#ifdef HAVE_REGEX_H
	regex_t         reg;
#endif
	VALUE_PAIR	*realmpair;
        REALM           *realm;
        char            *realmname;

	/*
	 *      It's not a proxy reply, so return NOOP
	 */

	if( request->proxy == NULL ) {
		return( RLM_MODULE_NOOP );
	}

	request_pairs = request->packet->vps;
	reply_items = &request->reply->vps;

 	/*
	 *	Get the realm.  Can't use request->config_items as
	 *      that gets freed by rad_authenticate....  use the one
	 *      set in the original request vps
	 */
	realmpair = pairfind(request_pairs, PW_REALM);
	if(!realmpair) {
	        /*    Can't find a realm, so no filtering of attributes 
		 *    or should we use a DEFAULT entry?
		 *    For now, just return NOTFOUND. (maybe NOOP?)
		 */ 
		return RLM_MODULE_NOTFOUND;
	}

	realmname = (char *) realmpair->strvalue;
        realm = realm_find(realmname, FALSE);

	/*
	 *	Find the attr_filter profile entry for the realm.
	 */
	for(pl = inst->attrs; pl; pl = pl->next) {

	    /*
	     *	If the current entry is NOT a default,
	     *	AND the realm does NOT match the current entry,
	     *	then skip to the next entry.
	     */
	    if ( (strcmp(pl->name, "DEFAULT") != 0)
		 && (strcmp(realmname, pl->name) != 0) )  {
			continue;
		}

		DEBUG2("  attr_filter: Matched entry %s at line %d", pl->name, pl->lineno);

		found = 1;
		
		check_item = pl->check;

		while( check_item != NULL ) {

		    /*
		     *      If it is a SET operator, add the attribute to
		     *      the reply list without checking reply_items.
		     *
		     */

		    if( check_item->operator == T_OP_SET ) {
		        mypairappend(check_item, &reply_tmp);
		    }
		    check_item = check_item->next;

		}  /* while( check_item != NULL ) */

		/* 
		 * Iterate through the reply items, comparing each reply item to every rule,
		 * then moving it to the reply_tmp list only if it matches all rules for that
		 * attribute.  IE, Idle-Timeout is moved only if it matches all rules that
		 * describe an Idle-Timeout.  
		 */

		for( reply_item = *reply_items; 
		     reply_item != NULL; 
		     reply_item = reply_item->next ) {

		  /* reset the pass,fail vars for each reply item */
		  pass = fail = 0;

		  /* reset the check_item pointer to the beginning of the list */
		  check_item = pl->check;

		  while( check_item != NULL ) {
		      
		      if(reply_item->attribute == check_item->attribute) {

			compare = simplepaircmp(request, reply_item, check_item);

			switch(check_item->operator) {

			    case T_OP_SET:            /* nothing to do for set */
			        break;
			    case T_OP_EQ:
			    default:
			        radlog(L_ERR, "Invalid operator for item %s: "
				       "reverting to '=='", check_item->name);
				
			    case T_OP_CMP_TRUE:       /* compare always == 0 */
			    case T_OP_CMP_FALSE:      /* compare always == 1 */
			    case T_OP_CMP_EQ:
			        if (compare == 0) {
				    pass++;
				} else {
				    fail++;
				}
				break;

			    case T_OP_NE:
			        if (compare != 0) {
				    pass++;
				} else {
				    fail++;
				}
				break;

			    case T_OP_LT:
			        if (compare < 0) {
				    pass++;
				} else {
				    fail++;
				}
				break;

			    case T_OP_GT:
			        if (compare > 0) {
				    pass++;
				} else {
				    fail++;
				}
				break;
				
			    case T_OP_LE:
			        if (compare <= 0) {
				    pass++;
				} else {
				    fail++;
				}
				break;

			    case T_OP_GE:
			        if (compare >= 0) {
				    pass++;
				} else {
				    fail++;
				}
				break;
#ifdef HAVE_REGEX_H
			    case T_OP_REG_EQ:
			        regcomp(&reg, (char *)check_item->strvalue, 0);
				compare = regexec(&reg, (char *)reply_item->strvalue,
						  0, NULL, 0);
				regfree(&reg);
				if (compare == 0) {
				    pass++;
				} else {
				    fail++;
				}
				break;

			    case T_OP_REG_NE:
			        regcomp(&reg, (char *)check_item->strvalue, 0);
				compare = regexec(&reg, (char *)reply_item->strvalue,
						  0, NULL, 0);
				regfree(&reg);
				if (compare != 0) {
				    pass++;
				} else {
				    fail++;
				}
				break;
#endif
			}  /* switch( check_item->operator ) */

		      }  /* if reply == check */

		      check_item = check_item->next;

		    }  /* while( check ) */

		    /* only move attribute if it passed all rules */
		    if (fail == 0 && pass > 0) {
		      mypairappend( reply_item, &reply_tmp);
		    }

		}  /* for( reply ) */
		
		/* If we shouldn't fall through, break */
		if(!fallthrough(pl->check))
		    break;
	}

	pairfree(&request->reply->vps);
	request->reply->vps = reply_tmp;
	
	/*
	 *	See if we succeeded.  If we didn't find the realm,
	 *	then exit from the module.
	 */
	if (!found)
		return RLM_MODULE_OK;

	/*
	 *	Remove server internal parameters.
	 */
	pairdelete(reply_items, PW_FALL_THROUGH);

	return RLM_MODULE_UPDATED;
}

/*
 *      Find the named realm in the database.  Create the
 *      set of attribute-value pairs to check and reply with
 *      for this realm from the database.
 */
static int attr_filter_postproxy(void *instance, REQUEST *request)
{
	struct attr_filter_instance *inst = instance;
	VALUE_PAIR	*request_pairs;
	VALUE_PAIR	**reply_items;
	VALUE_PAIR	*reply_item;
	VALUE_PAIR	*reply_tmp = NULL;
	VALUE_PAIR	*check_item;
	PAIR_LIST	*pl;
	int		found = 0;
        int		compare;
        int		pass, fail;
#ifdef HAVE_REGEX_H
	regex_t		reg;
#endif
	VALUE_PAIR	*realmpair;
	REALM		*realm;
	char		*realmname;

	/*
	 *	It's not a proxy reply, so return NOOP
	 */

	if( request->proxy == NULL ) {
		return( RLM_MODULE_NOOP );
	}

	request_pairs = request->packet->vps;
	reply_items = &request->proxy_reply->vps;

	/*
	 *	Get the realm.  Can't use request->config_items as
	 *	that gets freed by rad_authenticate....  use the one
	 *	set in the original request vps
	 */
	realmpair = pairfind(request_pairs, PW_REALM);
	if(!realmpair) {
		/*    Can't find a realm, so no filtering of attributes
		 *    or should we use a DEFAULT entry?
		 *    For now, just return NOTFOUND. (maybe NOOP?)
		 */
		return RLM_MODULE_NOTFOUND;
	}

	realmname = (char *) realmpair->strvalue;
	realm = realm_find(realmname, FALSE);

	/*
	 *      Find the attr_filter profile entry for the realm.
	 */
	for(pl = inst->attrs; pl; pl = pl->next) {

	    /*
	     *  If the current entry is NOT a default,
	     *  AND the realm does NOT match the current entry,
	     *  then skip to the next entry.
	     */
	    if ( (strcmp(pl->name, "DEFAULT") != 0) &&
		 (strcmp(realmname, pl->name) != 0) )  {
		    continue;
	    }

	    DEBUG2("  attr_filter: Matched entry %s at line %d",
		    pl->name, pl->lineno);
	    found = 1;

	    check_item = pl->check;

	    while( check_item != NULL ) {

		    /*
		     *    If it is a SET operator, add the attribute to
		     *    the reply list without checking reply_items.
		     */

		    if( check_item->operator == T_OP_SET ) {
			mypairappend(check_item, &reply_tmp);
		    }
		    check_item = check_item->next;

	    }  /* while( check_item != NULL ) */

	    /*
	     * Iterate through the reply items,
	     * comparing each reply item to every rule,
	     * then moving it to the reply_tmp list only if it matches all
	     * rules for that attribute.
	     * IE, Idle-Timeout is moved only if it matches
	     * all rules that describe an Idle-Timeout.
	     */

	    for( reply_item = *reply_items;
		 reply_item != NULL;
		 reply_item = reply_item->next ) {

		/* reset the pass,fail vars for each reply item */
		pass = fail = 0;

		/* reset the check_item pointer to the beginning of the list */
		check_item = pl->check;

		while( check_item != NULL ) {

		    if(reply_item->attribute == check_item->attribute) {

			compare = simplepaircmp(request, reply_item,
                                                check_item);
			switch(check_item->operator) {

			    case T_OP_SET:	/* nothing to do for set */
				break;
			    case T_OP_EQ:
				default:
				radlog(L_ERR, "Invalid operator for item %s: "
				              "reverting to '=='",
					      check_item->name);

			    case T_OP_CMP_TRUE:       /* compare always == 0 */
			    case T_OP_CMP_FALSE:      /* compare always == 1 */
			    case T_OP_CMP_EQ:
				if (compare == 0) {
				    pass++;
				} else {
				    fail++;
				}
				break;

			    case T_OP_NE:
				if (compare != 0) {
				    pass++;
				} else {
				    fail++;
				}
				break;

			    case T_OP_LT:
				if (compare < 0) {
				    pass++;
				} else {
				    fail++;
				}
				break;

			    case T_OP_GT:
				if (compare > 0) {
				    pass++;
				} else {
				    fail++;
				}
				break;

			    case T_OP_LE:
				if (compare <= 0) {
				    pass++;
				} else {
				    fail++;
				}
				break;

			    case T_OP_GE:
				if (compare >= 0) {
				    pass++;
				} else {
				    fail++;
				}
				break;
#ifdef HAVE_REGEX_H
			    case T_OP_REG_EQ:
				regcomp(&reg, (char *)check_item->strvalue, 0);
				compare = regexec(&reg,
						  (char *)reply_item->strvalue,
						  0, NULL, 0);
				regfree(&reg);
				if (compare == 0) {
				    pass++;
				} else {
				    fail++;
				}
				break;

			    case T_OP_REG_NE:
				regcomp(&reg, (char *)check_item->strvalue, 0);
				compare = regexec(&reg,
						  (char *)reply_item->strvalue,
						  0, NULL, 0);
				regfree(&reg);
				if (compare != 0) {
				    pass++;
				} else {
				    fail++;
				}
				break;
#endif
			}  /* switch( check_item->operator ) */

		    }  /* if reply == check */

		    check_item = check_item->next;

		}  /* while( check ) */

		/* only move attribute if it passed all rules */
		    if (fail == 0 && pass > 0) {
			mypairappend( reply_item, &reply_tmp);
		    }

	    }  /* for( reply ) */

	    /* If we shouldn't fall through, break */
	    if(!fallthrough(pl->check))
		break;
	}

	pairfree(&request->proxy_reply->vps);
	request->proxy_reply->vps = reply_tmp;

	/*
	 *	See if we succeeded.  If we didn't find the realm,
	 *	then exit from the module.
	 */
	if (!found)
	    return RLM_MODULE_OK;

	/*
	 *	Remove server internal parameters.
	 */
	pairdelete(reply_items, PW_FALL_THROUGH);

	return RLM_MODULE_UPDATED;
}

/*
 *	Clean up.
 */
static int attr_filter_detach(void *instance)
{
        struct attr_filter_instance *inst = instance;
        pairlist_free(&inst->attrs);
        free(inst->attrsfile);
        free(inst);
	return 0;
}


/* globally exported name */
module_t rlm_attr_filter = {
	"attr_filter",
	0,				/* type: reserved */
	NULL,				/* initialization */
	attr_filter_instantiate,	/* instantiation */
	{
		NULL,			/* authentication */
		attr_filter_authorize, 	/* authorization */
		NULL,			/* preaccounting */
		NULL,			/* accounting */
		NULL,			/* checksimul */
		NULL,			/* pre-proxy */
		attr_filter_postproxy,	/* post-proxy */
		NULL			/* post-auth */
	},
	attr_filter_detach,		/* detach */
	NULL				/* destroy */
};

