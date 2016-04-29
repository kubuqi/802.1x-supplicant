/*
 * radclient.c	General radius packet debug tool.
 *
 * Version:	$Id: radclient.c,v 1.58 2003/06/25 15:18:08 aland Exp $
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
 * Copyright 2000  Miquel van Smoorenburg <miquels@cistron.nl>
 * Copyright 2000  Alan DeKok <aland@ox.org>
 */
static const char rcsid[] = "$Id: radclient.c,v 1.58 2003/06/25 15:18:08 aland Exp $";

#include "autoconf.h"
#include "libradius.h"

#include <stdio.h>
#include <stdlib.h>

#if HAVE_UNISTD_H
#	include <unistd.h>
#endif

#include <string.h>
#include <ctype.h>
#include <netdb.h>
#include <sys/socket.h>

#if HAVE_NETINET_IN_H
#	include <netinet/in.h>
#endif

#if HAVE_SYS_SELECT_H
#	include <sys/select.h>
#endif

#if HAVE_GETOPT_H
#	include <getopt.h>
#endif

#include "conf.h"
#include "radpaths.h"
#include "missing.h"

static int retries = 10;
static float timeout = 3;
static const char *secret = NULL;
static int do_output = 1;
static int do_summary = 0;
static int filedone = 0;
static int totalapp = 0;
static int totaldeny = 0;
static char filesecret[256];

/*
 *	Read valuepairs from the fp up to End-Of-File.
 */
static VALUE_PAIR *readvp(FILE *fp)
{
	char buf[8192];
	LRAD_TOKEN last_token;
	char *p;
	VALUE_PAIR *vp;
	VALUE_PAIR *list;
	int error = 0;

	list = NULL;

	while (!error && fgets(buf, sizeof(buf), fp) != NULL) {

		p = buf;

		/* If we get a '\n' by itself, we assume that's the end of that VP */
		if((buf[0] == '\n') && (list)) {
			return error ? NULL: list;
		} 
		if((buf[0] == '\n') && (!list))
			continue;
		if(buf[0] == '#') {
			continue;
		} else {
			do {
				if ((vp = pairread(&p, &last_token)) == NULL) {
					librad_perror("radclient:");
					error = 1;
					break;
				}
				pairadd(&list, vp);
			} while (last_token == T_COMMA);
		}
	}
	filedone = 1;
	return error ? NULL: list;
}

static void usage(void)
{
	fprintf(stderr, "Usage: radclient [options] server[:port] <command> [<secret>]\n");
	
	fprintf(stderr, "  <command>    One of auth, acct, status, or disconnect.\n");
	fprintf(stderr, "  -c count    Send each packet 'count' times.\n");
	fprintf(stderr, "  -d raddb    Set dictionary directory.\n");
	fprintf(stderr, "  -f file     Read packets from file, not stdin.\n");
	fprintf(stderr, "  -r retries  If timeout, retry sending the packet 'retries' times.\n");
	fprintf(stderr, "  -t timeout  Wait 'timeout' seconds before retrying (may be a floating point number).\n");
	fprintf(stderr, "  -i id       Set request id to 'id'.  Values may be 0..255\n");
	fprintf(stderr, "  -S file     read secret from file, not command line.\n");
	fprintf(stderr, "  -q          Do not print anything out.\n");
	fprintf(stderr, "  -s          Print out summary information of auth results.\n");
	fprintf(stderr, "  -v          Show program version information.\n");
	fprintf(stderr, "  -x          Debugging mode.\n");

	exit(1);
}

static int getport(const char *name)
{
	struct	servent		*svp;

	svp = getservbyname (name, "udp");
	if (!svp) {
		return 0;
	}

	return ntohs(svp->s_port);
}

static int send_packet(RADIUS_PACKET *req, RADIUS_PACKET **rep)
{
	int i;
	struct timeval	tv;

	for (i = 0; i < retries; i++) {
		fd_set		rdfdesc;

		rad_send(req, NULL, secret);

		/* And wait for reply, timing out as necessary */
		FD_ZERO(&rdfdesc);
		FD_SET(req->sockfd, &rdfdesc);

		tv.tv_sec = (int)timeout;
		tv.tv_usec = 1000000 * (timeout - (int) timeout);

		/* Something's wrong if we don't get exactly one fd. */
		if (select(req->sockfd + 1, &rdfdesc, NULL, NULL, &tv) != 1) {
			continue;
		}

		*rep = rad_recv(req->sockfd);
		if (*rep != NULL) {
			/*
			 *	If we get a response from a machine
			 *	which we did NOT send a request to,
			 *	then complain.
			 */
			if (((*rep)->src_ipaddr != req->dst_ipaddr) ||
			    ((*rep)->src_port != req->dst_port)) {
				char src[64], dst[64];

				ip_ntoa(src, (*rep)->src_ipaddr);
				ip_ntoa(dst, req->dst_ipaddr);
				fprintf(stderr, "radclient: ERROR: Sent request to host %s:%d, got response from host %s:%d\n!",
					dst, req->dst_port,
					src, (*rep)->src_port);
				exit(1);
			}
			break;
		} else {	/* NULL: couldn't receive the packet */
			librad_perror("radclient:");
			exit(1);
		}
	}

	/* No response or no data read (?) */
	if (i == retries) {
		fprintf(stderr, "radclient: no response from server\n");
		exit(1);
	}

	if (rad_decode(*rep, req, secret) != 0) {
		librad_perror("rad_decode");
		exit(1);
	}

	/* libradius debug already prints out the value pairs for us */
	if (!librad_debug && do_output) {
		printf("Received response ID %d, code %d, length = %d\n",
				(*rep)->id, (*rep)->code, (*rep)->data_len);
		vp_printlist(stdout, (*rep)->vps);
	}
	if((*rep)->code == PW_AUTHENTICATION_ACK) {
		totalapp++;
	} else {
		totaldeny++;
	}

	return 0;
}

int main(int argc, char **argv)
{
	RADIUS_PACKET *req;
	RADIUS_PACKET *rep = NULL;
	char *p;
	int c;
	int port = 0;
	const char *radius_dir = RADDBDIR;
	char *filename = NULL;
	FILE *fp;
	int count = 1;
	int loop;
	char password[256];
	VALUE_PAIR *vp;
	int id;

	id = ((int)getpid() & 0xff);
	librad_debug = 0;

	while ((c = getopt(argc, argv, "c:d:f:hi:qst:r:S:xv")) != EOF) switch(c) {
		case 'c':
			if (!isdigit((int) *optarg)) 
				usage();
			count = atoi(optarg);
			break;
		case 'd':
			radius_dir = optarg;
			break;
		case 'f':
			filename = optarg;
			break;
		case 'q':
			do_output = 0;
			break;
		case 'x':
			librad_debug++;
			break;
		case 'r':
			if (!isdigit((int) *optarg)) 
				usage();
			retries = atoi(optarg);
			break;
		case 'i':
			if (!isdigit((int) *optarg)) 
				usage();
			id = atoi(optarg);
			if ((id < 0) || (id > 255)) {
				usage();
			}
			break;
		case 's':
			do_summary = 1;
			break;
		case 't':
			if (!isdigit((int) *optarg)) 
				usage();
			timeout = atof(optarg);
			break;
		case 'v':
			printf("radclient: $Id: radclient.c,v 1.58 2003/06/25 15:18:08 aland Exp $ built on " __DATE__ " at " __TIME__ "\n");
			exit(0);
			break;
               case 'S':
		       fp = fopen(optarg, "r");
                       if (!fp) {
                               fprintf(stderr, "radclient: Error opening %s: %s\n",
                                       optarg, strerror(errno));
                               exit(1);
                       }
                       if (fgets(filesecret, sizeof(filesecret), fp) == NULL) {
                               fprintf(stderr, "radclient: Error reading %s: %s\n",
                                       optarg, strerror(errno));
                               exit(1);
                       }
		       fclose(fp);

                       /* truncate newline */
		       p = filesecret + strlen(filesecret) - 1;
		       while ((p >= filesecret) &&
			      (*p < ' ')) {
			       *p = '\0';
			       --p;
		       }

                       if (strlen(filesecret) < 2) {
                               fprintf(stderr, "radclient: Secret in %s is too short\n", optarg);
                               exit(1);
                       }
                       secret = filesecret;
		       break;
		case 'h':
		default:
			usage();
			break;
	}
	argc -= (optind - 1);
	argv += (optind - 1);

	if ((argc < 3)  ||
	    ((secret == NULL) && (argc < 4))) {
		usage();
	}

	if (dict_init(radius_dir, RADIUS_DICTIONARY) < 0) {
		librad_perror("radclient");
		return 1;
	}

	if ((req = rad_alloc(1)) == NULL) {
		librad_perror("radclient");
		exit(1);
	}

	req->id = id;

	/*
	 *	Strip port from hostname if needed.
	 */
	if ((p = strchr(argv[1], ':')) != NULL) {
		*p++ = 0;
		port = atoi(p);
	}

	/*
	 *	See what kind of request we want to send.
	 */
	if (strcmp(argv[2], "auth") == 0) {
		if (port == 0) port = getport("radius");
		if (port == 0) port = PW_AUTH_UDP_PORT;
		req->code = PW_AUTHENTICATION_REQUEST;

	} else if (strcmp(argv[2], "acct") == 0) {
		if (port == 0) port = getport("radacct");
		if (port == 0) port = PW_ACCT_UDP_PORT;
		req->code = PW_ACCOUNTING_REQUEST;
		do_summary = 0;

	} else if (strcmp(argv[2], "status") == 0) {
		if (port == 0) port = getport("radius");
		if (port == 0) port = PW_AUTH_UDP_PORT;
		req->code = PW_STATUS_SERVER;

	} else if (strcmp(argv[2], "disconnect") == 0) {
		if (port == 0) port = PW_POD_UDP_PORT;
		req->code = PW_DISCONNECT_REQUEST;

	} else if (isdigit((int) argv[2][0])) {
		if (port == 0) port = getport("radius");
		if (port == 0) port = PW_AUTH_UDP_PORT;
		req->code = atoi(argv[2]);
	} else {
		usage();
	}

	/*
	 *	Resolve hostname.
	 */
	req->dst_port = port;
	req->dst_ipaddr = ip_getaddr(argv[1]);
	if (req->dst_ipaddr == INADDR_NONE) {
		fprintf(stderr, "radclient: Failed to find IP address for host %s\n", argv[1]);
		exit(1);
	}

	/*
	 *	Add the secret.
	 */
	if (argv[3]) secret = argv[3];

	/*
	 *	Read valuepairs.
	 *	Maybe read them, from stdin, if there's no
	 *	filename, or if the filename is '-'.
	 */
	if (filename && (strcmp(filename, "-") != 0)) {
		fp = fopen(filename, "r");
		if (!fp) {
			fprintf(stderr, "radclient: Error opening %s: %s\n",
				filename, strerror(errno));
			exit(1);
		}
	} else {
		fp = stdin;
	}
	
	/*
	 *	Send request.
	 */
	if ((req->sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("radclient: socket: ");
		exit(1);
	}

	while(!filedone) {
		if(req->vps) pairfree(&req->vps);

		if ((req->vps = readvp(fp)) == NULL) {
			break;
		}
	

		/*
		 *	Keep a copy of the the User-Password attribute.
		 */
		if ((vp = pairfind(req->vps, PW_PASSWORD)) != NULL) {
			strNcpy(password, (char *)vp->strvalue, sizeof(vp->strvalue));
		/*
		 *	Otherwise keep a copy of the CHAP-Password attribute.
		 */
		} else if ((vp = pairfind(req->vps, PW_CHAP_PASSWORD)) != NULL) {
			strNcpy(password, (char *)vp->strvalue, sizeof(vp->strvalue));
		} else {
			*password = '\0';
		}
	
		/*
		 *  Fix up Digest-Attributes issues
		 */
		for (vp = req->vps; vp != NULL; vp = vp->next) {
		  switch (vp->attribute) {
		  default:
		    break;

		  case PW_DIGEST_REALM:
		  case PW_DIGEST_NONCE:
		  case PW_DIGEST_METHOD:
		  case PW_DIGEST_URI:
		  case PW_DIGEST_QOP:
		  case PW_DIGEST_ALGORITHM:
		  case PW_DIGEST_BODY_DIGEST:
		  case PW_DIGEST_CNONCE:
		  case PW_DIGEST_NONCE_COUNT:
		  case PW_DIGEST_USER_NAME:
		    /* overlapping! */
		    memmove(&vp->strvalue[2], &vp->strvalue[0], vp->length);
		    vp->strvalue[0] = vp->attribute - PW_DIGEST_REALM + 1;
		    vp->length += 2;
		    vp->strvalue[1] = vp->length;
		    vp->attribute = PW_DIGEST_ATTRIBUTES;
		    break;
		  }
		}

		/*
		 *	Loop, sending the packet N times.
		 */
		for (loop = 0; loop < count; loop++) {
			req->id++;
	
			/*
			 *	If we've already sent a packet, free up the old
			 *	one, and ensure that the next packet has a unique
			 *	ID and authentication vector.
			 */
			if (req->data) {
				free(req->data);
				req->data = NULL;
			}

			librad_md5_calc(req->vector, req->vector,
					sizeof(req->vector));
				
			if (*password != '\0') {
				if ((vp = pairfind(req->vps, PW_PASSWORD)) != NULL) {
					strNcpy((char *)vp->strvalue, password, strlen(password) + 1);
					vp->length = strlen(password);
					
				} else if ((vp = pairfind(req->vps, PW_CHAP_PASSWORD)) != NULL) {
					strNcpy((char *)vp->strvalue, password, strlen(password) + 1);
					vp->length = strlen(password);
					
					rad_chap_encode(req, (char *) vp->strvalue, req->id, vp);
					vp->length = 17;
				}
			} /* there WAS a password */

			send_packet(req, &rep);
			rad_free(&rep);
		}
	}
	if(do_summary) {
		printf("\n\t   Total approved auths:  %d\n", totalapp);
		printf("\t     Total denied auths:  %d\n", totaldeny);
	}
	return 0;
}
